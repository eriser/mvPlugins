#include "stdafx.h"
#include "Oscillator.h"
#include "Synth.h"
#include "Logger.h"

namespace mvSynth {

Oscillator::Oscillator()
{
    mModuleType = ModuleType::PerVoice;
    AddPort(PortType::Input, "volume", 1);
    AddPort(PortType::Input, "pitch", 1);
    AddPort(PortType::Input, "detune", 1);
    AddPort(PortType::Input, "stereo", 1);
    AddPort(PortType::Output, "output", 2);

    noteOffset = 0.0;
    subvoicesNum = 5;
    retrig = false;
    detune = 0.005;
    stereo = 1.0;

    enabled = false;
}

void Oscillator::LoadDefaultWaveshape(WaveShape waveShape, Synth* synth)
{
    const int WAVE_TABLE_SIZE_POW = 11;
    const int WAVE_TABLE_SIZE = 1 << WAVE_TABLE_SIZE_POW;

    float wave[WAVE_TABLE_SIZE];

    switch (waveShape)
    {
    case WaveShape::Sine:
        for (int i = 0; i < WAVE_TABLE_SIZE; i++)
            wave[i] = 1.0f * sinf(2.0f * M_PI * (float)i / (float)WAVE_TABLE_SIZE);
        break;

    case WaveShape::Saw:
        for (int i = 0; i < WAVE_TABLE_SIZE; i++)
            wave[i] = 1.5f * (-0.5f + (float)i / (float)WAVE_TABLE_SIZE);
        break;

    case WaveShape::Square:
        for (int i = 0; i < WAVE_TABLE_SIZE; i++)
            wave[i] = (i < WAVE_TABLE_SIZE / 2) ? -0.5f : 0.5f;
        break;

    default:
        return;
    }

    mWaveTable.LoadData(wave, WAVE_TABLE_SIZE_POW, synth->GetInterpolator());
}

bool Oscillator::ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr)
{
    for (const auto& n : node)
    {
        const std::string& str = n.first.as<std::string>();

        if (str == "name")
        {
            mName = n.second.as<std::string>();
        }
        else if (str == "shape")
        {
            WaveShape shape;
            const std::string& shapeStr = n.second.as<std::string>();
            if (shapeStr == "sine")
                shape = WaveShape::Sine;
            else if (shapeStr == "square")
                shape = WaveShape::Square;
            else if (shapeStr == "saw")
                shape = WaveShape::Saw;
            else
            {
                errorStr += "Invalid oscillator shape '" + shapeStr + "'\n";
                return false;
            }

            LoadDefaultWaveshape(shape, synth);
        }
        else if (str == "pitch")
        {
            noteOffset = n.second.as<double>();
        }
        else if (str == "subvoices")
        {
            subvoicesNum = n.second.as<int>();
        }
        else if (str == "detune")
        {
            detune = n.second.as<double>();
        }
        else if (str == "stereo")
        {
            stereo = n.second.as<double>();
        }
        else if (str == "retrig")
        {
            retrig = n.second.as<bool>();
        }
        else
        {
            errorStr += "Invalid parameter '" + str + "' in oscillator module.\n";
            return false;
        }
    }

    return true;
}

// initialize oscillator per voice data
void Oscillator::OnInitVoice(int voiceID, Synth* synth, SampleType freq)
{
    OscillatorVoiceData &data = mVoiceData[voiceID];

    data.faded = false;
    data.baseFrequency = freq * synth->GetSampleRateInv();

    data.wtCtx.voicesNum = subvoicesNum;
    data.wtCtx.Reset();

    float subVoiceVolume = 1.0f / sqrtf(static_cast<float>(subvoicesNum));

    for (uint32 i = 0; i < subvoicesNum; ++i)
    {
        // set up initial subvoice phase
        if (retrig)
            data.wtCtx.phases[i] = 0.0f;
        else
            data.wtCtx.phases[i] = synth->RandD();

        // randomize detune factor
        // TODO: this should be configurable (no randomization, seed, distribution, etc.)
        data.subvoiceDetune[i] = 0.5 * (synth->RandD() - 0.5);

        // set panning
        data.wtCtx.leftPanning[i] = (i % 2) ? 0.0f : subVoiceVolume;
        data.wtCtx.rightPanning[i] = (i % 2) ? subVoiceVolume : 0.0f;
    }

    // if subvoices number is odd, one must be spreaded through both channels
    if (subvoicesNum % 2 == 1)
    {
        data.wtCtx.leftPanning[0] = subVoiceVolume / 2.0f;
        data.wtCtx.rightPanning[0] = subVoiceVolume / 2.0f;
    }
}

// synthesise wave and store result in mOutput frame
void Oscillator::OnProcess(int voiceID, Synth* synth)
{
    OscillatorVoiceData &data = mVoiceData[voiceID];

    if (data.faded)
        return;

    if (subvoicesNum == 0)
    {
        data.faded = true;
        return;
    }

    SampleType freq = data.baseFrequency * pow(2.0, noteOffset / 12.0);

    // calculate subvoices frequencies (log space)
    SampleType subVoiceFreqMult[MW_MAX_SUBVOICES_NUM];
    for (uint32 i = 0; i < subvoicesNum; i++)
    {
        // TODO: different unison subvoices distributions
        subVoiceFreqMult[i] = pow(2.0,
                                  detune * ((SampleType)i - (SampleType)(subvoicesNum - 1) /
                                            2.0 + data.subvoiceDetune[i]));
    }

    const SampleType fadeOutTreshold = 1e-5;
    bool canFadeOut = true;

    for (size_t i = 0; i < synth->GetFrameSize(); i++)
    {
        SampleType finalFreq = freq;
        finalFreq *= (1.0f + mInputs[PitchPortId].frame[i]);

        for (uint32 j = 0; j < subvoicesNum; j++)
            data.wtCtx.freqs[j] = static_cast<float>(finalFreq * subVoiceFreqMult[j]);

        SampleType leftValue;
        SampleType rightValue;
        mWaveTable.Synth_FPU(data.wtCtx, synth->GetInterpolator(),
                             leftValue, rightValue);

        SampleType volume = mInputs[VolumePortId].frame[i];

        // TODO: panning

        mOutputs[0].frame[2 * i] = volume * leftValue;
        mOutputs[0].frame[2 * i + 1] = volume * rightValue;

        // disable voice if it's not audiable
        if (Abs(volume) > fadeOutTreshold)
            canFadeOut = false;
    }

    if (canFadeOut)
        data.faded = true;
}

} // namespace mvSynth
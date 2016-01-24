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

    shape = WaveShape::Saw;
    noteOffset = 0.0;
    subvoicesNum = 5;
    retrig = false;
    detune = 0.005;
    stereo = 1.0;

    enabled = false;
}

bool Oscillator::ParseFromConfig(const YAML::Node& node, std::string& errorStr)
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
    data.baseFrequency = freq;

    for (unsigned int i = 0; i < subvoicesNum; i++)
    {
        // randomize phase & panning
        if (retrig)
        {
            data.states[i].phase = 0.0f;
            data.states[i].detuneOffset = 0.0f;
        }
        else
        {
            data.states[i].phase = synth->RandD() * M_PI;
            data.states[i].detuneOffset = 0.5 * (synth->RandD() - 0.5);
        }

        data.states[i].panning = (i % 2) ? 0 : 1;
        data.states[i].dcb_state = 0.0f;
        data.states[i].freq = 0.0f;
        data.states[i].last = 0.0f;

        // estimate wave value
        data.states[i].leak = 0.0f;
        data.states[i].firstSample = true;
    }

    // if subvoices number is odd, one must be spreaded through both channels
    if (subvoicesNum % 2 == 1)
        data.states[0].panning = 0.5;
}

// oscillator callback
typedef SampleType(*OscillatorCallback)(OscillatorState*, Synth*);


SampleType getSine(OscillatorState* state, Synth* synth)
{
    UNUSED(synth);

    // return fast_sin(state->phase);
    return Sin(state->phase);
}

// bandlimited sawtooth generator based on BLIT integration
SampleType getSaw(OscillatorState* state, Synth* synth)
{
    SampleType phase = state->phase / 2.0;
    SampleType p_ = state->freq * synth->GetSampleRateInv();
    unsigned int maxHarmonics = (unsigned int)Floor(0.5 / p_);
    unsigned int m = 2 * maxHarmonics + 1;

    if (phase > M_PI / 2.0)
        phase -= M_PI;

    SampleType tmp;
    if (Abs(phase) <= 1e-8)
    {
        // Taylor expansion of  sin(m * phase)/sin(phase) at phase = 0
        tmp = m + phase * phase * (m - m * m * m) / 6.0;
    }
    else
    {
        tmp = Sin(m * phase);
        tmp /= Sin(phase);
    }

    tmp = 2.0 * p_ * (tmp - 1.0) + state->leak;
    state->leak = tmp * 0.999;
    return tmp;
}

// bandlimited square wave generator based on BLIT integration
SampleType getSquare(OscillatorState* state, Synth* synth)
{
    SampleType temp = state->leak;
    SampleType p_ = 2.0 * state->freq * synth->GetSampleRateInv();
    SampleType phase = state->phase;

    unsigned int maxHarmonics = (unsigned int)Floor(0.5 / p_ + 0.5);
    int m = 2 * maxHarmonics;
    SampleType a_ = m * p_;

    SampleType denominator = Sin(phase);
    if (Abs(denominator) < 1e-10)
    {
        // Inexact comparison safely distinguishes betwen *close to zero*, and *close to PI*.
        if (phase < 0.1f || phase > 2.0 * M_PI - 0.1)
            state->leak = a_;
        else
            state->leak = -a_;
    }
    else
    {
        state->leak = Sin(m * phase) * p_ / denominator;
    }

    state->leak += temp;

    // Now apply DC blocker.
    state->last = state->leak - state->dcb_state + 0.999 * state->last;
    state->dcb_state = state->leak;

    return state->last;
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

    // pull branching out of the loop
    OscillatorCallback callback = nullptr;
    switch (shape)
    {
    case WaveShape::Saw:
        callback = &getSaw;
        break;
    case WaveShape::Square:
        callback = &getSquare;
        break;
    case WaveShape::Sine:
        callback = &getSine;
        break;
    };

    // invalid wave shape
    if (callback == nullptr)
    {
        data.faded = true;
        return;
    }

    // calculate subvoices frequencies (log space)
    SampleType subVoiceFreqMult[MW_MAX_SUBVOICES_NUM];
    for (uint32 i = 0; i < subvoicesNum; i++)
    {
        // TODO: different unison subvoices distributions
        subVoiceFreqMult[i] = pow(2.0,
                                  detune * ((SampleType)i - (SampleType)(subvoicesNum - 1) /
                                            2.0 + data.states[i].detuneOffset));
    }


    SampleType subVoiceVolume = 1.0 / (SampleType)subvoicesNum;
    const SampleType fadeOutTreshold = 1e-5;
    bool canFadeOut = true;

    for (size_t i = 0; i < synth->GetFrameSize(); i++)
    {
        SampleType leftValue = 0.0f;
        SampleType rightValue = 0.0f;

        SampleType finalFreq = freq;
        finalFreq *= (1.0f + mInputs[PitchPortId].frame[i]);

        // accumulate subvoices
        for (uint32 j = 0; j < subvoicesNum; j++)
        {
            SampleType subVoiceFreq = finalFreq * subVoiceFreqMult[j];

            // phase from 0 to PI
            data.states[j].freq = subVoiceFreq;
            data.states[j].phase += 2.0 * M_PI * subVoiceFreq * synth->GetSampleRateInv();
            if (data.states[j].phase > 2.0 * M_PI)
                data.states[j].phase -= 2.0 * M_PI;

            // get wave value
            SampleType mMonoValue = callback(data.states + j, synth);

            // stereo separation
            leftValue += mMonoValue * data.states[j].panning;
            rightValue += mMonoValue * (1.0 - data.states[j].panning);
        }

        SampleType volumeMult = mInputs[VolumePortId].frame[i];
        SampleType volume = subVoiceVolume * volumeMult;

        mOutputs[0].frame[2 * i] = volume * leftValue;
        mOutputs[0].frame[2 * i + 1] = volume * rightValue;

        // disable voice if it's not audiable
        if (Abs(volumeMult) > fadeOutTreshold)
            canFadeOut = false;
    }

    if (canFadeOut)
        data.faded = true;
}

} // namespace mvSynth
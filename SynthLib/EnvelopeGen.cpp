#include "stdafx.h"
#include "EnvelopeGen.h"
#include "Synth.h"

namespace mvSynth {

void EnvelopeGenVoiceData::Press()
{
    time = 0.0;
    releaseVolume = 0.0f;
    postRelease = false;
    lastValue = 0.0f;
}

void EnvelopeGenVoiceData::Release()
{
    time = 0.0;
    releaseVolume = lastValue;
    postRelease = true;
}

EnvelopeGen::EnvelopeGen()
{
    mModuleType = ModuleType::PerVoice;
    AddPort(PortType::Input, "attack", 1);
    AddPort(PortType::Input, "decay", 1);
    AddPort(PortType::Input, "sustain", 1);
    AddPort(PortType::Input, "release", 1);
    AddPort(PortType::Output, "output", 1);

    // defaults
    Set(0.005f, 0.05f, 1.0f, 0.1f);
}

bool EnvelopeGen::ParseFromConfig(const YAML::Node& node, std::string& errorStr)
{
    double attack = 0.005f;
    double decay = 0.05f;
    double sustain = 1.0f;
    double release = 0.1f;

    for (const auto& n : node)
    {
        const std::string& str = n.first.as<std::string>();

        if (str == "name")
        {
            mName = n.second.as<std::string>();
        }
        else if (str == "attack")
        {
            attack = n.second.as<double>();
        }
        else if (str == "decay")
        {
            decay = n.second.as<double>();
        }
        else if (str == "sustain")
        {
            sustain = n.second.as<double>();
        }
        else if (str == "release")
        {
            release = n.second.as<double>();
        }
        else
        {
            errorStr += "Invalid parameter '" + str + "' in envelope generator module.\n";
            return false;
        }
    }

    Set(attack, decay, sustain, release);
    return true;
}

void EnvelopeGen::OnInitVoice(int voiceID, Synth* synth, SampleType freq)
{
    UNUSED(freq);
    UNUSED(synth);
    mVoiceData[voiceID].Press();
}

void EnvelopeGen::OnReleaseVoice(int voiceID, Synth* synth)
{
    UNUSED(synth);
    mVoiceData[voiceID].Release();
}

void EnvelopeGen::OnProcess(int voiceID, Synth* synth)
{
    EnvelopeGenVoiceData &data = mVoiceData[voiceID];

    Frame<SampleType> &output = mOutputs[0].frame;

    if (data.postRelease)
    {
        // release envelope
        for (size_t i = 0; i < synth->GetFrameSize(); i++)
        {
            output[i] = data.releaseVolume * Exp(-data.time * mReleaseInv);
            data.time += synth->GetSampleRateInv();
        }
    }
    else
    {
        for (size_t i = 0; i < synth->GetFrameSize(); i++)
        {
            if (data.time < mAttack) // attack envelope
                output[i] = 0.5 - 0.5 * Cos(data.time * mAttackInv * M_PI);
            else if (data.time > 0.0)
                output[i] = mSustain + (1.0f - mSustain) * Exp(-(data.time - mAttack) * mDecayInv);
            else
                output[i] = 0.0;

            data.time += synth->GetSampleRateInv();
        }

        data.lastValue = output[synth->GetFrameSize() - 1];
    }
}

void EnvelopeGen::Set(SampleType attack, SampleType decay, SampleType sustain, SampleType release)
{
    mAttack = attack;
    mDecay = decay;
    mSustain = sustain;
    mRelease = release;

    mAttackInv = 1.0f / attack;
    mDecayInv = 1.0f / decay;
    mReleaseInv = 1.0f / release;
}

} // namespace mvSynth
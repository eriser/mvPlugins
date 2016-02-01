#include "stdafx.h"
#include "Module.h"
#include "Synth.h"

namespace mvSynth {

Link::Link()
{
    mOffset = 0.0;
    mScale = 1.0;

    source = dest = nullptr;
    sourcePort = destPort = 0;
}

void Link::SetScaling(const SampleType& offset, const SampleType& scale)
{
    mOffset = offset;
    mScale = scale;
}

Port::Port()
{
    name = "";
}

Module::Module()
{
    enabled = true;
    mModuleType = ModuleType::Global;
}

Module::~Module()
{
}

void Module::DisconnectInputs()
{
    for (auto& p : mInputs)
        p.links.clear();
}

bool Module::ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr)
{
    UNUSED(synth);
    UNUSED(node);
    UNUSED(errorStr);
    return true;
}

void Module::AddPort(PortType type, const char *name, int channels)
{
    Port port;
    port.name = name;
    port.channels = channels;

    if (type == PortType::Input)
        mInputs.push_back(port);
    else if (type == PortType::Output)
        mOutputs.push_back(port);
}

int Module::GetPort(PortType type, const char* name) const
{
    const std::vector<Port>* portList = nullptr;

    if (type == PortType::Input)
        portList = &mInputs;
    else if (type == PortType::Output)
        portList = &mOutputs;

    if (portList == nullptr)
        return 0;

    for (size_t i = 0; i < portList->size(); i++)
    {
        if (strcmp(name, (*portList)[i].name) == 0)
            return i;
    }

    return 0;
}

// Called when key is pressed. Module should initialize per voice data
void Module::OnInitVoice(int voiceID, Synth* synth, SampleType freq)
{
    UNUSED(voiceID);
    UNUSED(synth);
    UNUSED(freq);
}

// Called when key is released
void Module::OnReleaseVoice(int voiceID, Synth* synth)
{
    UNUSED(voiceID);
    UNUSED(synth);
}

void Module::ClearAllVoices()
{
    for (int i = 0; i < MW_MAX_VOICES; i++)
        processed[i] = false;
}

void Module::ResizeFrames(size_t frameSize)
{
    for (Port &port : mInputs)
        port.frame.Resize(frameSize * port.channels);

    for (Port &port : mOutputs)
        port.frame.Resize(frameSize * port.channels);
}

void Module::Process(int voiceID, Synth* synth)
{
    bool bypass = false;

    if ((mModuleType == ModuleType::Global) && (voiceID == SYNTH_GLOBAL_VOICE_ID))
    {
        if (processed[0])
            return;
        processed[0] = true;
    }
    else if ((mModuleType == ModuleType::PerVoice) && (voiceID != SYNTH_GLOBAL_VOICE_ID))
    {
        if (processed[voiceID])
            return;
        processed[voiceID] = true;
    }
    else
        bypass = true;

    for (Port &input : mInputs)
    {
        input.frame.Zero();

        // loop through links connected to the port
        for (Link* link : input.links)
        {
            if (link == nullptr)
                continue;

            Module* prev = link->source;

            prev->Process(voiceID, synth);

            int srcChannels = prev->mOutputs[link->sourcePort].channels;
            int destChannels = input.channels;

            const SampleType *srcBuffer = prev->mOutputs[link->sourcePort].frame.mData;
            SampleType *destBuffer = input.frame.mData;

            if (srcChannels == 1 && destChannels == 1) // mono -> mono
                for (size_t i = 0; i < synth->GetFrameSize(); i++)
                {
                    destBuffer[i] += srcBuffer[i] * link->mScale + link->mOffset;
                }
            else if (srcChannels == 1 && destChannels == 2) // mono -> stereo
                for (size_t i = 0; i < synth->GetFrameSize(); i++)
                {
                    SampleType sample = srcBuffer[i] * link->mScale + link->mOffset;
                    destBuffer[2 * i] += sample;
                    destBuffer[2 * i + 1] += sample;
                }
            else if (srcChannels == 2 && destChannels == 2) // stereo -> stereo
                for (size_t i = 0; i < synth->GetFrameSize(); i++)
                {
                    destBuffer[2 * i] += srcBuffer[2 * i] * link->mScale + link->mOffset;
                    destBuffer[2 * i + 1] += srcBuffer[2 * i + 1] * link->mScale + link->mOffset;
                }
            else if (srcChannels == 2 && destChannels == 1) // stereo -> mono
                for (size_t i = 0; i < synth->GetFrameSize(); i++)
                {
                    SampleType sample = 0.5 * (srcBuffer[2 * i] + srcBuffer[2 * i + 1]);
                    destBuffer[i] += sample * link->mScale + link->mOffset;
                }
        }
    }

    if (!bypass)
        OnProcess(voiceID, synth);
}

} // namespace mvSynth
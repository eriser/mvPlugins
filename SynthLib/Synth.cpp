#include "stdafx.h"
#include "Synth.h"
#include "yaml-cpp/yaml.h"

#include "Oscillator.h"
#include "EnvelopeGen.h"
#include "Filter.h"
#include "LFO.h"
#include "NoiseGen.h"

namespace mvSynth {

Synth::Synth()
{
    SetSampleRate(44100.0);

    mSeedX = 123456789;
    mSeedY = 362436069;
    mSeedZ = 521288629;
    mSeedW = 88675123;

    mVoices.reset(new Voice[MW_MAX_VOICES]);
    for (int i = 0; i < MW_MAX_VOICES; i++)
        mVoices[i].synth = this;

    mValidSamples = 0;
    frameSize = 0;
    SetFrameSize(1024);
}

size_t Synth::GetFrameSize() const
{
    return frameSize;
}

void Synth::SetFrameSize(size_t newSize)
{
    frameSize = newSize;
    mOutput.Resize(2 * frameSize);

    mMaster.ResizeFrames(frameSize);

    for (const auto& module : mModules)
        module.second->ResizeFrames(frameSize);
}

void Synth::SetSampleRate(const SampleType sampleRate)
{
    mSampleRate = sampleRate;
    mSampleRateInv = 1.0 / sampleRate;
}

uint32 Synth::RandU()
{
    mSeedT = mSeedX ^ (mSeedX << 11);
    mSeedX = mSeedY;
    mSeedY = mSeedZ;
    mSeedZ = mSeedW;
    mSeedW = mSeedW ^ (mSeedW >> 19) ^ (mSeedT ^ (mSeedT >> 8));
    return mSeedW;
}

SampleType Synth::RandD()
{
    uint32 low = RandU();
    uint32 high = RandU();
    high = 0x3ff00000 | (0x000fffff & high);
    uint64 a = (uint64)low;
    a |= ((uint64)high) << 32;

    return (*((SampleType*)&a) - 1.0);
}

SampleType Synth::RandF()
{
    uint32 a = (RandU() & 0x007fffff) | 0x40000000;
    return(*((SampleType*)&a) - 3.0f);
}

void Synth::Clear()
{
    mModules.clear();
    mLinks.clear();
    mMaster.DisconnectInputs();
}

void Synth::AddModule(Module* module)
{
    module->ResizeFrames(frameSize);
    mModules.insert(std::make_pair(module->mName, std::unique_ptr<Module>(module)));
}

Link* Synth::CreateLink(Module *src, const char* srcPortName, Module *dest, const char* destPorttName)
{
    std::unique_ptr<Link> link(new Link);
    Link* linkPtr = link.get();

    link->source = src;
    link->dest = (dest != nullptr) ? dest : &mMaster;
    link->sourcePort = link->source->GetPort(PortType::Output, srcPortName);
    link->destPort = link->dest->GetPort(PortType::Input, destPorttName);

    link->source->mOutputs[link->sourcePort].links.insert(linkPtr);
    link->dest->mInputs[link->destPort].links.insert(linkPtr);

    mLinks.emplace(std::move(link));
    return linkPtr;
}

void Synth::PressKey(int key, SampleType velocity, SampleType pan)
{
    // find best voice to exchange
    SampleType maxTime = 0.0f;
    int voiceID = -1;

    Lock lock(mMutex);

    for (int i = 0; i < MW_MAX_VOICES; i++)
    {
        if (mVoices[i].state == VoiceState::Disabled)
        {
            voiceID = i;
            break;
        }
        else if ((mVoices[i].state == VoiceState::Released) && (mVoices[i].time > maxTime))
        {
            maxTime = mVoices[i].time;
            voiceID = i;
        }
    }

    if (voiceID == -1)
    {
        for (int i = 0; i < MW_MAX_VOICES; i++)
        {
            if (mVoices[i].time > maxTime)
            {
                maxTime = mVoices[i].time;
                voiceID = i;
            }
        }
    }

    if (voiceID == -1)
        voiceID = rand() % MW_MAX_VOICES;

    // set up new voice
    mVoices[voiceID].Press(key, velocity, pan);

    mMaster.OnInitVoice(voiceID, this, mVoices[voiceID].freq);
    for (const auto& module : mModules)
        module.second->OnInitVoice(voiceID, this, mVoices[voiceID].freq);
}

void Synth::ReleaseKey(int key)
{
    SampleType minTime = 1e+20f;
    int voiceID = -1;

    Lock lock(mMutex);

    // find voice
    for (int i = 0; i < MW_MAX_VOICES; i++)
    {
        if ((mVoices[i].state == VoiceState::Pressed) &&
            (mVoices[i].key == key) && (mVoices[i].time < minTime))
        {
            minTime = mVoices[i].time;
            voiceID = i;
        }
    }

    if (voiceID >= 0 && voiceID < MW_MAX_VOICES)
    {
        mVoices[voiceID].Release();

        for (const auto& module : mModules)
            module.second->OnReleaseVoice(voiceID, this);
    }
}

void Synth::ReleaseAll()
{
    Lock lock(mMutex);

    for (int i = 0; i < MW_MAX_VOICES; i++)
    {
        if (mVoices[i].state == VoiceState::Pressed)
        {
            mVoices[i].Release();

            for (const auto& module : mModules)
                module.second->OnReleaseVoice(i, this);
        }
    }
}

void Synth::KillAll()
{
    Lock lock(mMutex);

    for (int i = 0; i < MW_MAX_VOICES; i++)
        mVoices[i].state = VoiceState::Disabled;
}

void Synth::GetStats(SynthStats* stats)
{
    Lock lock(mMutex);

    stats->pressedVoices = 0;
    stats->releasedVoices = 0;
    stats->totalVoices = MW_MAX_VOICES;

    // count pressed and released voices
    for (int i = 0; i < MW_MAX_VOICES; i++)
    {
        if (mVoices[i].state == VoiceState::Pressed)
            stats->pressedVoices++;
        else if (mVoices[i].state == VoiceState::Released)
            stats->releasedVoices++;
    }
}

float FixFloat(float a)
{
    uint32 u = *((uint32*)(&a));
    if ((u & 0x7F800000) == 0) u = 0; // denormalized
    if ((u & 0x7F800000) == 0x7F800000) u = 0; // NaN
    return *((float*)(&u));
}

void Synth::SynthFrame()
{
    mOutput.Clear(0.0f);
    mMaster.ClearAllVoices();
    mMaster.output = &mOutput;

    Lock lock(mMutex);

    for (const auto& module : mModules)
        module.second->ClearAllVoices();

    // process global modules
    mMaster.Process(SYNTH_GLOBAL_VOICE_ID, this);


    for (int i = 0; i < MW_MAX_VOICES; i++)
    {
        Voice* voice = &mVoices[i];
        if (voice->state == VoiceState::Disabled) continue;

        mMaster.Process(i, this);

        if (mMaster.faded[i] && voice->state == VoiceState::Released)
            voice->state = VoiceState::Disabled;
    }
}

void Synth::Synthesize(float** output, size_t samplesNum)
{
    for (size_t i = 0; i < samplesNum; i++)
    {
        if (mValidSamples == 0)
        {
            SynthFrame();
            mValidSamples = frameSize;
        }

        output[0][i] = FixFloat((float)mOutput.mData[2 * (frameSize - mValidSamples)]);
        output[1][i] = FixFloat((float)mOutput.mData[2 * (frameSize - mValidSamples) + 1]);

        mValidSamples--;
    }
}

bool Synth::ParseLinkFromConfig(const YAML::Node& node, std::string& errorStr)
{
    Module* destModule = nullptr;
    std::string srcStr, destStr, destPortStr;
    double scale = 1.0, offset = 0.0;

    for (const auto& n : node)
    {
        const std::string& str = n.first.as<std::string>();

        if (str == "src")
        {
            srcStr = n.second.as<std::string>();
        }
        else if (str == "dest")
        {
            destStr = n.second.as<std::string>();
        }
        else if (str == "destPort")
        {
            destPortStr = n.second.as<std::string>();
        }
        else if (str == "scale")
        {
            scale = n.second.as<double>();
        }
        else if (str == "offset")
        {
            offset = n.second.as<double>();
        }
        else
        {
            errorStr += "Invalid link parameter: '" + str + "'\n";
            return false;
        }
    }

    if (destPortStr.empty())
    {
        errorStr += "Link destination not specified\n";
        return false;
    }

    if (mModules.find(srcStr) == mModules.end())
    {
        errorStr += "Source module '" + srcStr + "' does not exist.\n";
        return false;
    }

    if (destStr != "master")
    {
        if (mModules.find(destStr) == mModules.end())
        {
            errorStr += "Destination module '" + destStr + "' does not exist.\n";
            return false;
        }
        destModule = mModules[destStr].get();
    }

    Link* link = CreateLink(mModules[srcStr].get(), "output", destModule, destPortStr.c_str());
    link->SetScaling(offset, scale);

    return true;
}

bool Synth::LoadConfig(const std::string& str)
{
    YAML::Node config = YAML::Load(str);
    std::string errorStr;
    bool success = true;

    Lock lock(mMutex);

    Clear();

    // first parse modules
    for (const auto& n : config)
    {
        std::string moduleName = n.first.as<std::string>();

        if (moduleName == "osc")
        {
            std::unique_ptr<Oscillator> osc(new Oscillator);
            if (osc->ParseFromConfig(n.second, errorStr))
                AddModule(osc.release());
            else
                success = false;
        }
        else if (moduleName == "env")
        {
            std::unique_ptr<EnvelopeGen> env(new EnvelopeGen);
            if (env->ParseFromConfig(n.second, errorStr))
                AddModule(env.release());
            else
                success = false;
        }
        else if (moduleName == "filter")
        {
            std::unique_ptr<Filter> filter(new Filter);
            if (filter->ParseFromConfig(n.second, errorStr))
                AddModule(filter.release());
            else
                success = false;
        }
        else if (moduleName == "lfo")
        {
            std::unique_ptr<LFO> lfo(new LFO);
            if (lfo->ParseFromConfig(n.second, errorStr))
                AddModule(lfo.release());
            else
                success = false;
        }
        else if (moduleName == "noise")
        {
            std::unique_ptr<NoiseGen> noise(new NoiseGen);
            if (noise->ParseFromConfig(n.second, errorStr))
                AddModule(noise.release());
            else
                success = false;
        }
        else if (moduleName != "link")
        {
            success = false;
            errorStr += "Invalid module type: '" + moduleName + "'\n";
        }
    }

    // then parse links
    for (const auto& n : config)
    {
        std::string moduleName = n.first.as<std::string>();

        if (moduleName == "link")
        {
            success &= ParseLinkFromConfig(n.second, errorStr);
        }
    }

    lock.unlock();

    if (!success)
    {
        // TODO: temporary
        MessageBoxA(0, errorStr.c_str(), "Failed to load preset", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

} // namespace mvSynth
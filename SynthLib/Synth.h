#pragma once

#include "Defines.h"
#include "Frame.h"
#include "MasterModule.h"
#include "Voice.h"
#include "Wavetable/Interpolator.hpp"

#include <mutex>

namespace mvSynth {

struct SynthStats
{
    int pressedVoices;
    int releasedVoices;
    int totalVoices;
};

// The main sythesiser class
class Synth
{
    friend class Module;
    typedef std::unique_lock<std::mutex> Lock;

    SampleType mSampleRate;
    SampleType mSampleRateInv;

    Interpolator mWaveTableInterpolator;

    std::unique_ptr<Voice[]> mVoices;

    // valid samples left in current frame
    size_t mValidSamples;
    size_t frameSize;
    Frame<SampleType> mOutput;

    // TODO: move to separate class
    uint32 mSeedX;
    uint32 mSeedY;
    uint32 mSeedZ;
    uint32 mSeedW;
    uint32 mSeedT;

    MasterModule mMaster;
    std::map<std::string, std::unique_ptr<Module>> mModules;
    std::set<std::unique_ptr<Link>> mLinks;

    std::mutex mMutex;

    // synthesisze internal frame
    void SynthFrame();

    bool ParseLinkFromConfig(const YAML::Node& node, std::string& errorStr);

public:
    Synth();

    uint32 RandU();
    SampleType RandF();
    SampleType RandD();

    /**
     * Remove all modules and links.
     */
    void Clear();

    /**
     * Add a module to the synth.
     * The module pointer is managed by Synth class - do not free it.
     */
    void AddModule(Module *module);

    /**
     * Create a new link between modules.
     * @returns Link pointer. The pointer is managed by Synth class - do not free it.
     */
    Link* CreateLink(Module *src, const char* srcPortName, Module *dest, const char* destPorttName);

    void SetFrameSize(size_t newSize);
    size_t GetFrameSize() const;

    void SetSampleRate(const SampleType sampleRate);

    __forceinline SampleType GetSampleRate() const
    {
        return mSampleRate;
    }

    __forceinline SampleType GetSampleRateInv() const
    {
        return mSampleRateInv;
    }

    __forceinline const Interpolator& GetInterpolator() const
    {
        return mWaveTableInterpolator;
    }

    void GetStats(SynthStats* stats);

    void PressKey(int key, SampleType velocity, SampleType pan);
    void ReleaseKey(int key);
    void ReleaseAll();
    void KillAll();

    void Synthesize(float** output, size_t samplesNum);

    /**
     * Parse and create preset from YAML string.
     */
    bool LoadConfig(const std::string& str, std::string* messages);
};

} // namespace mvSynth
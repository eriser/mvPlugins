#pragma once

#include "Module.h"

namespace mvSynth {

struct EnvelopeGenVoiceData
{
    SampleType lastValue;
    SampleType releaseVolume; // envelope value at key release
    SampleType time;          // time since key press or release

    bool postRelease;

    EnvelopeGenVoiceData()
    {
        releaseVolume = 0.0;
        time = 0.0;
        postRelease = false;
    }

    void Press();
    void Release();
};

/**
 * Simple ADSR envelope generator
 */
class EnvelopeGen final : public Module
{
private:
    friend class Synth;
    friend class Modulable;

    SampleType mAttackInv;
    SampleType mDecayInv;
    SampleType mReleaseInv;

    SampleType mAttack;
    SampleType mDecay;
    SampleType mSustain;
    SampleType mRelease;

    EnvelopeGenVoiceData mVoiceData[MW_MAX_VOICES];

public:
    EnvelopeGen();
    bool ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr);
    void OnProcess(int voiceID, Synth* synth);
    void OnInitVoice(int voiceID, Synth* synth, SampleType freq);
    void OnReleaseVoice(int voiceID, Synth* synth);

    void Set(SampleType attack, SampleType decay, SampleType sustain, SampleType release);
};

} // namespace mvSynth
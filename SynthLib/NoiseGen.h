#pragma once

#include "Module.h"

namespace mvSynth {

#define MW_NOISE_GEN_NUM 2


//#define MW_FILTER_IN_FREQUENCY  1 // cutoff frequency

struct NoiseGenVoiceData
{
    bool faded;
    SampleType lastValue[2];
    int counter;
};

enum class NoiseDistribution
{
    // a sample may have any value from -1 to 1 range
    Uniform,

    // a sample may only be -1 or 1
    Binary,

    // normal (gaussian) distribution of samples magnitude
    Gaussian
};

/**
 * Noise generator module.
 */
class NoiseGen final : public Module
{
    NoiseGenVoiceData mVoiceData[MW_MAX_VOICES];

public:
    // how many times (samples) should be a single generated value repeated?
    int hold;
    NoiseDistribution distribution;

    NoiseGen();
    bool ParseFromConfig(const YAML::Node& node, std::string& errorStr);
    void OnProcess(int voiceID, Synth* synth);
    void OnInitVoice(int voiceID, Synth* synth, SampleType freq);
};

} // namespace mvSynth
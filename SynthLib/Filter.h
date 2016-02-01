#pragma once

#include "Module.h"

#define MW_FILTER_MAX_ORDER 5

namespace mvSynth {

/**
 * Single biquad filter data
 */
struct FilterBiquadData
{
    SampleType y_1[2]; // previous sample
    SampleType y_2[2]; // before-previous sample
    SampleType x_1[2]; // previous sample
    SampleType x_2[2]; // before-previous sample
};

struct FilterVoiceData
{
    FilterBiquadData biquads[MW_FILTER_MAX_ORDER];
};


// cutoff frequency
#define MW_FILTER_IN_FREQUENCY 1

enum class FilterType
{
    Bypass = 0,
    Lowpass,
    Highpass,
    Bandpass,
    Notch,

    /**
     * TODO: lowshelf, highshelf, comb, etc.
     */
};

/**
 * Class describing RBJ filter unit
 */
class Filter final : public Module
{
    FilterVoiceData mVoiceData[MW_MAX_VOICES];

public:
    uint32 order;
    FilterType type;

    // Cutoff frequency. Cutoff equal to 1.0 means 22050 Hz.
    SampleType cutoff;

    // resonance factor
    SampleType resonance;

    Filter();
    bool ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr);
    void OnInitVoice(int voiceID, Synth* synth, SampleType freq);
    void OnProcess(int voiceID, Synth* synth);
};

} // namespace mvSynth
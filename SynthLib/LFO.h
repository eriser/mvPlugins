#pragma once

#include "Module.h"

namespace mvSynth {

/**
 * Prefedined wave shapes for LFO
 */
enum class LFOShape
{
    Sine = 0,
    Square,
    Saw,
    Triangle,
};

/**
 * Low Frequency Oscillator
 */
class LFO final : public Module
{
    SampleType mPhase;

public:
    LFOShape shape;
    SampleType freq;

    LFO();
    bool ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr);
    void OnProcess(int voiceID, Synth* synth);
};

} // namespace mvSynth
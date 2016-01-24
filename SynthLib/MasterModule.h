#pragma once

#include "Module.h"

namespace mvSynth {

class MasterModule final : public Module
{
public:
    Frame<SampleType>* output;
    bool faded[MW_MAX_VOICES];

    MasterModule();
    void OnProcess(int voiceID, Synth* synth);
    void OnInitVoice(int voiceID, Synth* synth, SampleType freq);
};

} // namespace mvSynth
#pragma once

#include "../SynthLib/Synth.h"

class SynthGUI
    : public Component
{
public:
    SynthGUI(mvSynth::Synth* synth);
    ~SynthGUI();

    void paint(Graphics& g) override;
    void resized() override;

private:
    mvSynth::Synth* synth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthGUI)
};
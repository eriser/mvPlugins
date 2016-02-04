#pragma once

#include "../SynthLib/Synth.h"

class OscillatorComponent
    : public Component
{
public:
    OscillatorComponent();
    ~OscillatorComponent();

    void paint(Graphics& g) override;
    void resized() override;

private:
    mvSynth::Synth* synth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorComponent)
};
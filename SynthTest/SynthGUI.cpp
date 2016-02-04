#include "JuceHeaders.h"
#include "SynthGUI.h"

SynthGUI::SynthGUI(mvSynth::Synth* synth)
    : synth(synth)
{
}

SynthGUI::~SynthGUI()
{
}

void SynthGUI::paint(Graphics& g)
{
    g.fillAll(Colours::white);
}

void SynthGUI::resized()
{

}
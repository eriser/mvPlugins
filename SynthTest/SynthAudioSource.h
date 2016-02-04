#pragma once

#include "../SynthLib/Synth.h"

class SynthAudioSource : public AudioSource
{
    // this collects real-time midi messages from the midi input device, and
    // turns them into blocks that we can process in our audio callback
    MidiMessageCollector midiCollector;

    // this represents the state of which keys on our on-screen keyboard are held
    // down. When the mouse is clicked on the keyboard component, this object also
    // generates midi messages for this, which we can pass on to our synth.
    MidiKeyboardState& keyboardState;

    mvSynth::Synth* synth;

    void handleMidiEvent(const MidiMessage& m);

public:
    SynthAudioSource(MidiKeyboardState& keyState, mvSynth::Synth* synth);

    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override;
    void releaseResources() override;
    void render(AudioBuffer<float>* buffer, int startSample, int numSamples);
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

    MidiMessageCollector* getMidiCollector();
};
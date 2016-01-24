#pragma once

#include "../SynthLib/Synth.h"

struct SynthAudioSource : public AudioSource
{
    SynthAudioSource(MidiKeyboardState& keyState);
    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override;
    void releaseResources() override;
    void handleMidiEvent(const MidiMessage& m);
    void render(AudioBuffer<float>* buffer, int startSample, int numSamples);
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

    // this collects real-time midi messages from the midi input device, and
    // turns them into blocks that we can process in our audio callback
    MidiMessageCollector midiCollector;

    // this represents the state of which keys on our on-screen keyboard are held
    // down. When the mouse is clicked on the keyboard component, this object also
    // generates midi messages for this, which we can pass on to our synth.
    MidiKeyboardState& keyboardState;

    mvSynth::Synth synth;
};

class SynthComponent
    : public Component
    , private FilenameComponentListener
    , private ButtonListener
{
public:
    SynthComponent(AudioDeviceManager& audioDeviceManager);
    ~SynthComponent();
    void paint(Graphics& g) override;
    void resized() override;
    void filenameComponentChanged(FilenameComponent*) override;
    void loadPreset(const std::string& presetStr);
    void buttonClicked(Button* button) override;

private:
    AudioDeviceManager& deviceManager;
    MidiKeyboardState keyboardState;
    AudioSourcePlayer audioSourcePlayer;
    SynthAudioSource synthAudioSource;
    MidiKeyboardComponent keyboardComponent;

    // TODO: create custom tokenizer
    CPlusPlusCodeTokeniser tokenizer;
    FilenameComponent fileChooser;
    CodeDocument codeDocument;
    ScopedPointer<CodeEditorComponent> editor;

    ScopedPointer<Button> realoadButton;
    TextEditor outputBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthComponent)
};
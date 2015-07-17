#pragma once

struct SineWaveSound : public SynthesiserSound
{
    SineWaveSound() { }

    bool appliesToNote(int /*midiNoteNumber*/) override
    {
        return true;
    }

    bool appliesToChannel(int /*midiChannel*/) override
    {
        return true;
    }
};

struct SineWaveVoice : public SynthesiserVoice
{
    SineWaveVoice();
    bool canPlaySound(SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity,
                   SynthesiserSound*, int /*currentPitchWheelPosition*/) override;
    void stopNote(float /*velocity*/, bool allowTailOff) override;
    void pitchWheelMoved(int /*newValue*/) override;
    void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override;
    void renderNextBlock(AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;

private:
    double currentAngle, angleDelta, level, tailOff;
};

struct SynthAudioSource : public AudioSource
{
    SynthAudioSource(MidiKeyboardState& keyState);
    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;

    // this collects real-time midi messages from the midi input device, and
    // turns them into blocks that we can process in our audio callback
    MidiMessageCollector midiCollector;

    // this represents the state of which keys on our on-screen keyboard are held
    // down. When the mouse is clicked on the keyboard component, this object also
    // generates midi messages for this, which we can pass on to our synth.
    MidiKeyboardState& keyboardState;

    // the synth itself!
    Synthesiser synth;
};

class SynthComponent : public Component
{
public:
    SynthComponent(AudioDeviceManager& audioDeviceManager);
    ~SynthComponent();
    void paint(Graphics& g) override;
    void resized() override;

private:
    AudioDeviceManager& deviceManager;
    MidiKeyboardState keyboardState;
    AudioSourcePlayer audioSourcePlayer;
    SynthAudioSource synthAudioSource;
    MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthComponent)
};
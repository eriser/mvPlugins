#pragma once

class MvSpectrographMainWindow;

class MvSpectrographAudioProcessor : public AudioProcessor
{
public:
    MvSpectrographAudioProcessor();
    ~MvSpectrographAudioProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(AudioSampleBuffer&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;

    void getStateInformation(MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool setPreferredBusArrangement(bool isInputBus, int busIndex,
                                    const AudioChannelSet& preferred) override;

private:
    AudioParameterFloat* gainParam;
    AudioParameterBool* showWindowParam;

    ScopedPointer<MvSpectrographMainWindow> mMainWindow;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MvSpectrographAudioProcessor)
};

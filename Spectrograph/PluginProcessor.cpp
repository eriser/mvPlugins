#include "JuceHeaders.h"
#include "PluginProcessor.h"
#include "../Common/GenericEditor.h"
#include "MainWindow.h"
#include "Common.h"

MvSpectrographAudioProcessor::MvSpectrographAudioProcessor()
{
    mMainWindow = new MvSpectrographMainWindow;

    addParameter(gainParam = new AudioParameterFloat("gain", "Gain", -20.0f, 20.0f, 0.0f));
    addParameter(showWindowParam = new AudioParameterBool("showWindow", "Show Window", true));
}

MvSpectrographAudioProcessor::~MvSpectrographAudioProcessor()
{
}

const String MvSpectrographAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MvSpectrographAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool MvSpectrographAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool MvSpectrographAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double MvSpectrographAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MvSpectrographAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MvSpectrographAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MvSpectrographAudioProcessor::setCurrentProgram(int index)
{
    UNUSED(index);
}

const String MvSpectrographAudioProcessor::getProgramName(int index)
{
    UNUSED(index);
    return String();
}

void MvSpectrographAudioProcessor::changeProgramName(int index, const String& newName)
{
    UNUSED(index);
    UNUSED(newName);
}

void MvSpectrographAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    UNUSED(sampleRate);
    UNUSED(samplesPerBlock);
}

void MvSpectrographAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void MvSpectrographAudioProcessor::processBlock(AudioSampleBuffer& buffer, MidiBuffer&)
{
    // TODO
}

bool MvSpectrographAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* MvSpectrographAudioProcessor::createEditor()
{
    return new GenericEditor(*this);
}

void MvSpectrographAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    UNUSED(destData);
}

void MvSpectrographAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    UNUSED(data);
    UNUSED(sizeInBytes);
}

bool MvSpectrographAudioProcessor::setPreferredBusArrangement(bool isInputBus, int busIndex,
                                                              const AudioChannelSet& preferred)
{
    const int numChannels = preferred.size();

    // do not allow disabling channels
    if (numChannels == 0) return false;

    // always have the same channel layout on both input and output on the main bus
    if (!AudioProcessor::setPreferredBusArrangement(!isInputBus, busIndex, preferred))
        return false;

    return AudioProcessor::setPreferredBusArrangement(isInputBus, busIndex, preferred);
}

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MvSpectrographAudioProcessor();
}

#include "JuceHeaders.h"
#include "MidiSetup.h"
#include "SynthAudioSource.h"
#include "SynthTextEditorComponent.h"
#include "SynthGUI.h"

class MainContentComponent : public Component
{
public:
    MainContentComponent()
        : tabs(TabbedButtonBar::Orientation::TabsAtTop)
        , keyboardComponent(keyboardState, MidiKeyboardComponent::horizontalKeyboard)
    {
        deviceManager.initialise(2, 2, 0, true, String(), 0);

        synthAudioSource = new SynthAudioSource(keyboardState, &synth);

        addAndMakeVisible(keyboardComponent);
        audioSourcePlayer.setSource(synthAudioSource.get());
        deviceManager.addAudioCallback(&audioSourcePlayer);
        deviceManager.addMidiInputCallback(String::empty, synthAudioSource->getMidiCollector());

        Component* midiSetupComp = new MidiSetupComponent(deviceManager);
        Component* audioSetupComp = new AudioDeviceSelectorComponent(
            deviceManager, 0, 256, 0, 256, false, false, true, false);

        tabs.addTab("GUI Editor", Colours::lightgrey, new SynthGUI(&synth), true);
        tabs.addTab("Text Editor", Colours::lightgrey, new SynthTextEditorComponent(&synth), true);
        tabs.addTab("Audio setup", Colours::whitesmoke, audioSetupComp, true);
        tabs.addTab("MIDI setup", Colours::whitesmoke, midiSetupComp, true);


        setSize(600, 600);
        addAndMakeVisible(tabs);
        addAndMakeVisible(keyboardComponent);
    }

    ~MainContentComponent()
    {
        audioSourcePlayer.setSource(nullptr);
        deviceManager.removeMidiInputCallback(String::empty, synthAudioSource->getMidiCollector());
        deviceManager.removeAudioCallback(&audioSourcePlayer);

        tabs.clearTabs();
    }

    void resized() override
    {
        Rectangle<int> rect(getLocalBounds());
        keyboardComponent.setBounds(rect.removeFromBottom(64));
        tabs.setBounds(rect);
    }

private:
    mvSynth::Synth synth;

    AudioDeviceManager deviceManager;
    MidiKeyboardState keyboardState;
    AudioSourcePlayer audioSourcePlayer;
    ScopedPointer<SynthAudioSource> synthAudioSource;
    MidiKeyboardComponent keyboardComponent;

    TabbedComponent tabs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

Component* createMainContentComponent()
{
    return new MainContentComponent();
};

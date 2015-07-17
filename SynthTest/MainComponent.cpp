#include "JuceHeaders.h"
#include "MidiSetup.h"
#include "SynthComponent.h"

class MainContentComponent : public TabbedComponent
{
public:
    MainContentComponent()
        : TabbedComponent(TabbedButtonBar::TabsAtTop)
    {
        sharedAudioDeviceManager.initialise(2, 2, 0, true, String(), 0);

        Component* midiSetupComp = new MidiSetupComponent(sharedAudioDeviceManager);
        Component* synthComp = new SynthComponent(sharedAudioDeviceManager);
        Component* audioSetupComp = new AudioDeviceSelectorComponent(
            sharedAudioDeviceManager, 0, 256, 0, 256, false, false, true, false);

        addTab("Synth", Colours::whitesmoke, synthComp, true);
        addTab("Audio setup", Colours::whitesmoke, audioSetupComp, true);
        addTab("MIDI setup", Colours::whitesmoke, midiSetupComp, true);

        setSize(600, 600);
    }

    ~MainContentComponent()
    {
        clearTabs();
    }

private:
    AudioDeviceManager sharedAudioDeviceManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

Component* createMainContentComponent()
{
    return new MainContentComponent();
};

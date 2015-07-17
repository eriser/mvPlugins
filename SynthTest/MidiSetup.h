#pragma once

/** Simple list box that just displays a StringArray. */
class MidiLogListBoxModel : public ListBoxModel
{
public:
    MidiLogListBoxModel(const Array<MidiMessage>& list);
    int getNumRows() override;
    void paintListBoxItem(int row, Graphics& g, int width, int height, bool rowIsSelected) override;

private:
    const Array<MidiMessage>& midiMessageList;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLogListBoxModel)
};


class MidiSetupComponent
    : public Component
    , private ComboBox::Listener
    , private MidiInputCallback
    , private MidiKeyboardStateListener
    , private AsyncUpdater
{
public:
    MidiSetupComponent(AudioDeviceManager& audioDeviceManager);
    ~MidiSetupComponent();
    void paint(Graphics& g) override;
    void resized() override;

private:
    AudioDeviceManager& deviceManager;
    ComboBox midiInputList;
    Label midiInputListLabel;
    int lastInputIndex;
    bool isAddingFromMidiInput;
    MidiKeyboardState keyboardState;

    ListBox messageListBox;
    Array<MidiMessage> midiMessageList;
    MidiLogListBoxModel midiLogListBoxModel;

    void setMidiInput(int index);

    void comboBoxChanged(ComboBox* box) override;

    // These methods handle callbacks from the midi device + on-screen keyboard..
    void handleIncomingMidiMessage(MidiInput*, const MidiMessage& message) override;
    void handleNoteOn(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    // This is used to dispach an incoming message to the message thread
    struct IncomingMessageCallback : public CallbackMessage
    {
        IncomingMessageCallback(MidiSetupComponent* d, const MidiMessage& m)
            : demo(d), message(m)
        {
        }

        void messageCallback() override
        {
            if (demo != nullptr)
                demo->addMessageToList(message);
        }

        Component::SafePointer<MidiSetupComponent> demo;
        MidiMessage message;
    };

    void postMessageToList(const MidiMessage& message);
    void addMessageToList(const MidiMessage& message);
    void handleAsyncUpdate() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiSetupComponent)
};
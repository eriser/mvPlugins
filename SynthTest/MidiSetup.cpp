#include "JuceHeaders.h"
#include "MidiSetup.h"

static String getMidiMessageDescription(const MidiMessage& m)
{
    if (m.isNoteOn())           return "Note on " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
    if (m.isNoteOff())          return "Note off " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
    if (m.isProgramChange())    return "Program change " + String(m.getProgramChangeNumber());
    if (m.isPitchWheel())       return "Pitch wheel " + String(m.getPitchWheelValue());
    if (m.isAftertouch())       return "After touch " + MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " + String(m.getAfterTouchValue());
    if (m.isChannelPressure())  return "Channel pressure " + String(m.getChannelPressureValue());
    if (m.isAllNotesOff())      return "All notes off";
    if (m.isAllSoundOff())      return "All sound off";
    if (m.isMetaEvent())        return "Meta event";

    if (m.isController())
    {
        String name(MidiMessage::getControllerName(m.getControllerNumber()));

        if (name.isEmpty())
            name = "[" + String(m.getControllerNumber()) + "]";

        return "Controler " + name + ": " + String(m.getControllerValue());
    }

    return String::toHexString(m.getRawData(), m.getRawDataSize());
}


MidiLogListBoxModel::MidiLogListBoxModel(const Array<MidiMessage>& list)
    : midiMessageList(list)
{
}

int MidiLogListBoxModel::getNumRows()
{
    return midiMessageList.size();
}

void MidiLogListBoxModel::paintListBoxItem(int row, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(Colours::blue.withAlpha(0.2f));

    if (isPositiveAndBelow(row, midiMessageList.size()))
    {
        g.setColour(Colours::black);

        const MidiMessage& message = midiMessageList.getReference(row);
        double time = message.getTimeStamp();

        g.drawText(String::formatted("%02d:%02d:%02d",
                                        ((int)(time / 3600.0)) % 24,
                                        ((int)(time / 60.0)) % 60,
                                        ((int)time) % 60)
                    + "  -  " + getMidiMessageDescription(message),
                    Rectangle<int>(width, height).reduced(4, 0),
                    Justification::centredLeft, true);
    }
}

MidiSetupComponent::MidiSetupComponent(AudioDeviceManager& audioDeviceManager)
    : deviceManager(audioDeviceManager)
    , lastInputIndex(0)
    , isAddingFromMidiInput(false)
    , midiLogListBoxModel(midiMessageList)
{
    setOpaque(true);

    // MIDI Inputs
    addAndMakeVisible(midiInputListLabel);
    midiInputListLabel.setText("MIDI Input:", dontSendNotification);
    midiInputListLabel.attachToComponent(&midiInputList, true);

    addAndMakeVisible(midiInputList);
    midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
    const StringArray midiInputs(MidiInput::getDevices());
    midiInputList.addItemList(midiInputs, 1);
    midiInputList.addListener(this);

    // find the first enabled device and use that by default
    for (int i = 0; i < midiInputs.size(); ++i)
    {
        if (deviceManager.isMidiInputEnabled(midiInputs[i]))
        {
            setMidiInput(i);
            break;
        }
    }

    // if no enabled devices were found just use the first one in the list
    if (midiInputList.getSelectedId() == 0)
        setMidiInput(0);

    addAndMakeVisible(messageListBox);
    messageListBox.setModel(&midiLogListBoxModel);
    messageListBox.setColour(ListBox::backgroundColourId, Colour(0x32ffffff));
    messageListBox.setColour(ListBox::outlineColourId, Colours::black);
}

MidiSetupComponent::~MidiSetupComponent()
{
    keyboardState.removeListener(this);
    deviceManager.removeMidiInputCallback(MidiInput::getDevices()[midiInputList.getSelectedItemIndex()], this);
    midiInputList.removeListener(this);
}

void MidiSetupComponent::paint(Graphics& g)
{
    g.fillAll(Colours::white);
}

void MidiSetupComponent::resized()
{
    Rectangle<int> area(getLocalBounds());
    midiInputList.setBounds(area.removeFromTop(36).removeFromRight(getWidth() - 150).reduced(8));
    messageListBox.setBounds(area.reduced(8));
}

void MidiSetupComponent::setMidiInput(int index)
{
    const StringArray list(MidiInput::getDevices());

    deviceManager.removeMidiInputCallback(list[lastInputIndex], this);

    const String newInput(list[index]);

    if (!deviceManager.isMidiInputEnabled(newInput))
        deviceManager.setMidiInputEnabled(newInput, true);

    deviceManager.addMidiInputCallback(newInput, this);
    midiInputList.setSelectedId(index + 1, dontSendNotification);

    lastInputIndex = index;
}

void MidiSetupComponent::comboBoxChanged(ComboBox* box)
{
    if (box == &midiInputList)
        setMidiInput(midiInputList.getSelectedItemIndex());
}

// These methods handle callbacks from the midi device + on-screen keyboard..
void MidiSetupComponent::handleIncomingMidiMessage(MidiInput*, const MidiMessage& message)
{
    const ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
    keyboardState.processNextMidiEvent(message);
    postMessageToList(message);
}

void MidiSetupComponent::handleNoteOn(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    if (!isAddingFromMidiInput)
    {
        MidiMessage m(MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity));
        m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
        postMessageToList(m);
    }
}

void MidiSetupComponent::handleNoteOff(MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    if (!isAddingFromMidiInput)
    {
        MidiMessage m(MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity));
        m.setTimeStamp(Time::getMillisecondCounterHiRes() * 0.001);
        postMessageToList(m);
    }
}

void MidiSetupComponent::postMessageToList(const MidiMessage& message)
{
    (new IncomingMessageCallback(this, message))->post();
}

void MidiSetupComponent::addMessageToList(const MidiMessage& message)
{
    midiMessageList.add(message);
    triggerAsyncUpdate();
}

void MidiSetupComponent::handleAsyncUpdate()
{
    messageListBox.updateContent();
    messageListBox.scrollToEnsureRowIsOnscreen(midiMessageList.size() - 1);
    messageListBox.repaint();
}
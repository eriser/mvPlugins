#include "JuceHeaders.h"
#include "SynthTextEditorComponent.h"

#include <fstream>
#include <streambuf>

SynthTextEditorComponent::SynthTextEditorComponent(mvSynth::Synth* synth)
    : fileChooser("File", File::nonexistent, false, false, false, "*.yaml", String(),
                  "Choose a YAML preset file to open it in the editor")
    , synth(synth)
{
    // output box
    addAndMakeVisible(outputBox);
    outputBox.setReadOnly(true);
    outputBox.setMultiLine(true);
    outputBox.setFont(Font(Font::getDefaultMonospacedFontName(), 10.0f, Font::plain));

    // load init preset
    std::ifstream presetFile("../SynthPresets/init.yaml");
    std::string str((std::istreambuf_iterator<char>(presetFile)), std::istreambuf_iterator<char>());
    loadPreset(str);

    addAndMakeVisible(realoadButton = new TextButton("Reload", "Load preset form the editor"));
    realoadButton->addListener(this);

    // editor box
    addAndMakeVisible(editor = new CodeEditorComponent(codeDocument, &tokenizer));
    editor->loadContent(str);
    editor->setTabSize(4, true);

    setOpaque(true);
    setSize(640, 480);

    addAndMakeVisible(fileChooser);
    fileChooser.addListener(this);
}

SynthTextEditorComponent::~SynthTextEditorComponent()
{
}

void SynthTextEditorComponent::paint(Graphics& g)
{
    g.fillAll(Colours::white);
}

void SynthTextEditorComponent::resized()
{
    Rectangle<int> rect(getLocalBounds());

    Rectangle<int> tempRect = rect.removeFromBottom(64);
    realoadButton->setBounds(tempRect.removeFromLeft(64).reduced(4));
    outputBox.setBounds(tempRect);

    fileChooser.setBounds(rect.removeFromTop(25));
    editor->setBounds(rect);
}

void SynthTextEditorComponent::buttonClicked(Button* button)
{
    if (button == realoadButton)
    {
        std::string str = editor->getTextInRange(Range<int>(0, INT_MAX)).toStdString();

        loadPreset(str);
    }
}

void SynthTextEditorComponent::loadPreset(const std::string& presetStr)
{
    synth->KillAll();

    std::string messages;
    if (synth->LoadConfig(presetStr, &messages))
        outputBox.setText("Preset loaded:\n" + messages);
    else
        outputBox.setText("Failed to load preset:\n" + messages);
}

void SynthTextEditorComponent::filenameComponentChanged(FilenameComponent*)
{
    String str = fileChooser.getCurrentFile().loadFileAsString();
    editor->loadContent(str);
    loadPreset(str.toStdString());
}
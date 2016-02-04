#pragma once

#include "../SynthLib/Synth.h"

class SynthTextEditorComponent
    : public Component
    , private FilenameComponentListener
    , private ButtonListener
{
public:
    SynthTextEditorComponent(mvSynth::Synth* synth);
    ~SynthTextEditorComponent();

    void paint(Graphics& g) override;
    void resized() override;
    void filenameComponentChanged(FilenameComponent*) override;
    void loadPreset(const std::string& presetStr);
    void buttonClicked(Button* button) override;

private:
    mvSynth::Synth* synth;

    // TODO: create custom tokenizer
    CPlusPlusCodeTokeniser tokenizer;

    CodeDocument codeDocument;
    ScopedPointer<CodeEditorComponent> editor;
    ScopedPointer<Button> realoadButton;
    TextEditor outputBox;

    FilenameComponent fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthTextEditorComponent)
};
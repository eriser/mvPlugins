#pragma once

class SpectrographComponent
    : public Component
    , private OpenGLRenderer
{
};

class MvSpectrographMainWindow : public DocumentWindow
{
public:
    MvSpectrographMainWindow();
    void closeButtonPressed() override;

private:

    ComponentBoundsConstrainer constrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MvSpectrographMainWindow)
};

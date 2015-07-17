#include "JuceHeaders.h"
#include "MainWindow.h"

MvSpectrographMainWindow::MvSpectrographMainWindow()
    : DocumentWindow("Spectrograph", Colours::lightgrey, DocumentWindow::allButtons)
{
    setContentOwned(new Component("test"), false);

    setSize(800, 600);
    centreWithSize(getWidth(), getHeight());
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setVisible(true);

    constrainer.setMinimumSize(400, 400);
    setConstrainer(&constrainer);
}

void MvSpectrographMainWindow::closeButtonPressed()
{
    setVisible(false);
}
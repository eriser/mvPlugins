#include "stdafx.h"
#include "MasterModule.h"
#include "Synth.h"

namespace mvSynth {

MasterModule::MasterModule()
{
    mModuleType = ModuleType::PerVoice;
    AddPort(PortType::Input, "input", 2);
}

void MasterModule::OnInitVoice(int voiceID, Synth* synth, SampleType freq)
{
    UNUSED(freq);
    UNUSED(synth);

    faded[voiceID] = false;
}

void MasterModule::OnProcess(int voiceID, Synth* synth)
{
    const SampleType fadeOutTreshold = 1.0e-5; // TODO: should be as config parameter
    bool canFadeOut = true;

    for (size_t i = 0; i < synth->GetFrameSize(); i++)
    {
        double left = mInputs[0].frame[2 * i];
        double right = mInputs[0].frame[2 * i + 1];

        if ((Abs(left) > fadeOutTreshold) || (Abs(right) > fadeOutTreshold))
            canFadeOut = false;
    }

    if (canFadeOut)
        faded[voiceID] = true;

    (*output) += mInputs[0].frame;
}

} // namespace mvSynth
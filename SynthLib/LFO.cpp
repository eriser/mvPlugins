#include "stdafx.h"
#include "LFO.h"
#include "Synth.h"

namespace mvSynth {

LFO::LFO()
{
    // TODO: per voice LFO

    AddPort(PortType::Output, "output", 1);
    mModuleType = ModuleType::Global;

    mPhase = 0.0;
    freq = 1.0;
    shape = LFOShape::Sine;
}

bool LFO::ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr)
{
    UNUSED(synth);

    for (const auto& n : node)
    {
        const std::string& str = n.first.as<std::string>();

        if (str == "name")
        {
            mName = n.second.as<std::string>();
        }
        else if (str == "shape")
        {
            const std::string& shapeStr = n.second.as<std::string>();
            if (shapeStr == "sine")
                shape = LFOShape::Sine;
            else if (shapeStr == "square")
                shape = LFOShape::Square;
            else if (shapeStr == "saw")
                shape = LFOShape::Saw;
            else
            {
                errorStr += "Invalid LFO shape '" + str + "'\n";
                return false;
            }
        }
        else if (str == "freq")
        {
            freq = n.second.as<double>();
        }
        else
        {
            errorStr += "Invalid parameter '" + str + "' in LFO module.\n";
            return false;
        }
    }

    return true;
}

void LFO::OnProcess(int voiceID, Synth* synth)
{
    UNUSED(voiceID);

    for (size_t i = 0; i < synth->GetFrameSize(); i++)
    {
        double value = 0.0;

        switch (shape)
        {
        case LFOShape::Sine:
            value = Sin(mPhase);
            break;
        case LFOShape::Square:
            value = (mPhase < M_PI) ? 1.0f : -1.0f;
            break;
        case LFOShape::Saw:
            value = -1.0f + mPhase / M_PI;
            break;
        }

        mPhase += 2.0 * M_PI * synth->GetSampleRateInv() * freq;
        mPhase = fmod(mPhase, 2.0 * M_PI);

        mOutputs[0].frame[i] = value;
    }
}

} // namespace mvSynth
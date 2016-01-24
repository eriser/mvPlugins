#include "stdafx.h"
#include "NoiseGen.h"
#include "Synth.h"

namespace mvSynth {

NoiseGen::NoiseGen()
{
    mModuleType = ModuleType::PerVoice;
    AddPort(PortType::Input, "volume", 1);
    AddPort(PortType::Input, "pan", 1);
    AddPort(PortType::Input, "stereo", 1);
    AddPort(PortType::Output, "output", 2);

    hold = 1;
    distribution = NoiseDistribution::Uniform;
}

bool NoiseGen::ParseFromConfig(const YAML::Node& node, std::string& errorStr)
{
    for (const auto& n : node)
    {
        const std::string& str = n.first.as<std::string>();

        if (str == "name")
        {
            mName = n.second.as<std::string>();
        }
        else if (str == "distr")
        {
            const std::string& distrStr = n.second.as<std::string>();
            if (distrStr == "uniform")
                distribution = NoiseDistribution::Uniform;
            else if (distrStr == "binary")
                distribution = NoiseDistribution::Binary;
            else if (distrStr == "normal")
                distribution = NoiseDistribution::Gaussian;
            else
            {
                errorStr += "Invalid noise distribution '" + str + "'\n";
                return false;
            }
        }
        else if (str == "hold")
        {
            hold = n.second.as<int>();
        }
        else
        {
            errorStr += "Invalid parameter '" + str + "' in noise generator module.\n";
            return false;
        }
    }

    return true;
}


void NoiseGen::OnProcess(int voiceID, Synth* synth)
{
    NoiseGenVoiceData &data = mVoiceData[voiceID];

    if (data.faded)
        return;

    const SampleType fadeOutTreshold = 1e-5;
    bool canFadeOut = true;

    for (size_t i = 0; i < synth->GetFrameSize(); i++)
    {
        if (data.counter >= hold)
        {
            switch (distribution)
            {
                case NoiseDistribution::Uniform:
                {
                    data.lastValue[0] = 2.0 * synth->RandD() - 1.0;
                    data.lastValue[1] = 2.0 * synth->RandD() - 1.0;
                    break;
                }

                case NoiseDistribution::Gaussian:
                {
                    double U = synth->RandD();
                    double V = synth->RandD();

                    // TODO: this is slow
                    double temp = sqrt(-2.0 * log(U));
                    data.lastValue[0] = temp * cos(2 * M_PI * V);
                    data.lastValue[1] = temp * sin(2 * M_PI * V);
                    break;
                }

                case NoiseDistribution::Binary:
                {
                    size_t U = synth->RandU();
                    data.lastValue[0] = (U & 0x1) ? 1.0f : -1.0f;
                    data.lastValue[1] = (U & 0x2) ? 1.0f : -1.0f;
                    break;
                }
            }
            data.counter = 0;
        }
        data.counter++;

        double volumeMult = mInputs[0].frame[i];

        mOutputs[0].frame[2 * i] = volumeMult * data.lastValue[0];
        mOutputs[0].frame[2 * i + 1] = volumeMult * data.lastValue[1];

        if (Abs(volumeMult) > fadeOutTreshold)
            canFadeOut = false;
    }

    if (canFadeOut)
        data.faded = true;
}

void NoiseGen::OnInitVoice(int voiceID, Synth* synth, SampleType freq)
{
    UNUSED(freq);
    UNUSED(synth);

    NoiseGenVoiceData &data = mVoiceData[voiceID];

    data.faded = false;
    data.counter = hold;
    data.lastValue[0] = data.lastValue[1] = 0.0;
}

} // namespace mvSynth
#include "stdafx.h"
#include "Filter.h"
#include "Synth.h"

namespace mvSynth {

// default maximum filter frequency (if "cutoff" param is equal to 1.0)
const double DEF_MAX_FILTER_FREQUENCY = 22050.0f;

Filter::Filter()
{
    mModuleType = ModuleType::PerVoice;
    AddPort(PortType::Input, "input", 2);
    AddPort(PortType::Input, "cutoff", 1);
    AddPort(PortType::Output, "output", 2);

    type = FilterType::Lowpass;
    order = 1;
    cutoff = 0.5;
    resonance = 0.5;
}

bool Filter::ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr)
{
    UNUSED(synth);

    for (const auto& n : node)
    {
        const std::string& str = n.first.as<std::string>();

        if (str == "name")
        {
            mName = n.second.as<std::string>();
        }
        else if (str == "type")
        {
            const std::string& typeStr = n.second.as<std::string>();
            if (typeStr == "lowpass")
                type = FilterType::Lowpass;
            else if (typeStr == "highpass")
                type = FilterType::Highpass;
            else if (typeStr == "bandpass")
                type = FilterType::Bandpass;
            else if (typeStr == "notch")
                type = FilterType::Notch;
            else
            {
                errorStr += "Invalid filter type '" + typeStr + "'\n";
                return false;
            }
        }
        else if (str == "cutoff")
        {
            cutoff = n.second.as<double>();
        }
        else if (str == "resonance")
        {
            resonance = n.second.as<double>();
        }
        else if (str == "order")
        {
            order = n.second.as<int>();
        }
        else
        {
            errorStr += "Invalid parameter '" + str + "' in filter module.\n";
            return false;
        }
    }

    return true;
}

void Filter::OnInitVoice(int voiceID, Synth* synth, SampleType freq)
{
    UNUSED(freq);
    UNUSED(synth);

    FilterVoiceData &data = mVoiceData[voiceID];

    for (int i = 0; i < MW_FILTER_MAX_ORDER; i++)
    {
        data.biquads[i].y_1[0] = data.biquads[i].y_1[1] = 0.0;
        data.biquads[i].y_2[0] = data.biquads[i].y_2[1] = 0.0;

        data.biquads[i].x_1[0] = data.biquads[i].x_1[1] = 0.0;
        data.biquads[i].x_2[0] = data.biquads[i].x_2[1] = 0.0;
    }
}


#define MIN_FREQ 0.0001
#define MAX_FREQ 0.999

void Filter::OnProcess(int voiceID, Synth* synth)
{
    FilterVoiceData &data = mVoiceData[voiceID];

    if (type == FilterType::Bypass)
    {
        mOutputs[0].frame = mInputs[0].frame;
        return;
    }

    const double freqScale = (2.0 * DEF_MAX_FILTER_FREQUENCY) * synth->GetSampleRateInv();

    for (size_t i = 0; i < synth->GetFrameSize(); i++)
    {
        double freq = cutoff + mInputs[1].frame[i];
        freq *= freqScale;

        // clamp
        if (freq < MIN_FREQ)
            freq = MIN_FREQ;
        else if (freq > MAX_FREQ)
            freq = MAX_FREQ;

        double w0 = M_PI * freq;

        // compute coefficients
        double a0, a1, a2, b0, b1, b2;

        double cos_w0 = Cos(w0); // cosine should be approximated here !!!
        double sin_w0 = sqrt(1.0 - cos_w0 * cos_w0); // Sin(w0);
        double alpha = sin_w0 / (2.0 * resonance);

        switch (type)
        {
        case FilterType::Lowpass:
            b0 = (1.0 - cos_w0) / 2.0;
            b1 = 1.0 - cos_w0;
            b2 = (1.0 - cos_w0) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cos_w0;
            a2 = 1.0 - alpha;
            break;
        case FilterType::Highpass:
            b0 = (1.0 + cos_w0) / 2.0;
            b1 = -(1.0 + cos_w0);
            b2 = (1.0 + cos_w0) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cos_w0;
            a2 = 1.0 - alpha;
            break;
        case FilterType::Bandpass:
            b0 = alpha;
            b1 = 0.0;
            b2 = -alpha;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cos_w0;
            a2 = 1.0 - alpha;
            break;
        case FilterType::Notch:
            b0 = 1.0;
            b1 = -2.0 * cos_w0;
            b2 = 1.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cos_w0;
            a2 = 1.0 - alpha;
            break;
        }

        double left = mInputs[0].frame[2 * i];
        double right = mInputs[0].frame[2 * i + 1];

        for (uint32 j = 0; j < order; j++)
        {
            FilterBiquadData &biquad = data.biquads[j];

            double y_0[2]; // current biquad output sample
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;
            a1 /= a0;
            a2 /= a0;
            y_0[0] = b0 * left + b1 * biquad.x_1[0] + b2 * biquad.x_2[0] - a1 * biquad.y_1[0] - a2 * biquad.y_2[0];
            y_0[1] = b0 * right + b1 * biquad.x_1[1] + b2 * biquad.x_2[1] - a1 * biquad.y_1[1] - a2 * biquad.y_2[1];

            biquad.x_2[0] = biquad.x_1[0];
            biquad.x_2[1] = biquad.x_1[1];
            biquad.x_1[0] = left;
            biquad.x_1[1] = right;

            biquad.y_2[0] = biquad.y_1[0];
            biquad.y_2[1] = biquad.y_1[1];
            biquad.y_1[0] = y_0[0];
            biquad.y_1[1] = y_0[1];

            left = y_0[0];
            right = y_0[1];
        }

        mOutputs[0].frame[2 * i] = left;
        mOutputs[0].frame[2 * i + 1] = right;
    }
}

} // namespace mvSynth
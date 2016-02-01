#pragma once

#include "Interpolator.hpp"
#include <vector>

namespace mvSynth {

#define ALIGNED __declspec(align(16))

#define IIR_FILTER_SIZE 12

class ALIGNED Downsampler final
{
    ALIGNED static const double a[];
    ALIGNED static const double b[];
    ALIGNED double mX[IIR_FILTER_SIZE];
    ALIGNED double mY[IIR_FILTER_SIZE];

public:
    Downsampler();
    void Reset();

    /**
    * Downsamples two samples into one using IIR lowpass filter.
    * @param input Array of 2 input samples.
    */
    double Downsample(float* input);

    double Downsample_SSE(float* input);
};

} // namespace mvSynth
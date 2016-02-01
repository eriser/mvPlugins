#pragma once

#include "Interpolator.hpp"
#include "Downsampler.hpp"

namespace mvSynth {

#define MAX_VOICES 16
#define MIPMAP_BLEND_TRESHOLD 0.98f

class WaveTableContext final
{
    friend class WaveTable;

    Downsampler mLeftDownsampler;
    Downsampler mRightDownsampler;

public:

    // TODO: better API
    int voicesNum;

    // This will be updated by Synth methods.
    // Must be in [0.0, 1.0) range.
    float phases[MAX_VOICES];

    // Voice frequencies.
    // 0.0 - 0Hz, 1.0 - sampling rate frequency, so only [0.0, 1.0) values are useful.
    float freqs[MAX_VOICES];

    float leftPanning[MAX_VOICES];
    float rightPanning[MAX_VOICES];

    WaveTableContext();

    /**
     * Clear filter history.
     */
    void Reset();
};

class WaveTable
{
    int mRootSize; // size of 0th mipmap in samples
    float mRootSizeF;

    int mMipsNum;  // total number of mipmaps = log2(m_RootSize)+1
    float** mData;

public:
    WaveTable();
    ~WaveTable();

    void Release();

    /*
        order:
        0 -> 1 sample, 1 -> 2 samples, 2 -> 4 samples, ...
    */
    int LoadData(float* pData, int order, const Interpolator& interpolator);

    /**
     * Interpolate sample from specified mipmap level.
     * Simple FPU version.
     * @param mipmap        mipmap index
     * @param phase         sampling point [0.0 .. 1.0)
     * @param pInterpolator interpolator configuration pointer
     */
    float Sample_FPU(int mipmap, float phase, const Interpolator& interpolator) const;

    /**
     * SSE version of @p Sample method.
     */
    __m128 Sample_SSE(int mipmap, __m128 phase, const Interpolator& interpolator) const;

    /**
     * Synthesize samples buffer.
     * @param samplesNum   Number of samples to generate.
     * @param freqBuff     Signal frequency for each sample.
     * @param ctx          Synthesiser context.
     * @param interpolator Wavetable interpolator.
     * @param[out] output  Buffer to write.
     */
    void Synth_FPU(WaveTableContext& ctx, const Interpolator& interpolator,
                   double& outLeft, double& outRight) const;

    /**
     * SSE version of @p Synth method.
     */
    void Synth_SSE(size_t samplesNum, const float* freqBuff, WaveTableContext& ctx,
                   const Interpolator& interpolator, float* output) const;
};

} // namespace mvSynth
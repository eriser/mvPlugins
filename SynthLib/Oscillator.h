#pragma once

#include "Module.h"
#include "Wavetable/Wavetable.hpp"

#define MW_MAX_SUBVOICES_NUM 16

namespace mvSynth {

// oscillator shapes
enum class WaveShape
{
    Sine = 0,
    Square,
    Saw,
};

// per-voice oscillator data
struct OscillatorVoiceData
{
    //set to 1, if envelope is near to 0
    bool faded;

    // base oscillator frequency
    SampleType baseFrequency;

    // detune factor for each unison voice
    SampleType subvoiceDetune[MW_MAX_SUBVOICES_NUM];

    // wavetable state
    WaveTableContext wtCtx;
};

/**
 * Class describing bandlimited oscillator unit.
 */
class Oscillator final : public Module
{
    WaveTable mWaveTable;
    OscillatorVoiceData mVoiceData[MW_MAX_VOICES];

public:
    static const int VolumePortId = 0;
    static const int PitchPortId = 1;
    static const int DetunePortId = 2;
    static const int StereoPortId = 3;

    SampleType noteOffset; // +12.0 - one octave up
    SampleType detune;     // unison detune
    SampleType stereo;     // 0 - mono, 1 - stereo
    uint32 subvoicesNum;   // number of unison subvoices
    bool retrig;           // reset subvoices phases during key press

    Oscillator();
    void LoadDefaultWaveshape(WaveShape waveShape, Synth* synth);

    bool ParseFromConfig(Synth* synth, const YAML::Node& node, std::string& errorStr);
    void OnProcess(int voiceID, Synth* synth);
    void OnInitVoice(int voiceID, Synth* synth, SampleType freq);
};

} // namespace mvSynth
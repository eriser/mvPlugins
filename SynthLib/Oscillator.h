#pragma once

#include "Module.h"

#define MW_OSCILLATORS_NUM 4
#define MW_MAX_SUBVOICES_NUM 16

namespace mvSynth {

// oscillator shapes
enum class WaveShape
{
    Sine = 0,
    Square,
    Saw,
};

// structure used by band-limited oscillators
struct OscillatorState
{
    SampleType phase;
    SampleType leak;
    SampleType freq;
    SampleType detuneOffset;
    SampleType d_freq;

    SampleType dcb_state;
    SampleType last;

    SampleType panning;
    bool firstSample;
};

// per-voice oscillator data
struct OscillatorVoiceData
{
    bool faded; //set to 1, if envelope is near to 0
    SampleType baseFrequency;
    OscillatorState states[MW_MAX_SUBVOICES_NUM];
};

/**
 * Class describing bandlimited oscillator unit.
 */
class Oscillator final : public Module
{
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
    WaveShape shape;
    bool retrig;           // reset subvoices phases during key press

    Oscillator();
    bool ParseFromConfig(const YAML::Node& node, std::string& errorStr);
    void OnProcess(int voiceID, Synth* synth);
    void OnInitVoice(int voiceID, Synth* synth, SampleType freq);
};

} // namespace mvSynth
#include "stdafx.h"
#include "Voice.h"

namespace mvSynth {

Voice::Voice()
{
    state = VoiceState::Disabled;
    time = 0.0f;
    key = 0;
    freq = 0.0f;
    velocity = 0.0f;
    panning = 0.0f;
}

void Voice::Press(int keyId, SampleType vel, SampleType pan)
{
    state = VoiceState::Pressed;
    time = 0.0f;

    key = keyId;
    freq = 110.0f * Pow(2.0f, (SampleType)key / 12.0f);
    velocity = vel;
    panning = pan;
}

void Voice::Release()
{
    state = VoiceState::Released;
    time = 0.0f;
}

} // namespace mvSynth
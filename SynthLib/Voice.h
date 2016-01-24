#pragma once

#include "Defines.h"
#include "Frame.h"
#include "MasterModule.h"

namespace mvSynth {

enum class VoiceState
{
    Disabled,  // not processed
    Pressed,   // pressed - requires processing
    Released,  // released - may require processing until the sound has not completely faded
};

/**
 * Class representing "voice" - a single played note.
 */
struct Voice
{
    Synth* synth;
    VoiceState state;

    // how long is voice active / released
    SampleType time;

    // keyboard key ID
    int key;

    // from MIDI
    SampleType freq;
    SampleType velocity;
    SampleType panning;

    Voice();

    /**
     * Press a key - start new note. Changes the voice state into "Pressed".
     * @param keyId  Keyboard key number
     * @param vel    Velocity
     * @param pan    Panning
     */
    void Press(int keyId, SampleType vel, SampleType pan);

    /**
     * Release a key. Changes the voice state into "Released".
     */
    void Release();
};

} // namespace mvSynth
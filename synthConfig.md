# Modules

## Oscillator (osc)

Parameters:

* shape     - Waveform shape. Currently supported are: "saw", "square", "sine".
* pitch     - Pitch offset. +12.0 means one octave up.
* stereo    - Stereo separation. 0.0 - mono, 1.0 - full stereo separation.
* subvoices - Number of unison subvoices.
* detune    - Detune factor for unison subvoices.
              0.0 - no detune, 1.0 - detune accros whole frequency spectrum, so use small values.
* retrig    - Reset unison subvoices phases when a key is pressed.

## Envelope Generator (env)

Parameters:

* attack  - Attack time in seconds.
* decay   - Attack time factor (the lower, the faster).
* sustain - Sustan level after decay.
* release - Release time factor (the lower, the faster).

## Filter (filter)

Parameters:

* type - Filter type. Currently supported are: "lowpass", "highpass", "bandpass", "notch".
* cutoff - Cutoff frequency. 0.0 - 0 Hz, 1.0 - 22050 Hz, regardless of synth sampling rate.
* resonance - Resonance factor. The higher, the more frequency boost near cutoff frequency.
* order - How many times the filter will be applied. Value of 1 implies 12dB/oct slope, 2 - 24dB/oct, etc.

## Low Frequency Oscillator (lfo)

Parameters:

* shape - Waveform shape. Currently supported are: "saw", "square", "sine".
* freq  - oscillator frequency in Hertz.

## Noise Generator (noise)

* distr - Generated sample magnitudes distribution. Currently supported are:
	* "uniform" - uniform distribution from range [-1.0, 1.0],
	* "binary" - binary distribution - generates only -1.0 and 1.0 values,
	* "normal" - normal (Gaussian) distribution.
* hold - how many samples should be a single generated value repeated.
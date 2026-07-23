#pragma once
#include <JuceHeader.h>
#include "Filter.h"

//==============================================================================
/** Per-slot DSP state for one unit in an effects chain — a generator's own FX
    chain and each shared FX bus both use this same struct, one instance per
    slot, since a slot can be any one of the FxType cases below. Which case is
    actually applied lives on the owner (GeneratorState::fxTypes / the shared
    FX bus's fxBusTypes) and is read live each block; see
    ViolentAudioProcessor::applyGeneratorFx / processFxBuses. */
struct FxSlotDSP
{
    using StereoIIRFilter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    juce::dsp::Compressor<float>  compressor;
    juce::dsp::Gain<float>        makeup;
    juce::dsp::NoiseGate<float>   gate;
    juce::dsp::Reverb             reverb;
    StereoIIRFilter                distToneFilter;   // Distortion's tone-shaping lowpass
    TPTFilterProcessor             filter;            // used when this slot's type is Filter
    bool prepared = false;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        compressor.prepare (spec);
        makeup.prepare (spec);
        gate.prepare (spec);
        reverb.prepare (spec);
        distToneFilter.prepare (spec);
        *distToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
            spec.sampleRate, 4000.0);
        filter.prepare (spec);
        prepared = true;
    }
};

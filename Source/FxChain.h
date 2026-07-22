#pragma once
#include <JuceHeader.h>

static constexpr int MAX_FX       = 8;
static constexpr int MAX_FX_BUSES = 8;
static constexpr int NUM_FX_TYPES = 8;

//==============================================================================
enum class FxType { None = 0, Distortion, Compressor, Gate, Reverb, Chorus, Delay, Filter };

inline const char* fxTypeName (FxType t)
{
    switch (t) {
        case FxType::Distortion: return "Distortion";
        case FxType::Compressor: return "Compressor";
        case FxType::Gate:       return "Gate";
        case FxType::Reverb:     return "Reverb";
        case FxType::Chorus:     return "Chorus";
        case FxType::Delay:      return "Delay";
        case FxType::Filter:     return "Filter";
        case FxType::None:
        default:                 return "None";
    }
}

//==============================================================================
// Per-slot DSP state for a single FX unit within a generator's FX chain.
struct FxSlotDSP
{
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                    juce::dsp::IIR::Coefficients<float>> distToneFilter;
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Gain<float>       makeup;
    juce::dsp::NoiseGate<float>  gate;
    juce::dsp::Reverb            reverb;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        distToneFilter.prepare (spec);
        compressor.prepare (spec);
        makeup.prepare (spec);
        gate.prepare (spec);
        reverb.prepare (spec);
    }
};

//==============================================================================
namespace ParamIDs
{
    inline juce::String busDrive    (int b) { return "bus" + juce::String (b) + "_drive"; }
    inline juce::String busTone     (int b) { return "bus" + juce::String (b) + "_tone"; }
    inline juce::String busLevel    (int b) { return "bus" + juce::String (b) + "_level"; }
    inline juce::String busDistType (int b) { return "bus" + juce::String (b) + "_dtype"; }
    inline juce::String busThresh   (int b) { return "bus" + juce::String (b) + "_thresh"; }
    inline juce::String busRatio    (int b) { return "bus" + juce::String (b) + "_ratio"; }
    inline juce::String busAttack   (int b) { return "bus" + juce::String (b) + "_attack"; }
    inline juce::String busRelease  (int b) { return "bus" + juce::String (b) + "_release"; }
    inline juce::String busMakeup   (int b) { return "bus" + juce::String (b) + "_makeup"; }
    inline juce::String busRoom     (int b) { return "bus" + juce::String (b) + "_room"; }
    inline juce::String busDamping  (int b) { return "bus" + juce::String (b) + "_damp"; }
    inline juce::String busWet      (int b) { return "bus" + juce::String (b) + "_wet"; }
    inline juce::String busWidth    (int b) { return "bus" + juce::String (b) + "_width"; }
    inline juce::String busFilterType (int b) { return "bus" + juce::String (b) + "_ftype"; }
    inline juce::String busFilterCut  (int b) { return "bus" + juce::String (b) + "_fcut"; }
    inline juce::String busFilterRes  (int b) { return "bus" + juce::String (b) + "_fres"; }
}

#pragma once
#include <JuceHeader.h>

//==============================================================================
static constexpr int MAX_GENERATORS  = 8;
static constexpr int MAX_GEN_FILTERS = 4;

// Source types — waveforms + sample playback unified
enum class SourceType
{
    Sine = 0, Saw, Square, Triangle, Noise,
    Sample   // uses a loaded audio buffer as the waveform
};
static constexpr int NUM_SOURCE_TYPES = 6;

inline const char* sourceTypeName (SourceType t)
{
    switch (t) {
        case SourceType::Sine:     return "Sine";
        case SourceType::Saw:      return "Saw";
        case SourceType::Square:   return "Square";
        case SourceType::Triangle: return "Triangle";
        case SourceType::Noise:    return "Noise";
        case SourceType::Sample:   return "Sample";
        default:                   return "Sine";
    }
}

//==============================================================================
namespace ParamIDs
{
    // Generator-level
    inline juce::String genEn    (int g) { return "gen_" + juce::String (g) + "_en"; }
    inline juce::String genLevel (int g) { return "gen_" + juce::String (g) + "_level"; }
    inline juce::String genPan   (int g) { return "gen_" + juce::String (g) + "_pan"; }

    // Source (unified oscillator/sampler) inside generator g
    inline juce::String genSrcType      (int g) { return "gen_" + juce::String (g) + "_src_type"; }
    inline juce::String genSrcGain      (int g) { return "gen_" + juce::String (g) + "_src_gain"; }
    inline juce::String genSrcOct       (int g) { return "gen_" + juce::String (g) + "_src_oct"; }
    inline juce::String genSrcSemi      (int g) { return "gen_" + juce::String (g) + "_src_semi"; }
    inline juce::String genSrcDet       (int g) { return "gen_" + juce::String (g) + "_src_det"; }
    inline juce::String genSrcPhase     (int g) { return "gen_" + juce::String (g) + "_src_phase"; }
    inline juce::String genSrcPW        (int g) { return "gen_" + juce::String (g) + "_src_pw"; }
    inline juce::String genSrcPan       (int g) { return "gen_" + juce::String (g) + "_src_pan"; }
    inline juce::String genSrcVel       (int g) { return "gen_" + juce::String (g) + "_src_vel"; }
    inline juce::String genSrcUni       (int g) { return "gen_" + juce::String (g) + "_src_uni"; }
    inline juce::String genSrcUniSpread (int g) { return "gen_" + juce::String (g) + "_src_unispread"; }
    inline juce::String genSrcLoop      (int g) { return "gen_" + juce::String (g) + "_src_loop"; }
    // ADSR
    inline juce::String genSrcAtt (int g) { return "gen_" + juce::String (g) + "_src_att"; }
    inline juce::String genSrcDec (int g) { return "gen_" + juce::String (g) + "_src_dec"; }
    inline juce::String genSrcSus (int g) { return "gen_" + juce::String (g) + "_src_sus"; }
    inline juce::String genSrcRel (int g) { return "gen_" + juce::String (g) + "_src_rel"; }

    // Filters inside generator g, slot f
    inline juce::String genFltEn   (int g, int f) { return "gen_" + juce::String (g) + "_flt" + juce::String (f) + "_en"; }
    inline juce::String genFltType (int g, int f) { return "gen_" + juce::String (g) + "_flt" + juce::String (f) + "_type"; }
    inline juce::String genFltCut  (int g, int f) { return "gen_" + juce::String (g) + "_flt" + juce::String (f) + "_cut"; }
    inline juce::String genFltRes  (int g, int f) { return "gen_" + juce::String (g) + "_flt" + juce::String (f) + "_res"; }

    // Send from generator g into shared FX bus b, 0..1
    inline juce::String genSend (int g, int b) { return "gen_" + juce::String (g) + "_send" + juce::String (b); }
}

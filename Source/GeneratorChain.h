#pragma once
#include <JuceHeader.h>
#include "FxChain.h"

//==============================================================================
static constexpr int MAX_STREAMS        = 8;
static constexpr int MAX_STREAM_FILTERS = 4;
static constexpr int MAX_STREAM_FX      = 8;

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
    // Stream-level
    inline juce::String streamEn    (int s) { return "st_" + juce::String(s) + "_en"; }
    inline juce::String streamLevel (int s) { return "st_" + juce::String(s) + "_level"; }
    inline juce::String streamPan   (int s) { return "st_" + juce::String(s) + "_pan"; }

    // Source (unified oscillator/sampler) inside stream s
    inline juce::String stSrcType      (int s) { return "st_" + juce::String(s) + "_src_type"; }
    inline juce::String stSrcGain      (int s) { return "st_" + juce::String(s) + "_src_gain"; }
    inline juce::String stSrcOct       (int s) { return "st_" + juce::String(s) + "_src_oct"; }
    inline juce::String stSrcSemi      (int s) { return "st_" + juce::String(s) + "_src_semi"; }
    inline juce::String stSrcDet       (int s) { return "st_" + juce::String(s) + "_src_det"; }
    inline juce::String stSrcPhase     (int s) { return "st_" + juce::String(s) + "_src_phase"; }
    inline juce::String stSrcPW        (int s) { return "st_" + juce::String(s) + "_src_pw"; }
    inline juce::String stSrcPan       (int s) { return "st_" + juce::String(s) + "_src_pan"; }
    inline juce::String stSrcVel       (int s) { return "st_" + juce::String(s) + "_src_vel"; }
    inline juce::String stSrcUni       (int s) { return "st_" + juce::String(s) + "_src_uni"; }
    inline juce::String stSrcUniSpread (int s) { return "st_" + juce::String(s) + "_src_unispread"; }
    inline juce::String stSrcLoop      (int s) { return "st_" + juce::String(s) + "_src_loop"; }
    // ADSR
    inline juce::String stSrcAtt (int s) { return "st_" + juce::String(s) + "_src_att"; }
    inline juce::String stSrcDec (int s) { return "st_" + juce::String(s) + "_src_dec"; }
    inline juce::String stSrcSus (int s) { return "st_" + juce::String(s) + "_src_sus"; }
    inline juce::String stSrcRel (int s) { return "st_" + juce::String(s) + "_src_rel"; }

    // Filters inside stream s, slot f
    inline juce::String stFltEn   (int s, int f) { return "st_" + juce::String(s) + "_flt" + juce::String(f) + "_en"; }
    inline juce::String stFltType (int s, int f) { return "st_" + juce::String(s) + "_flt" + juce::String(f) + "_type"; }
    inline juce::String stFltCut  (int s, int f) { return "st_" + juce::String(s) + "_flt" + juce::String(f) + "_cut"; }
    inline juce::String stFltRes  (int s, int f) { return "st_" + juce::String(s) + "_flt" + juce::String(f) + "_res"; }

    // FX inside stream s, slot x
    inline juce::String stFxDrive    (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_drive"; }
    inline juce::String stFxTone     (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_tone"; }
    inline juce::String stFxLevel    (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_level"; }
    inline juce::String stFxDistType (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_dtype"; }
    inline juce::String stFxThresh   (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_thresh"; }
    inline juce::String stFxRatio    (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_ratio"; }
    inline juce::String stFxAttack   (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_attack"; }
    inline juce::String stFxRelease  (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_release"; }
    inline juce::String stFxMakeup   (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_makeup"; }
    inline juce::String stFxRoom     (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_room"; }
    inline juce::String stFxDamping  (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_damp"; }
    inline juce::String stFxWet      (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_wet"; }
    inline juce::String stFxWidth    (int s, int x) { return "st_" + juce::String(s) + "_fx" + juce::String(x) + "_width"; }
}

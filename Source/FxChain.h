#pragma once
#include <JuceHeader.h>

static constexpr int MAX_FX       = 8;
static constexpr int NUM_FX_TYPES = 7;

//==============================================================================
enum class FxType { None = 0, Distortion, Compressor, Gate, Reverb, Chorus, Delay };

inline const char* fxTypeName (FxType t)
{
    switch (t) {
        case FxType::Distortion: return "Distortion";
        case FxType::Compressor: return "Compressor";
        case FxType::Gate:       return "Gate";
        case FxType::Reverb:     return "Reverb";
        case FxType::Chorus:     return "Chorus";
        case FxType::Delay:      return "Delay";
        case FxType::None:
        default:                 return "None";
    }
}

//==============================================================================
namespace ParamIDs
{
    inline juce::String fxDrive    (int n) { return "fx_" + juce::String (n) + "_drive"; }
    inline juce::String fxTone     (int n) { return "fx_" + juce::String (n) + "_tone"; }
    inline juce::String fxLevel    (int n) { return "fx_" + juce::String (n) + "_level"; }
    inline juce::String fxDistType (int n) { return "fx_" + juce::String (n) + "_dtype"; }
    inline juce::String fxThresh   (int n) { return "fx_" + juce::String (n) + "_thresh"; }
    inline juce::String fxRatio    (int n) { return "fx_" + juce::String (n) + "_ratio"; }
    inline juce::String fxAttack   (int n) { return "fx_" + juce::String (n) + "_attack"; }
    inline juce::String fxRelease  (int n) { return "fx_" + juce::String (n) + "_release"; }
    inline juce::String fxMakeup   (int n) { return "fx_" + juce::String (n) + "_makeup"; }
    inline juce::String fxRoom     (int n) { return "fx_" + juce::String (n) + "_room"; }
    inline juce::String fxDamping  (int n) { return "fx_" + juce::String (n) + "_damp"; }
    inline juce::String fxWet      (int n) { return "fx_" + juce::String (n) + "_wet"; }
    inline juce::String fxWidth    (int n) { return "fx_" + juce::String (n) + "_width"; }
}

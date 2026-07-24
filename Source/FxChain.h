#pragma once
#include <JuceHeader.h>

// A shared pool of Effect Components — no longer owned by individual
// generators or a separate "bus"/"master filter" concept. Each one is
// routed to whichever generators it applies to (see
// ViolentAudioProcessor::EffectComponent::routing).
static constexpr int MAX_EFFECTS  = 16;
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
namespace ParamIDs
{
    inline juce::String effectDrive    (int x) { return "fx" + juce::String (x) + "_drive"; }
    inline juce::String effectTone     (int x) { return "fx" + juce::String (x) + "_tone"; }
    inline juce::String effectLevel    (int x) { return "fx" + juce::String (x) + "_level"; }
    inline juce::String effectDistType (int x) { return "fx" + juce::String (x) + "_dtype"; }
    inline juce::String effectThresh   (int x) { return "fx" + juce::String (x) + "_thresh"; }
    inline juce::String effectRatio    (int x) { return "fx" + juce::String (x) + "_ratio"; }
    inline juce::String effectAttack   (int x) { return "fx" + juce::String (x) + "_attack"; }
    inline juce::String effectRelease  (int x) { return "fx" + juce::String (x) + "_release"; }
    inline juce::String effectMakeup   (int x) { return "fx" + juce::String (x) + "_makeup"; }
    inline juce::String effectRoom     (int x) { return "fx" + juce::String (x) + "_room"; }
    inline juce::String effectDamping  (int x) { return "fx" + juce::String (x) + "_damp"; }
    inline juce::String effectWet      (int x) { return "fx" + juce::String (x) + "_wet"; }
    inline juce::String effectWidth    (int x) { return "fx" + juce::String (x) + "_width"; }
    inline juce::String effectFilterType (int x) { return "fx" + juce::String (x) + "_ftype"; }
    inline juce::String effectFilterCut  (int x) { return "fx" + juce::String (x) + "_fcut"; }
    inline juce::String effectFilterRes  (int x) { return "fx" + juce::String (x) + "_fres"; }
}

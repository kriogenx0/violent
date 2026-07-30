#pragma once
#include <JuceHeader.h>

//==============================================================================
// A shared pool of Modulator Components — each one is an Envelope or LFO
// source that can target any one modulatable parameter (see
// ViolentAudioProcessor::getModulatableParameters()), nudging that
// parameter's value up or down by up to its Amount around whatever it's
// currently set to.
static constexpr int MAX_MODULATORS = 8;

enum class ModulatorSourceType { LFO = 0, Envelope };
static constexpr int NUM_MODULATOR_SOURCE_TYPES = 2;

inline const char* modulatorSourceTypeName (ModulatorSourceType t)
{
    switch (t) {
        case ModulatorSourceType::LFO:      return "LFO";
        case ModulatorSourceType::Envelope: return "Envelope";
        default:                            return "LFO";
    }
}

enum class LfoShape { Sine = 0, Triangle, Square, Saw };
static constexpr int NUM_LFO_SHAPES = 4;

inline const char* lfoShapeName (LfoShape s)
{
    switch (s) {
        case LfoShape::Sine:     return "Sine";
        case LfoShape::Triangle: return "Triangle";
        case LfoShape::Square:   return "Square";
        case LfoShape::Saw:      return "Saw";
        default:                 return "Sine";
    }
}

namespace ParamIDs
{
    inline juce::String modAmount   (int m) { return "mod" + juce::String (m) + "_amount"; }
    inline juce::String modLfoRate  (int m) { return "mod" + juce::String (m) + "_lforate"; }
    inline juce::String modLfoShape (int m) { return "mod" + juce::String (m) + "_lfoshape"; }
    inline juce::String modEnvAtt   (int m) { return "mod" + juce::String (m) + "_envatt"; }
    inline juce::String modEnvDec   (int m) { return "mod" + juce::String (m) + "_envdec"; }
    inline juce::String modEnvSus   (int m) { return "mod" + juce::String (m) + "_envsus"; }
    inline juce::String modEnvRel   (int m) { return "mod" + juce::String (m) + "_envrel"; }
}

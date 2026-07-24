#pragma once
#include <JuceHeader.h>
#include "FxChain.h"

//==============================================================================
static constexpr int MAX_GENERATORS      = 8;
// A shared pool of MIDI Modifier Components — no longer owned by individual
// generators. Each one is routed to whichever generators it applies to (see
// ViolentAudioProcessor::MidiModifierComponent::routing).
static constexpr int MAX_MIDI_MODIFIERS  = 8;

// MIDI modifier types
enum class MidiModType { PitchShift = 0, KeyShift, Arp };
static constexpr int NUM_MIDI_MOD_TYPES = 3;

inline const char* midiModTypeName (MidiModType t)
{
    switch (t) {
        case MidiModType::PitchShift: return "Pitch Shift";
        case MidiModType::KeyShift:   return "Key Shift";
        case MidiModType::Arp:        return "Arpeggiator";
        default:                      return "Pitch Shift";
    }
}

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
    inline juce::String generatorEn    (int s) { return "gen_" + juce::String(s) + "_en"; }
    inline juce::String generatorLevel (int s) { return "gen_" + juce::String(s) + "_level"; }
    inline juce::String generatorPan   (int s) { return "gen_" + juce::String(s) + "_pan"; }

    // MIDI Modifier Components, slot m — which sub-params are actually used
    // depends on that slot's chosen MidiModType
    inline juce::String midiModTranspose (int m) { return "mm" + juce::String(m) + "_transpose"; }
    inline juce::String midiModOctave    (int m) { return "mm" + juce::String(m) + "_octave"; }
    inline juce::String midiModKeyRoot   (int m) { return "mm" + juce::String(m) + "_key_root"; }
    inline juce::String midiModKeyScale  (int m) { return "mm" + juce::String(m) + "_key_scale"; }
    inline juce::String midiModArpRate   (int m) { return "mm" + juce::String(m) + "_arp_rate"; }

    // Source (unified oscillator/sampler) inside generator s
    inline juce::String genSrcType      (int s) { return "gen_" + juce::String(s) + "_src_type"; }
    inline juce::String genSrcGain      (int s) { return "gen_" + juce::String(s) + "_src_gain"; }
    inline juce::String genSrcOct       (int s) { return "gen_" + juce::String(s) + "_src_oct"; }
    inline juce::String genSrcSemi      (int s) { return "gen_" + juce::String(s) + "_src_semi"; }
    inline juce::String genSrcDet       (int s) { return "gen_" + juce::String(s) + "_src_det"; }
    inline juce::String genSrcPhase     (int s) { return "gen_" + juce::String(s) + "_src_phase"; }
    inline juce::String genSrcPW        (int s) { return "gen_" + juce::String(s) + "_src_pw"; }
    inline juce::String genSrcPan       (int s) { return "gen_" + juce::String(s) + "_src_pan"; }
    inline juce::String genSrcVel       (int s) { return "gen_" + juce::String(s) + "_src_vel"; }
    inline juce::String genSrcUni       (int s) { return "gen_" + juce::String(s) + "_src_uni"; }
    inline juce::String genSrcUniSpread (int s) { return "gen_" + juce::String(s) + "_src_unispread"; }
    inline juce::String genSrcLoop      (int s) { return "gen_" + juce::String(s) + "_src_loop"; }
    // ADSR
    inline juce::String genSrcAtt (int s) { return "gen_" + juce::String(s) + "_src_att"; }
    inline juce::String genSrcDec (int s) { return "gen_" + juce::String(s) + "_src_dec"; }
    inline juce::String genSrcSus (int s) { return "gen_" + juce::String(s) + "_src_sus"; }
    inline juce::String genSrcRel (int s) { return "gen_" + juce::String(s) + "_src_rel"; }
}

#pragma once
#include <JuceHeader.h>
#include "FxChain.h"

//==============================================================================
static constexpr int MAX_GENERATORS          = 8;
static constexpr int MAX_GENERATOR_FX        = 8;
static constexpr int MAX_GENERATOR_MIDI_MODS = 4;

// MIDI modifier types — a generator can chain several of these, in any order
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

    // MIDI modifiers — a chain of stages before the generator, slot m; which
    // sub-params are actually used depends on that slot's chosen MidiModType
    inline juce::String genMidiTranspose (int s, int m) { return "gen_" + juce::String(s) + "_midi" + juce::String(m) + "_transpose"; }
    inline juce::String genMidiOctave    (int s, int m) { return "gen_" + juce::String(s) + "_midi" + juce::String(m) + "_octave"; }
    inline juce::String genMidiKeyRoot   (int s, int m) { return "gen_" + juce::String(s) + "_midi" + juce::String(m) + "_key_root"; }
    inline juce::String genMidiKeyScale  (int s, int m) { return "gen_" + juce::String(s) + "_midi" + juce::String(m) + "_key_scale"; }
    inline juce::String genMidiArpRate   (int s, int m) { return "gen_" + juce::String(s) + "_midi" + juce::String(m) + "_arp_rate"; }

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

    // FX inside generator s, slot x
    inline juce::String genFxDrive    (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_drive"; }
    inline juce::String genFxTone     (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_tone"; }
    inline juce::String genFxLevel    (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_level"; }
    inline juce::String genFxDistType (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_dtype"; }
    inline juce::String genFxThresh   (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_thresh"; }
    inline juce::String genFxRatio    (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_ratio"; }
    inline juce::String genFxAttack   (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_attack"; }
    inline juce::String genFxRelease  (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_release"; }
    inline juce::String genFxMakeup   (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_makeup"; }
    inline juce::String genFxRoom     (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_room"; }
    inline juce::String genFxDamping  (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_damp"; }
    inline juce::String genFxWet      (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_wet"; }
    inline juce::String genFxWidth    (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_width"; }
    inline juce::String genFxFilterType (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_ftype"; }
    inline juce::String genFxFilterCut  (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_fcut"; }
    inline juce::String genFxFilterRes  (int s, int x) { return "gen_" + juce::String(s) + "_fx" + juce::String(x) + "_fres"; }

    // Send from generator s into shared FX bus b, 0..1
    inline juce::String genSend (int s, int b) { return "gen_" + juce::String(s) + "_send" + juce::String(b); }
}

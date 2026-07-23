#pragma once
#include <JuceHeader.h>
#include "../GeneratorChain.h"

//==============================================================================
namespace MidiModifier
{
    /** Applies a generator's chain of modifiers to a note, in slot order.
        Pitch Shift and Key Shift are deterministic per-note transforms
        applied here; Arp slots are no-ops in this function since they change
        note timing/gating rather than pitch (see hasArp()/firstArpSlot()
        and ViolentAudioProcessor::renderMidiModifiers for the arp sequencer
        itself). Result is clamped to the valid MIDI note range. */
    int apply (const juce::AudioProcessorValueTreeState& apvts, int generatorIdx, int note,
               int numMods, const std::array<MidiModType, MAX_GENERATOR_MIDI_MODS>& types) noexcept;

    /** True if any slot in the chain is an Arp modifier. */
    bool hasArp (int numMods, const std::array<MidiModType, MAX_GENERATOR_MIDI_MODS>& types) noexcept;

    /** Index of the first Arp slot in the chain, or -1 if none. If a generator
        stacks several Arp modifiers, the first one's rate is what's used. */
    int firstArpSlot (int numMods, const std::array<MidiModType, MAX_GENERATOR_MIDI_MODS>& types) noexcept;
}

//==============================================================================
/** Per-generator arpeggiator sequencer state — held notes (already passed
    through apply()) and the sample-accurate step timer that turns them into
    a stream of note-on/off events. Only meaningful while hasArp() is true for
    that generator; ViolentAudioProcessor::renderMidiModifiers drives it and
    owns actually starting/stopping voices, since that requires the voice pool. */
struct MidiModifierState
{
    std::vector<int> heldNotes;
    int  arpStepIndex       = -1;
    int  arpSamplePos       = 0;
    int  arpNextEventSample = 0;
    bool arpNoteIsOn        = false;
    int  arpCurrentNote     = -1;
};

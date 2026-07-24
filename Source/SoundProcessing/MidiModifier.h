#pragma once
#include <JuceHeader.h>
#include "../GeneratorChain.h"

//==============================================================================
namespace MidiModifier
{
    /** Applies one modifier's transform to a note. Pitch Shift and Key Shift
        are deterministic per-note transforms applied here; Arp is a no-op
        since it changes note timing/gating rather than pitch (see
        ViolentAudioProcessor::isArpEnabled/renderMidiModifiers for the arp
        sequencer itself). Result is clamped to the valid MIDI note range. */
    int applyOne (const juce::AudioProcessorValueTreeState& apvts, int slot, MidiModType type, int note) noexcept;
}

//==============================================================================
/** Per (modifier, generator) arpeggiator sequencer state — held notes
    (already passed through MidiModifier::applyOne for every modifier ahead
    of this one in the chain for that generator) and the sample-accurate step
    timer that turns them into a stream of note-on/off events. Only
    meaningful while that modifier is an Arp type routed to that generator;
    ViolentAudioProcessor::renderMidiModifiers drives it and owns actually
    starting/stopping voices, since that requires the voice pool. */
struct MidiModifierState
{
    std::vector<int> heldNotes;
    int  arpStepIndex       = -1;
    int  arpSamplePos       = 0;
    int  arpNextEventSample = 0;
    bool arpNoteIsOn        = false;
    int  arpCurrentNote     = -1;
};

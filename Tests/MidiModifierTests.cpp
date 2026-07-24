#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace
{

void setInt (ViolentAudioProcessor& proc, const juce::String& id, int value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterInt*> (proc.apvts.getParameter (id)))
        *p = value;
}

void setChoice (ViolentAudioProcessor& proc, const juce::String& id, int index)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
        *p = index;
}

// Appends a new shared MIDI Modifier Component routed to the given generator,
// returning its slot index — mirrors how the UI's "+ Add MIDI Modifier"
// popup appends one, then the row's routing checkboxes route it.
int addModifier (ViolentAudioProcessor& proc, MidiModType type, int generatorIdx = 0)
{
    const int slot = proc.numMidiModifiers++;
    auto& mm = proc.midiModifiers[(size_t) slot];
    mm.type = type;
    mm.enabled = true;
    mm.routing[(size_t) generatorIdx] = true;
    return slot;
}

void clearModifiers (ViolentAudioProcessor& proc)
{
    for (auto& mm : proc.midiModifiers) mm = {};
    proc.numMidiModifiers = 0;
}

class MidiModifierTests : public juce::UnitTest
{
public:
    MidiModifierTests() : juce::UnitTest ("MIDI modifier", "Processor") {}

    void runTest() override
    {
        ViolentAudioProcessor proc;

        beginTest ("With no modifiers, notes pass through unchanged");
        {
            expect (! proc.isArpEnabled (0));
            expectEquals (proc.applyMidiModifier (0, 60), 60);
        }

        beginTest ("Adding a Pitch Shift modifier routed to generator 0 lets transpose/octave take effect");
        {
            // Slots are reused across tests (each test clears the chain but
            // params aren't reset with it), so every test that touches
            // transpose/octave sets both explicitly rather than assuming a
            // fresh default.
            const int slot = addModifier (proc, MidiModType::PitchShift, 0);
            setInt (proc, ParamIDs::midiModTranspose (slot), 5);
            setInt (proc, ParamIDs::midiModOctave (slot), 0);
            expectEquals (proc.applyMidiModifier (0, 60), 65);
            setInt (proc, ParamIDs::midiModOctave (slot), 1);
            expectEquals (proc.applyMidiModifier (0, 60), 77); // +5 semitones, +1 octave
            clearModifiers (proc);
        }

        beginTest ("Removing the modifier again bypasses it even with sub-settings still set");
        {
            // midiModTranspose(0) still holds 5 from the previous test, but
            // with no modifiers it should never be read.
            expectEquals (proc.applyMidiModifier (0, 60), 60);
        }

        beginTest ("A modifier only affects generators it's routed to");
        {
            const int slot = addModifier (proc, MidiModType::PitchShift, 1); // routed to generator 1 only
            setInt (proc, ParamIDs::midiModTranspose (slot), 12);
            setInt (proc, ParamIDs::midiModOctave (slot), 0);
            expectEquals (proc.applyMidiModifier (1, 60), 72);
            expectEquals (proc.applyMidiModifier (0, 60), 60); // untouched — not routed here
            clearModifiers (proc);
        }

        beginTest ("Octave shifts by 12 semitones per octave, either direction");
        {
            const int slot = addModifier (proc, MidiModType::PitchShift, 0);
            setInt (proc, ParamIDs::midiModTranspose (slot), 0);
            setInt (proc, ParamIDs::midiModOctave (slot), 1);
            expectEquals (proc.applyMidiModifier (0, 60), 72);
            setInt (proc, ParamIDs::midiModOctave (slot), -2);
            expectEquals (proc.applyMidiModifier (0, 60), 36);
            clearModifiers (proc);
        }

        beginTest ("Result is clamped to the valid MIDI note range");
        {
            const int slot = addModifier (proc, MidiModType::PitchShift, 0);
            setInt (proc, ParamIDs::midiModOctave (slot), 0);
            setInt (proc, ParamIDs::midiModTranspose (slot), 24);
            expectEquals (proc.applyMidiModifier (0, 120), 127);
            setInt (proc, ParamIDs::midiModTranspose (slot), -24);
            expectEquals (proc.applyMidiModifier (0, 5), 0);
            clearModifiers (proc);
        }

        beginTest ("Key Shift quantizes out-of-scale notes to the nearest scale tone");
        {
            const int slot = addModifier (proc, MidiModType::KeyShift, 0);
            setChoice (proc, ParamIDs::midiModKeyRoot (slot), 0);  // C
            setChoice (proc, ParamIDs::midiModKeyScale (slot), 0); // Major

            // C major = {C D E F G A B}. C#4 (61) sits a semitone above C4 and
            // a whole tone below D4, so the nearer scale tone is C4 (60).
            expectEquals (proc.applyMidiModifier (0, 61), 60);

            // Notes already in the scale are left alone.
            expectEquals (proc.applyMidiModifier (0, 62), 62); // D4

            clearModifiers (proc);
        }

        beginTest ("Modifiers chain in order: Pitch Shift then Key Shift");
        {
            const int pitchSlot = addModifier (proc, MidiModType::PitchShift, 0);
            const int keySlot   = addModifier (proc, MidiModType::KeyShift, 0);
            setInt    (proc, ParamIDs::midiModOctave    (pitchSlot), 0);
            setInt    (proc, ParamIDs::midiModTranspose (pitchSlot), 1);  // C4 -> C#4
            setChoice (proc, ParamIDs::midiModKeyRoot   (keySlot),  0);   // C
            setChoice (proc, ParamIDs::midiModKeyScale  (keySlot),  0);   // Major

            // Transposed to C#4 (61) first, then quantized back to C4 (60).
            expectEquals (proc.applyMidiModifier (0, 60), 60);
            clearModifiers (proc);
        }

        beginTest ("A disabled modifier is bypassed even though it's routed and would otherwise apply");
        {
            const int slot = addModifier (proc, MidiModType::PitchShift, 0);
            setInt (proc, ParamIDs::midiModTranspose (slot), 12);
            setInt (proc, ParamIDs::midiModOctave (slot), 0);
            expectEquals (proc.applyMidiModifier (0, 60), 72);

            proc.midiModifiers[(size_t) slot].enabled = false;
            expectEquals (proc.applyMidiModifier (0, 60), 60);
            clearModifiers (proc);
        }

        beginTest ("isArpEnabled reflects whether an enabled Arp modifier is routed to that generator");
        {
            expect (! proc.isArpEnabled (0));
            addModifier (proc, MidiModType::PitchShift, 0);
            expect (! proc.isArpEnabled (0));
            const int arpSlot = addModifier (proc, MidiModType::Arp, 0);
            expect (proc.isArpEnabled (0));
            expect (! proc.isArpEnabled (1)); // not routed to generator 1

            proc.midiModifiers[(size_t) arpSlot].enabled = false;
            expect (! proc.isArpEnabled (0));

            clearModifiers (proc);
            expect (! proc.isArpEnabled (0));
        }
    }
};

static MidiModifierTests midiModifierTests;

} // namespace

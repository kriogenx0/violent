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

// Clears any modifiers from a previous test and appends one of the given
// type, returning its slot index — mirrors how the UI's "+ MIDI Modifier"
// popup appends a new chain entry.
int addModifier (ViolentAudioProcessor& proc, MidiModType type)
{
    auto& gen = proc.generators[0];
    const int slot = gen.numMidiMods++;
    gen.midiModTypes[(size_t) slot] = type;
    return slot;
}

void clearModifiers (ViolentAudioProcessor& proc)
{
    proc.generators[0].numMidiMods = 0;
}

class MidiModifierTests : public juce::UnitTest
{
public:
    MidiModifierTests() : juce::UnitTest ("MIDI modifier", "Processor") {}

    void runTest() override
    {
        ViolentAudioProcessor proc;

        beginTest ("With no modifiers in the chain, notes pass through unchanged");
        {
            expect (! proc.isArpEnabled (0));
            expectEquals (proc.applyMidiModifier (0, 60), 60);
        }

        beginTest ("Adding a Pitch Shift modifier lets transpose/octave take effect");
        {
            // Slots are reused across tests (each test clears the chain but
            // params aren't reset with it), so every test that touches
            // transpose/octave sets both explicitly rather than assuming a
            // fresh default.
            const int slot = addModifier (proc, MidiModType::PitchShift);
            setInt (proc, ParamIDs::genMidiTranspose (0, slot), 5);
            setInt (proc, ParamIDs::genMidiOctave (0, slot), 0);
            expectEquals (proc.applyMidiModifier (0, 60), 65);
            setInt (proc, ParamIDs::genMidiOctave (0, slot), 1);
            expectEquals (proc.applyMidiModifier (0, 60), 77); // +5 semitones, +1 octave
            clearModifiers (proc);
        }

        beginTest ("Removing the modifier again bypasses it even with sub-settings still set");
        {
            // genMidiTranspose(0, 0) still holds 5 from the previous test, but
            // with an empty chain it should never be read.
            expectEquals (proc.applyMidiModifier (0, 60), 60);
        }

        beginTest ("Octave shifts by 12 semitones per octave, either direction");
        {
            const int slot = addModifier (proc, MidiModType::PitchShift);
            setInt (proc, ParamIDs::genMidiTranspose (0, slot), 0);
            setInt (proc, ParamIDs::genMidiOctave (0, slot), 1);
            expectEquals (proc.applyMidiModifier (0, 60), 72);
            setInt (proc, ParamIDs::genMidiOctave (0, slot), -2);
            expectEquals (proc.applyMidiModifier (0, 60), 36);
            clearModifiers (proc);
        }

        beginTest ("Result is clamped to the valid MIDI note range");
        {
            const int slot = addModifier (proc, MidiModType::PitchShift);
            setInt (proc, ParamIDs::genMidiOctave (0, slot), 0);
            setInt (proc, ParamIDs::genMidiTranspose (0, slot), 24);
            expectEquals (proc.applyMidiModifier (0, 120), 127);
            setInt (proc, ParamIDs::genMidiTranspose (0, slot), -24);
            expectEquals (proc.applyMidiModifier (0, 5), 0);
            clearModifiers (proc);
        }

        beginTest ("Key Shift quantizes out-of-scale notes to the nearest scale tone");
        {
            const int slot = addModifier (proc, MidiModType::KeyShift);
            setChoice (proc, ParamIDs::genMidiKeyRoot (0, slot), 0);  // C
            setChoice (proc, ParamIDs::genMidiKeyScale (0, slot), 0); // Major

            // C major = {C D E F G A B}. C#4 (61) sits a semitone above C4 and
            // a whole tone below D4, so the nearer scale tone is C4 (60).
            expectEquals (proc.applyMidiModifier (0, 61), 60);

            // Notes already in the scale are left alone.
            expectEquals (proc.applyMidiModifier (0, 62), 62); // D4

            clearModifiers (proc);
        }

        beginTest ("Modifiers chain in order: Pitch Shift then Key Shift");
        {
            const int pitchSlot = addModifier (proc, MidiModType::PitchShift);
            const int keySlot   = addModifier (proc, MidiModType::KeyShift);
            setInt    (proc, ParamIDs::genMidiOctave    (0, pitchSlot), 0);
            setInt    (proc, ParamIDs::genMidiTranspose (0, pitchSlot), 1);  // C4 -> C#4
            setChoice (proc, ParamIDs::genMidiKeyRoot   (0, keySlot),  0);   // C
            setChoice (proc, ParamIDs::genMidiKeyScale  (0, keySlot),  0);   // Major

            // Transposed to C#4 (61) first, then quantized back to C4 (60).
            expectEquals (proc.applyMidiModifier (0, 60), 60);
            clearModifiers (proc);
        }

        beginTest ("isArpEnabled reflects whether the chain contains an Arp modifier");
        {
            expect (! proc.isArpEnabled (0));
            addModifier (proc, MidiModType::PitchShift);
            expect (! proc.isArpEnabled (0));
            addModifier (proc, MidiModType::Arp);
            expect (proc.isArpEnabled (0));
            clearModifiers (proc);
            expect (! proc.isArpEnabled (0));
        }
    }
};

static MidiModifierTests midiModifierTests;

} // namespace

#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace
{

void setInt (ViolentAudioProcessor& proc, const juce::String& id, int value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterInt*> (proc.apvts.getParameter (id)))
        *p = value;
}

void setBool (ViolentAudioProcessor& proc, const juce::String& id, bool value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterBool*> (proc.apvts.getParameter (id)))
        *p = value;
}

void setChoice (ViolentAudioProcessor& proc, const juce::String& id, int index)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
        *p = index;
}

class MidiModifierTests : public juce::UnitTest
{
public:
    MidiModifierTests() : juce::UnitTest ("MIDI modifier", "Processor") {}

    void runTest() override
    {
        ViolentAudioProcessor proc;

        beginTest ("The MIDI modifier stage is off by default, so notes pass through unchanged");
        {
            expect (! proc.isArpEnabled (0));
            setInt (proc, ParamIDs::genMidiTranspose (0), 5);
            expectEquals (proc.applyMidiModifier (0, 60), 60);
            setInt (proc, ParamIDs::genMidiTranspose (0), 0);
        }

        beginTest ("Enabling the stage lets sub-settings (e.g. transpose) take effect");
        {
            setBool (proc, ParamIDs::genMidiModEnabled (0), true);
            setInt (proc, ParamIDs::genMidiTranspose (0), 5);
            expectEquals (proc.applyMidiModifier (0, 60), 65);
            setInt (proc, ParamIDs::genMidiTranspose (0), 0);
        }

        beginTest ("Disabling the stage again bypasses it even with sub-settings still set");
        {
            setInt (proc, ParamIDs::genMidiTranspose (0), 5);
            setBool (proc, ParamIDs::genMidiModEnabled (0), false);
            expectEquals (proc.applyMidiModifier (0, 60), 60);
            setInt (proc, ParamIDs::genMidiTranspose (0), 0);
        }

        // The rest of these tests exercise the sub-settings themselves, so
        // they run with the modifier stage explicitly enabled throughout.
        setBool (proc, ParamIDs::genMidiModEnabled (0), true);

        beginTest ("Transpose shifts by semitones");
        {
            setInt (proc, ParamIDs::genMidiTranspose (0), 5);
            expectEquals (proc.applyMidiModifier (0, 60), 65);
            setInt (proc, ParamIDs::genMidiTranspose (0), 0);
        }

        beginTest ("Octave shifts by 12 semitones per octave, either direction");
        {
            setInt (proc, ParamIDs::genMidiOctave (0), 1);
            expectEquals (proc.applyMidiModifier (0, 60), 72);
            setInt (proc, ParamIDs::genMidiOctave (0), -2);
            expectEquals (proc.applyMidiModifier (0, 60), 36);
            setInt (proc, ParamIDs::genMidiOctave (0), 0);
        }

        beginTest ("Result is clamped to the valid MIDI note range");
        {
            setInt (proc, ParamIDs::genMidiTranspose (0), 24);
            expectEquals (proc.applyMidiModifier (0, 120), 127);
            setInt (proc, ParamIDs::genMidiTranspose (0), -24);
            expectEquals (proc.applyMidiModifier (0, 5), 0);
            setInt (proc, ParamIDs::genMidiTranspose (0), 0);
        }

        beginTest ("Key quantize snaps out-of-scale notes to the nearest scale tone");
        {
            setBool (proc, ParamIDs::genMidiKeyEnabled (0), true);
            setChoice (proc, ParamIDs::genMidiKeyRoot (0), 0);  // C
            setChoice (proc, ParamIDs::genMidiKeyScale (0), 0); // Major

            // C major = {C D E F G A B}. C#4 (61) sits a semitone above C4 and
            // a whole tone below D4, so the nearer scale tone is C4 (60).
            expectEquals (proc.applyMidiModifier (0, 61), 60);

            // Notes already in the scale are left alone.
            expectEquals (proc.applyMidiModifier (0, 62), 62); // D4

            setBool (proc, ParamIDs::genMidiKeyEnabled (0), false);
        }

        beginTest ("Arp flag reflects the bound parameter, but only while the stage is enabled");
        {
            expect (! proc.isArpEnabled (0));
            setBool (proc, ParamIDs::genMidiArpEnabled (0), true);
            expect (proc.isArpEnabled (0));

            setBool (proc, ParamIDs::genMidiModEnabled (0), false);
            expect (! proc.isArpEnabled (0));

            setBool (proc, ParamIDs::genMidiModEnabled (0), true);
            setBool (proc, ParamIDs::genMidiArpEnabled (0), false);
        }
    }
};

static MidiModifierTests midiModifierTests;

} // namespace

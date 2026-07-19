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

        beginTest ("No modification leaves note unchanged");
        {
            expectEquals (proc.applyMidiModifier (0, 60), 60);
        }

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

        beginTest ("Arp flag reflects the bound parameter");
        {
            expect (! proc.isArpEnabled (0));
            setBool (proc, ParamIDs::genMidiArpEnabled (0), true);
            expect (proc.isArpEnabled (0));
            setBool (proc, ParamIDs::genMidiArpEnabled (0), false);
        }
    }
};

static MidiModifierTests midiModifierTests;

} // namespace

#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace
{

void setChoice (ViolentAudioProcessor& proc, const juce::String& id, int index)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
        *p = index;
}

int getChoice (ViolentAudioProcessor& proc, const juce::String& id)
{
    return static_cast<int> (proc.apvts.getRawParameterValue (id)->load());
}

// Deletes a preset file on scope exit so tests never leave junk behind in the
// user's real presets folder (savePreset()/loadPreset() write to the actual
// ~/Library/Application Support path — there's no sandboxed test location).
struct ScopedPresetFile
{
    juce::File file;
    ~ScopedPresetFile() { file.deleteFile(); }
};

class PresetTests : public juce::UnitTest
{
public:
    PresetTests() : juce::UnitTest ("Presets", "Processor") {}

    void runTest() override
    {
        beginTest ("State XML round-trips generator name, colour, and source type");
        {
            ViolentAudioProcessor writer;
            setChoice (writer, ParamIDs::genSrcType (0), 2); // Square
            writer.generators[0].name = "My Synth";
            writer.generators[0].colour = juce::Colour (0xff112233);

            auto xml = writer.createStateXml();
            expect (xml != nullptr);

            ViolentAudioProcessor reader;
            reader.restoreStateFromXml (*xml);

            expectEquals (getChoice (reader, ParamIDs::genSrcType (0)), 2);
            expectEquals (reader.generators[0].name, juce::String ("My Synth"));
            expect (reader.generators[0].colour == juce::Colour (0xff112233));
        }

        beginTest ("State XML round-trips Effect Components (type, name, enabled, routing)");
        {
            ViolentAudioProcessor writer;
            writer.numEffects = 2;
            writer.effects[0].type = FxType::Compressor;
            writer.effects[0].name = "Comp Bus";
            writer.effects[0].enabled = false;
            writer.effects[0].routing[0] = true;
            writer.effects[0].routing[2] = true;
            writer.effects[1].type = FxType::Filter;
            writer.effects[1].name = "Filter Bus";

            auto xml = writer.createStateXml();
            expect (xml != nullptr);

            ViolentAudioProcessor reader;
            reader.restoreStateFromXml (*xml);

            expectEquals (reader.numEffects, 2);
            expect (reader.effects[0].type == FxType::Compressor);
            expectEquals (reader.effects[0].name, juce::String ("Comp Bus"));
            expect (! reader.effects[0].enabled);
            expect (reader.effects[0].routing[0]);
            expect (! reader.effects[0].routing[1]);
            expect (reader.effects[0].routing[2]);
            expect (reader.effects[1].type == FxType::Filter);
        }

        beginTest ("State XML round-trips MIDI Modifier Components (type, name, enabled, routing)");
        {
            ViolentAudioProcessor writer;
            writer.numMidiModifiers = 1;
            writer.midiModifiers[0].type = MidiModType::Arp;
            writer.midiModifiers[0].name = "My Arp";
            writer.midiModifiers[0].enabled = true;
            writer.midiModifiers[0].routing[1] = true;

            auto xml = writer.createStateXml();
            expect (xml != nullptr);

            ViolentAudioProcessor reader;
            reader.restoreStateFromXml (*xml);

            expectEquals (reader.numMidiModifiers, 1);
            expect (reader.midiModifiers[0].type == MidiModType::Arp);
            expectEquals (reader.midiModifiers[0].name, juce::String ("My Arp"));
            expect (reader.midiModifiers[0].routing[1]);
            expect (! reader.midiModifiers[0].routing[0]);
        }

        beginTest ("savePreset() writes a file and getPresetNames() lists it");
        {
            ViolentAudioProcessor proc;
            const juce::String name = "UnitTestPreset_DoNotKeep";
            ScopedPresetFile guard { proc.getPresetsDirectory().getChildFile (name + ".violentpreset") };

            setChoice (proc, ParamIDs::genSrcType (0), 2); // Square

            expect (proc.savePreset (name));
            expect (guard.file.existsAsFile());
            expect (proc.getPresetNames().contains (name));
            expectEquals (proc.currentPresetName, name);
        }

        beginTest ("loadPreset() restores previously saved parameter values");
        {
            ViolentAudioProcessor proc;
            const juce::String name = "UnitTestPreset_DoNotKeep";
            ScopedPresetFile guard { proc.getPresetsDirectory().getChildFile (name + ".violentpreset") };

            setChoice (proc, ParamIDs::genSrcType (0), 2); // Square
            expect (proc.savePreset (name));

            setChoice (proc, ParamIDs::genSrcType (0), 0); // change to Sine
            expectEquals (getChoice (proc, ParamIDs::genSrcType (0)), 0);

            expect (proc.loadPreset (name));
            expectEquals (getChoice (proc, ParamIDs::genSrcType (0)), 2);
        }

        beginTest ("loadPreset() fails gracefully for a preset that doesn't exist");
        {
            ViolentAudioProcessor proc;
            expect (! proc.loadPreset ("Definitely Not A Real Preset Name 12345"));
        }
    }
};

static PresetTests presetTests;

} // namespace

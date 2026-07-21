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
        beginTest ("State XML round-trips generator source type across processor instances");
        {
            ViolentAudioProcessor writer;
            setChoice (writer, ParamIDs::genSrcType (0), 2); // Square
            writer.generators[0].numFx = 1;
            writer.generators[0].fxTypes[0] = FxType::Filter;

            auto xml = writer.createStateXml();
            expect (xml != nullptr);

            ViolentAudioProcessor reader;
            reader.restoreStateFromXml (*xml);

            expectEquals (getChoice (reader, ParamIDs::genSrcType (0)), 2);
            expectEquals (reader.generators[0].numFx, 1);
            expect (reader.generators[0].fxTypes[0] == FxType::Filter);
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

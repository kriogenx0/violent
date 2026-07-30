#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace
{

void setFloat (ViolentAudioProcessor& proc, const juce::String& id, float value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterFloat*> (proc.apvts.getParameter (id)))
        *p = value;
}

class ModulatorTests : public juce::UnitTest
{
public:
    ModulatorTests() : juce::UnitTest ("Modulator", "Processor") {}

    void runTest() override
    {
        constexpr double sampleRate = 48000.0;
        constexpr int    blockSize  = 512;

        beginTest ("getModulatedValue() returns the raw value when no modulators target it");
        {
            ViolentAudioProcessor proc;
            expectWithinAbsoluteError (proc.getModulatedValue (ParamIDs::genSrcGain (0)), 0.0f, 0.0001f);
        }

        beginTest ("getModulatableParameters() lists generator source and effect params, not MIDI modifier or choice/bool params");
        {
            ViolentAudioProcessor proc;
            const auto params = proc.getModulatableParameters();

            auto contains = [&] (const juce::String& id)
            {
                for (auto& pr : params) if (pr.first == id) return true;
                return false;
            };

            expect (contains (ParamIDs::genSrcGain (0)));
            expect (contains (ParamIDs::effectDrive (0)));
            expect (! contains (ParamIDs::midiModTranspose (0))); // out of scope for this pass
            expect (! contains (ParamIDs::genSrcType (0)));       // choice, not continuous
            expect (! contains (ParamIDs::generatorEn (0)));      // bool, not continuous
        }

        beginTest ("A disabled Modulator has no effect even when routed to a target");
        {
            ViolentAudioProcessor proc;
            proc.setPlayConfigDetails (0, 2, sampleRate, blockSize);
            proc.prepareToPlay (sampleRate, blockSize);

            proc.numModulators = 1;
            auto& mod = proc.modulators[0];
            mod.sourceType    = ModulatorSourceType::LFO;
            mod.enabled       = false;
            mod.targetParamID = ParamIDs::genSrcGain (0);
            setFloat (proc, ParamIDs::modAmount (0), 1.0f);
            setFloat (proc, ParamIDs::modLfoRate (0), 5.0f);

            juce::AudioBuffer<float> buffer (2, blockSize);
            juce::MidiBuffer midi;
            for (int i = 0; i < 4; ++i) proc.processBlock (buffer, midi);

            expectWithinAbsoluteError (proc.getModulatedValue (ParamIDs::genSrcGain (0)), 0.0f, 0.0001f);
        }

        beginTest ("An enabled LFO Modulator routed to a target shifts its modulated value");
        {
            ViolentAudioProcessor proc;
            proc.setPlayConfigDetails (0, 2, sampleRate, blockSize);
            proc.prepareToPlay (sampleRate, blockSize);

            proc.numModulators = 1;
            auto& mod = proc.modulators[0];
            mod.sourceType    = ModulatorSourceType::LFO;
            mod.enabled       = true;
            mod.targetParamID = ParamIDs::genSrcGain (0);
            setFloat (proc, ParamIDs::modAmount (0), 1.0f);
            setFloat (proc, ParamIDs::modLfoRate (0), 5.0f);

            juce::AudioBuffer<float> buffer (2, blockSize);
            juce::MidiBuffer midi;
            proc.processBlock (buffer, midi);

            const float modulated = proc.getModulatedValue (ParamIDs::genSrcGain (0));
            expect (! juce::approximatelyEqual (modulated, 0.0f));

            // Clearing the target should immediately drop back to the raw value.
            mod.targetParamID = {};
            expectWithinAbsoluteError (proc.getModulatedValue (ParamIDs::genSrcGain (0)), 0.0f, 0.0001f);
        }

        beginTest ("An Envelope Modulator attacks on note-on and releases once the last note lifts");
        {
            ViolentAudioProcessor proc;
            proc.setPlayConfigDetails (0, 2, sampleRate, blockSize);
            proc.prepareToPlay (sampleRate, blockSize);

            proc.numModulators = 1;
            auto& mod = proc.modulators[0];
            mod.sourceType    = ModulatorSourceType::Envelope;
            mod.enabled       = true;
            mod.targetParamID = ParamIDs::genSrcGain (0);
            setFloat (proc, ParamIDs::modAmount (0), 1.0f);
            setFloat (proc, ParamIDs::modEnvAtt (0), 0.001f); // fast attack so one block is enough

            juce::AudioBuffer<float> buffer (2, blockSize);
            juce::MidiBuffer noteOn;
            noteOn.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);
            proc.processBlock (buffer, noteOn);

            const float duringNote = proc.getModulatedValue (ParamIDs::genSrcGain (0));
            expect (! juce::approximatelyEqual (duringNote, 0.0f));

            setFloat (proc, ParamIDs::modEnvRel (0), 0.001f); // fast release
            juce::MidiBuffer noteOff;
            noteOff.addEvent (juce::MidiMessage::noteOff (1, 60), 0);
            proc.processBlock (buffer, noteOff);

            juce::MidiBuffer empty;
            for (int i = 0; i < 4; ++i) proc.processBlock (buffer, empty); // let it finish releasing

            expectWithinAbsoluteError (proc.getModulatedValue (ParamIDs::genSrcGain (0)), 0.0f, 0.01f);
        }
    }
};

static ModulatorTests modulatorTests;

} // namespace

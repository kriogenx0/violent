#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace
{

juce::String textFor (ViolentAudioProcessor& proc, const juce::String& paramID, float value)
{
    auto* param = proc.apvts.getParameter (paramID);
    return param->getText (param->convertTo0to1 (value), 0);
}

class ParameterFormattingTests : public juce::UnitTest
{
public:
    ParameterFormattingTests() : juce::UnitTest ("Parameter formatting", "Processor") {}

    void runTest() override
    {
        beginTest ("Pan parameters (Source Pan and Mixer Pan) display as a percentage with L/R, not a raw -1..1 number");
        {
            ViolentAudioProcessor proc;

            for (const auto& paramID : { ParamIDs::genSrcPan (0), ParamIDs::generatorPan (0) })
            {
                expectEquals (textFor (proc, paramID, 0.0f), juce::String ("C"));
                expectEquals (textFor (proc, paramID, -1.0f), juce::String ("100% L"));
                expectEquals (textFor (proc, paramID, 1.0f), juce::String ("100% R"));
                expectEquals (textFor (proc, paramID, -0.5f), juce::String ("50% L"));
                expectEquals (textFor (proc, paramID, 0.5f), juce::String ("50% R"));
            }
        }
    }
};

static ParameterFormattingTests parameterFormattingTests;

} // namespace

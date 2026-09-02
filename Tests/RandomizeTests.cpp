#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace
{

class RandomizeTests : public juce::UnitTest
{
public:
    RandomizeTests() : juce::UnitTest ("Randomize", "Processor") {}

    void runTest() override
    {
        beginTest ("randomizeAll() never touches Gain, Pan, Level, or Velocity sensitivity");
        {
            ViolentAudioProcessor proc;

            const float gain0  = *proc.apvts.getRawParameterValue (ParamIDs::genSrcGain (0));
            const float pan0   = *proc.apvts.getRawParameterValue (ParamIDs::genSrcPan (0));
            const float vel0   = *proc.apvts.getRawParameterValue (ParamIDs::genSrcVel (0));
            const float level0 = *proc.apvts.getRawParameterValue (ParamIDs::generatorLevel (0));
            const float mixPan0 = *proc.apvts.getRawParameterValue (ParamIDs::generatorPan (0));

            for (int i = 0; i < 20; ++i)
                proc.randomizeAll();

            expectEquals (proc.apvts.getRawParameterValue (ParamIDs::genSrcGain (0))->load(), gain0);
            expectEquals (proc.apvts.getRawParameterValue (ParamIDs::genSrcPan (0))->load(), pan0);
            expectEquals (proc.apvts.getRawParameterValue (ParamIDs::genSrcVel (0))->load(), vel0);
            expectEquals (proc.apvts.getRawParameterValue (ParamIDs::generatorLevel (0))->load(), level0);
            expectEquals (proc.apvts.getRawParameterValue (ParamIDs::generatorPan (0))->load(), mixPan0);
        }

        beginTest ("randomizeAll() varies the number of active generators and effects, within valid bounds");
        {
            ViolentAudioProcessor proc;

            bool sawDifferentGeneratorCount = false, sawDifferentEffectCount = false;
            const int firstGenCount = proc.numActiveGenerators;
            const int firstFxCount  = 0; // effects start at 0 before any randomize

            for (int i = 0; i < 30; ++i)
            {
                proc.randomizeAll();

                expect (proc.numActiveGenerators >= 1 && proc.numActiveGenerators <= MAX_GENERATORS,
                        "generator count must stay within [1, MAX_GENERATORS]");
                expect (proc.numEffects >= 0 && proc.numEffects <= MAX_EFFECTS,
                        "effect count must stay within [0, MAX_EFFECTS]");

                if (proc.numActiveGenerators != firstGenCount) sawDifferentGeneratorCount = true;
                if (proc.numEffects != firstFxCount) sawDifferentEffectCount = true;
            }

            expect (sawDifferentGeneratorCount, "generator count should vary across repeated randomizes");
            expect (sawDifferentEffectCount, "effect count should vary across repeated randomizes");
        }

        beginTest ("randomizeAll() only assigns implemented effect types, and every active generator stays enabled and non-Sample");
        {
            ViolentAudioProcessor proc;

            for (int i = 0; i < 20; ++i)
            {
                proc.randomizeAll();

                for (int x = 0; x < proc.numEffects; ++x)
                {
                    const auto type = proc.effects[(size_t) x].type;
                    expect (type == FxType::Distortion || type == FxType::Compressor || type == FxType::Gate
                                || type == FxType::Reverb || type == FxType::Filter,
                            "every active effect should be one of the five implemented, addable types");
                    expect (proc.effects[(size_t) x].enabled, "a freshly randomized effect should be enabled");
                }

                for (int s = 0; s < proc.numActiveGenerators; ++s)
                {
                    expect (proc.apvts.getRawParameterValue (ParamIDs::generatorEn (s))->load() != 0.0f,
                            "every active generator should be enabled after randomize");
                    const int srcType = (int) proc.apvts.getRawParameterValue (ParamIDs::genSrcType (s))->load();
                    expect (srcType != (int) SourceType::Sample,
                            "randomize should never assign Sample mode (no loaded buffer means silence)");
                }
            }
        }
    }
};

static RandomizeTests randomizeTests;

} // namespace

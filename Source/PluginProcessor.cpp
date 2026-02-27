#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ViolentAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // EQ
    params.push_back (std::make_unique<AudioParameterBool>  (ParamIDs::EQ_ENABLED, "EQ Enabled", true));
    for (int i = 0; i < NUM_EQ_BANDS; ++i)
    {
        juce::String id   = juce::String ("eq_band_") + juce::String (i);
        juce::String name = juce::String ("EQ Band ")  + juce::String (i);
        params.push_back (std::make_unique<AudioParameterFloat> (
            id, name, NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
    }

    // Reverb
    params.push_back (std::make_unique<AudioParameterBool>  (ParamIDs::REVERB_ENABLED, "Reverb Enabled", false));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::REVERB_ROOM,    "Room Size",  NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::REVERB_DAMPING, "Damping",    NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::REVERB_WET,     "Wet Level",  NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.33f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::REVERB_WIDTH,   "Width",      NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    // Distortion
    params.push_back (std::make_unique<AudioParameterBool>  (ParamIDs::DIST_ENABLED, "Dist Enabled", false));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::DIST_DRIVE,   "Drive",
        NormalisableRange<float> (1.0f, 100.0f, 0.1f, 0.4f), 1.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::DIST_TONE,    "Tone",
        NormalisableRange<float> (200.0f, 8000.0f, 1.0f, 0.5f), 4000.0f,
        AudioParameterFloatAttributes().withLabel ("Hz")));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::DIST_LEVEL,   "Level",
        NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));
    params.push_back (std::make_unique<AudioParameterChoice> (ParamIDs::DIST_TYPE, "Dist Type",
        StringArray { "Soft Clip", "Hard Clip", "Fuzz" }, 0));

    // Compressor
    params.push_back (std::make_unique<AudioParameterBool>  (ParamIDs::COMP_ENABLED,   "Comp Enabled", false));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::COMP_THRESHOLD, "Threshold",
        NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -12.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::COMP_RATIO,     "Ratio",
        NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.5f), 4.0f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::COMP_ATTACK,    "Attack",
        NormalisableRange<float> (0.1f, 200.0f, 0.1f, 0.5f), 10.0f,
        AudioParameterFloatAttributes().withLabel ("ms")));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::COMP_RELEASE,   "Release",
        NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.5f), 100.0f,
        AudioParameterFloatAttributes().withLabel ("ms")));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::COMP_MAKEUP,    "Makeup",
        NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));

    // Gate
    params.push_back (std::make_unique<AudioParameterBool>  (ParamIDs::GATE_ENABLED,   "Gate Enabled", false));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::GATE_THRESHOLD, "Gate Threshold",
        NormalisableRange<float> (-96.0f, 0.0f, 0.1f), -40.0f,
        AudioParameterFloatAttributes().withLabel ("dB")));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::GATE_ATTACK,    "Gate Attack",
        NormalisableRange<float> (0.1f, 100.0f, 0.1f, 0.5f), 1.0f,
        AudioParameterFloatAttributes().withLabel ("ms")));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::GATE_RELEASE,   "Gate Release",
        NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.5f), 100.0f,
        AudioParameterFloatAttributes().withLabel ("ms")));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::GATE_RATIO,     "Gate Ratio",
        NormalisableRange<float> (1.0f, 100.0f, 0.1f, 0.4f), 10.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
ViolentAudioProcessor::ViolentAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Register as listener for all parameters so we can update DSP on change
    const juce::StringArray paramIDs {
        ParamIDs::EQ_ENABLED,
        ParamIDs::EQ_BAND_0, ParamIDs::EQ_BAND_1, ParamIDs::EQ_BAND_2,
        ParamIDs::EQ_BAND_3, ParamIDs::EQ_BAND_4, ParamIDs::EQ_BAND_5,
        ParamIDs::EQ_BAND_6, ParamIDs::EQ_BAND_7, ParamIDs::EQ_BAND_8,
        ParamIDs::EQ_BAND_9,
        ParamIDs::REVERB_ENABLED, ParamIDs::REVERB_ROOM, ParamIDs::REVERB_DAMPING,
        ParamIDs::REVERB_WET, ParamIDs::REVERB_WIDTH,
        ParamIDs::DIST_ENABLED, ParamIDs::DIST_DRIVE, ParamIDs::DIST_TONE,
        ParamIDs::DIST_LEVEL, ParamIDs::DIST_TYPE,
        ParamIDs::COMP_ENABLED, ParamIDs::COMP_THRESHOLD, ParamIDs::COMP_RATIO,
        ParamIDs::COMP_ATTACK, ParamIDs::COMP_RELEASE, ParamIDs::COMP_MAKEUP,
        ParamIDs::GATE_ENABLED, ParamIDs::GATE_THRESHOLD, ParamIDs::GATE_ATTACK,
        ParamIDs::GATE_RELEASE, ParamIDs::GATE_RATIO
    };

    for (const auto& id : paramIDs)
        apvts.addParameterListener (id, this);
}

ViolentAudioProcessor::~ViolentAudioProcessor()
{
    const juce::StringArray paramIDs {
        ParamIDs::EQ_ENABLED,
        ParamIDs::EQ_BAND_0, ParamIDs::EQ_BAND_1, ParamIDs::EQ_BAND_2,
        ParamIDs::EQ_BAND_3, ParamIDs::EQ_BAND_4, ParamIDs::EQ_BAND_5,
        ParamIDs::EQ_BAND_6, ParamIDs::EQ_BAND_7, ParamIDs::EQ_BAND_8,
        ParamIDs::EQ_BAND_9,
        ParamIDs::REVERB_ENABLED, ParamIDs::REVERB_ROOM, ParamIDs::REVERB_DAMPING,
        ParamIDs::REVERB_WET, ParamIDs::REVERB_WIDTH,
        ParamIDs::DIST_ENABLED, ParamIDs::DIST_DRIVE, ParamIDs::DIST_TONE,
        ParamIDs::DIST_LEVEL, ParamIDs::DIST_TYPE,
        ParamIDs::COMP_ENABLED, ParamIDs::COMP_THRESHOLD, ParamIDs::COMP_RATIO,
        ParamIDs::COMP_ATTACK, ParamIDs::COMP_RELEASE, ParamIDs::COMP_MAKEUP,
        ParamIDs::GATE_ENABLED, ParamIDs::GATE_THRESHOLD, ParamIDs::GATE_ATTACK,
        ParamIDs::GATE_RELEASE, ParamIDs::GATE_RATIO
    };

    for (const auto& id : paramIDs)
        apvts.removeParameterListener (id, this);
}

//==============================================================================
void ViolentAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    processSpec.sampleRate      = sampleRate;
    processSpec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    processSpec.numChannels      = static_cast<juce::uint32> (getTotalNumOutputChannels());

    // Initialise EQ band state Ptrs before prepare() so each mono filter
    // receives a valid (non-null) coefficients pointer.
    for (int i = 0; i < NUM_EQ_BANDS; ++i)
        eqBands[i].state = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sampleRate, static_cast<double> (EQ_FREQUENCIES[i]),
            static_cast<double> (EQ_Q), 1.0f);

    for (auto& band : eqBands)
        band.prepare (processSpec);

    // Initialise distortion tone filter state Ptr.
    distToneFilter.state = juce::dsp::IIR::Coefficients<float>::makeLowPass (
        sampleRate, 4000.0);
    distToneFilter.prepare (processSpec);

    compressor.prepare (processSpec);
    makeupGain.prepare (processSpec);
    noiseGate.prepare  (processSpec);
    reverb.prepare     (processSpec);

    prepared = true;
    updateDSP();
}

void ViolentAudioProcessor::releaseResources()
{
    prepared = false;
}

bool ViolentAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

//==============================================================================
void ViolentAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (paramsDirty.exchange (false))
        updateDSP();

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    // 1. EQ
    if (apvts.getRawParameterValue (ParamIDs::EQ_ENABLED)->load() > 0.5f)
        for (auto& band : eqBands)
            band.process (ctx);

    // 2. Distortion
    if (apvts.getRawParameterValue (ParamIDs::DIST_ENABLED)->load() > 0.5f)
    {
        const float drive  = apvts.getRawParameterValue (ParamIDs::DIST_DRIVE)->load();
        const float level  = juce::Decibels::decibelsToGain (
                                 apvts.getRawParameterValue (ParamIDs::DIST_LEVEL)->load());
        const int   type   = static_cast<int> (
                                 apvts.getRawParameterValue (ParamIDs::DIST_TYPE)->load());

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float* data = buffer.getWritePointer (ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float x = data[s] * drive;
                switch (type)
                {
                    case 0: // Soft clip  — tanh
                        x = std::tanh (x);
                        break;
                    case 1: // Hard clip
                        x = juce::jlimit (-1.0f, 1.0f, x);
                        break;
                    case 2: // Fuzz — asymmetric soft-clip + even-harmonic fold
                        x = std::tanh (x * 1.5f) + 0.1f * std::sin (x * juce::MathConstants<float>::pi);
                        x = juce::jlimit (-1.0f, 1.0f, x);
                        break;
                    default:
                        x = std::tanh (x);
                        break;
                }
                data[s] = x * level;
            }
        }

        // Tone filter (post-dist LP)
        distToneFilter.process (ctx);
    }

    // 3. Compressor
    if (apvts.getRawParameterValue (ParamIDs::COMP_ENABLED)->load() > 0.5f)
    {
        compressor.process (ctx);
        makeupGain.process (ctx);
    }

    // 4. Gate
    if (apvts.getRawParameterValue (ParamIDs::GATE_ENABLED)->load() > 0.5f)
        noiseGate.process (ctx);

    // 5. Reverb
    if (apvts.getRawParameterValue (ParamIDs::REVERB_ENABLED)->load() > 0.5f)
        reverb.process (ctx);
}

//==============================================================================
void ViolentAudioProcessor::parameterChanged (const juce::String&, float)
{
    paramsDirty = true;
}

void ViolentAudioProcessor::updateDSP()
{
    updateEQ();
    updateDistortion();
    updateCompressor();
    updateGate();
    updateReverb();
}

void ViolentAudioProcessor::updateEQ()
{
    if (!prepared) return;

    const double sr = processSpec.sampleRate;
    for (int i = 0; i < NUM_EQ_BANDS; ++i)
    {
        const juce::String id = juce::String ("eq_band_") + juce::String (i);
        const float gainDB    = apvts.getRawParameterValue (id)->load();
        *eqBands[i].state     = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                                    sr, static_cast<double> (EQ_FREQUENCIES[i]),
                                    static_cast<double> (EQ_Q),
                                    juce::Decibels::decibelsToGain (gainDB));
    }
}

void ViolentAudioProcessor::updateDistortion()
{
    if (!prepared) return;

    const float toneHz = apvts.getRawParameterValue (ParamIDs::DIST_TONE)->load();
    *distToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                                processSpec.sampleRate,
                                static_cast<double> (toneHz));
}

void ViolentAudioProcessor::updateCompressor()
{
    if (!prepared) return;

    const float thresh   = apvts.getRawParameterValue (ParamIDs::COMP_THRESHOLD)->load();
    const float ratio    = apvts.getRawParameterValue (ParamIDs::COMP_RATIO)->load();
    const float attackMs = apvts.getRawParameterValue (ParamIDs::COMP_ATTACK)->load();
    const float relMs    = apvts.getRawParameterValue (ParamIDs::COMP_RELEASE)->load();
    const float makeupDB = apvts.getRawParameterValue (ParamIDs::COMP_MAKEUP)->load();

    compressor.setThreshold (thresh);
    compressor.setRatio      (ratio);
    compressor.setAttack     (attackMs);
    compressor.setRelease    (relMs);
    makeupGain.setGainDecibels (makeupDB);
}

void ViolentAudioProcessor::updateGate()
{
    if (!prepared) return;

    const float thresh   = apvts.getRawParameterValue (ParamIDs::GATE_THRESHOLD)->load();
    const float ratio    = apvts.getRawParameterValue (ParamIDs::GATE_RATIO)->load();
    const float attackMs = apvts.getRawParameterValue (ParamIDs::GATE_ATTACK)->load();
    const float relMs    = apvts.getRawParameterValue (ParamIDs::GATE_RELEASE)->load();

    noiseGate.setThreshold (thresh);
    noiseGate.setRatio      (ratio);
    noiseGate.setAttack     (attackMs);
    noiseGate.setRelease    (relMs);
}

void ViolentAudioProcessor::updateReverb()
{
    if (!prepared) return;

    juce::dsp::Reverb::Parameters p;
    p.roomSize   = apvts.getRawParameterValue (ParamIDs::REVERB_ROOM)->load();
    p.damping    = apvts.getRawParameterValue (ParamIDs::REVERB_DAMPING)->load();
    p.wetLevel   = apvts.getRawParameterValue (ParamIDs::REVERB_WET)->load();
    p.dryLevel   = 1.0f - p.wetLevel;
    p.width      = apvts.getRawParameterValue (ParamIDs::REVERB_WIDTH)->load();
    p.freezeMode = 0.0f;
    reverb.setParameters (p);
}

//==============================================================================
juce::AudioProcessorEditor* ViolentAudioProcessor::createEditor()
{
    return new ViolentAudioProcessorEditor (*this);
}

void ViolentAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void ViolentAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViolentAudioProcessor();
}

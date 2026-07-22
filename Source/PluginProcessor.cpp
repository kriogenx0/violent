#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
ViolentAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // ---- Generators ----
    for (int s = 0; s < MAX_GENERATORS; ++s)
    {
        const String sn = " Generator " + String (s + 1) + " ";

        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::generatorEn (s),    sn + "Enabled", s == 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::generatorLevel (s), sn + "Level",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::generatorPan (s),   sn + "Pan",
            NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));

        // MIDI modifier — applied to notes before they reach this generator.
        // The whole stage is optional and off by default; when disabled,
        // notes pass through unchanged regardless of the sub-settings below.
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::genMidiModEnabled (s), sn + "MIDI Modifier Enabled", false));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::genMidiTranspose (s), sn + "MIDI Transpose", -24, 24, 0));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::genMidiOctave (s),    sn + "MIDI Octave", -3, 3, 0));
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::genMidiKeyEnabled (s), sn + "MIDI Key Enabled", false));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::genMidiKeyRoot (s), sn + "MIDI Key Root",
            StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::genMidiKeyScale (s), sn + "MIDI Key Scale",
            StringArray { "Major", "Minor" }, 0));
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::genMidiArpEnabled (s), sn + "MIDI Arp Enabled", false));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genMidiArpRate (s), sn + "MIDI Arp Rate",
            NormalisableRange<float> (0.05f, 1.0f, 0.001f, 0.5f), 0.15f,
            AudioParameterFloatAttributes().withLabel ("s")));

        // Source (osc/sampler unified)
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::genSrcType (s),      sn + "Source Type",
            StringArray { "Sine", "Saw", "Square", "Triangle", "Noise", "Sample" }, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcGain (s),      sn + "Source Gain",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::genSrcOct (s),       sn + "Source Octave", -3, 3, 0));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::genSrcSemi (s),      sn + "Source Semitone", -12, 12, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcDet (s),       sn + "Source Detune",
            NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("ct")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcPhase (s),     sn + "Source Phase",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcPW (s),        sn + "Source Pulse Width",
            NormalisableRange<float> (0.05f, 0.95f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcPan (s),       sn + "Source Pan",
            NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcVel (s),       sn + "Source Velocity",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::genSrcUni (s),       sn + "Source Unison", 1, 8, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcUniSpread (s), sn + "Source Unison Spread",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 15.0f,
            AudioParameterFloatAttributes().withLabel ("ct")));
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::genSrcLoop (s),      sn + "Source Loop", false));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcAtt (s), sn + "Source Attack",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.01f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcDec (s), sn + "Source Decay",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.1f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcSus (s), sn + "Source Sustain",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.7f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcRel (s), sn + "Source Release",
            NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.3f), 0.3f,
            AudioParameterFloatAttributes().withLabel ("s")));

        // FX
        for (int x = 0; x < MAX_GENERATOR_FX; ++x)
        {
            const String xn = sn + "FX" + String (x) + " ";
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxDrive    (s, x), xn + "Drive",
                NormalisableRange<float> (1.0f, 100.0f, 0.1f, 0.4f), 1.0f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxTone     (s, x), xn + "Tone",
                NormalisableRange<float> (200.0f, 8000.0f, 1.0f, 0.5f), 4000.0f,
                AudioParameterFloatAttributes().withLabel ("Hz")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxLevel    (s, x), xn + "Level",
                NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
                AudioParameterFloatAttributes().withLabel ("dB")));
            params.push_back (std::make_unique<AudioParameterChoice> (
                ParamIDs::genFxDistType (s, x), xn + "Dist Type",
                StringArray { "Soft Clip", "Hard Clip", "Fuzz" }, 0));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxThresh   (s, x), xn + "Threshold",
                NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -12.0f,
                AudioParameterFloatAttributes().withLabel ("dB")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxRatio    (s, x), xn + "Ratio",
                NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.5f), 4.0f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxAttack   (s, x), xn + "Attack",
                NormalisableRange<float> (0.1f, 200.0f, 0.1f, 0.5f), 10.0f,
                AudioParameterFloatAttributes().withLabel ("ms")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxRelease  (s, x), xn + "Release",
                NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.5f), 100.0f,
                AudioParameterFloatAttributes().withLabel ("ms")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxMakeup   (s, x), xn + "Makeup",
                NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f,
                AudioParameterFloatAttributes().withLabel ("dB")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxRoom     (s, x), xn + "Room",
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxDamping  (s, x), xn + "Damping",
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxWet      (s, x), xn + "Wet",
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.33f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxWidth    (s, x), xn + "Width",
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
            params.push_back (std::make_unique<AudioParameterChoice> (
                ParamIDs::genFxFilterType (s, x), xn + "Filter Type",
                StringArray { "LP", "HP", "BP", "Notch" }, 0));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxFilterCut (s, x), xn + "Filter Cutoff",
                NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 8000.0f,
                AudioParameterFloatAttributes().withLabel ("Hz")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFxFilterRes (s, x), xn + "Filter Resonance",
                NormalisableRange<float> (0.1f, 12.0f, 0.01f, 0.5f), 0.707f));
        }

        // Sends to shared FX buses
        for (int b = 0; b < MAX_FX_BUSES; ++b)
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genSend (s, b), sn + "Send " + String (b + 1),
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    }

    // ---- Shared FX buses (generators send into these; each runs one effect) ----
    for (int b = 0; b < MAX_FX_BUSES; ++b)
    {
        const String bn = " Bus " + String (b + 1) + " ";
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busDrive    (b), bn + "Drive",
            NormalisableRange<float> (1.0f, 100.0f, 0.1f, 0.4f), 1.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busTone     (b), bn + "Tone",
            NormalisableRange<float> (200.0f, 8000.0f, 1.0f, 0.5f), 4000.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busLevel    (b), bn + "Level",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::busDistType (b), bn + "Dist Type",
            StringArray { "Soft Clip", "Hard Clip", "Fuzz" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busThresh   (b), bn + "Threshold",
            NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -12.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busRatio    (b), bn + "Ratio",
            NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.5f), 4.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busAttack   (b), bn + "Attack",
            NormalisableRange<float> (0.1f, 200.0f, 0.1f, 0.5f), 10.0f,
            AudioParameterFloatAttributes().withLabel ("ms")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busRelease  (b), bn + "Release",
            NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.5f), 100.0f,
            AudioParameterFloatAttributes().withLabel ("ms")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busMakeup   (b), bn + "Makeup",
            NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busRoom     (b), bn + "Room",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busDamping  (b), bn + "Damping",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busWet      (b), bn + "Wet",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.33f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busWidth    (b), bn + "Width",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::busFilterType (b), bn + "Filter Type",
            StringArray { "LP", "HP", "BP", "Notch" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busFilterCut (b), bn + "Filter Cutoff",
            NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 8000.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::busFilterRes (b), bn + "Filter Resonance",
            NormalisableRange<float> (0.1f, 12.0f, 0.01f, 0.5f), 0.707f));
    }

    // ---- Master filters (applied last, after all generators) ----
    for (int f = 0; f < MAX_MASTER_FILTERS; ++f)
    {
        const String fn = "Master Filter " + String (f + 1) + " ";
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::masterFltType (f), fn + "Type",
            StringArray { "LP", "HP", "BP", "Notch" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::masterFltCut (f), fn + "Cutoff",
            NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 8000.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::masterFltRes (f), fn + "Resonance",
            NormalisableRange<float> (0.1f, 12.0f, 0.01f, 0.5f), 0.707f));
    }

    // ---- Global EQ ----
    params.push_back (std::make_unique<AudioParameterBool> (
        ParamIDs::EQ_ENABLED, "EQ Enabled", false));
    for (int i = 0; i < NUM_EQ_BANDS; ++i)
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::eqBand (i), "EQ Band " + String (i),
            NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));

    // ---- Sample slots (legacy, kept for future use) ----
    for (int i = 0; i < MAX_SAMPLES; ++i)
    {
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::smpEn (i), "Sample " + String (i + 1) + " Enabled", false));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::smpRoot (i), "Sample " + String (i + 1) + " Root", 0, 127, 60));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpGain (i), "Sample " + String (i + 1) + " Gain",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpAtt  (i), "Sample " + String (i + 1) + " Attack",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.005f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpDec  (i), "Sample " + String (i + 1) + " Decay",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.1f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpSus  (i), "Sample " + String (i + 1) + " Sustain",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpRel  (i), "Sample " + String (i + 1) + " Release",
            NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.3f), 0.2f));
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::smpLoop (i), "Sample " + String (i + 1) + " Loop", false));
    }

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::StringArray& ViolentAudioProcessor::eqParamIDs()
{
    static const juce::StringArray ids {
        ParamIDs::EQ_ENABLED,
        ParamIDs::EQ_BAND_0, ParamIDs::EQ_BAND_1, ParamIDs::EQ_BAND_2,
        ParamIDs::EQ_BAND_3, ParamIDs::EQ_BAND_4, ParamIDs::EQ_BAND_5,
        ParamIDs::EQ_BAND_6, ParamIDs::EQ_BAND_7, ParamIDs::EQ_BAND_8,
        ParamIDs::EQ_BAND_9
    };
    return ids;
}

//==============================================================================
ViolentAudioProcessor::ViolentAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats();
    for (const auto& id : eqParamIDs())
        apvts.addParameterListener (id, this);
}

ViolentAudioProcessor::~ViolentAudioProcessor()
{
    for (const auto& id : eqParamIDs())
        apvts.removeParameterListener (id, this);
}

//==============================================================================
void ViolentAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    processSpec.sampleRate       = sampleRate;
    processSpec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    processSpec.numChannels      = 2;

    for (auto& v : voices) v.prepare (sampleRate);

    for (auto& dsp : generatorDSP)
        dsp.prepare (processSpec);

    for (auto& band : eqBands)
        band.prepare (processSpec);
    setEQBand (0, 0.0f);   // init state

    for (auto& f : masterFilterDSP) f.prepare (processSpec);
    masterFilterScratch.setSize (2, samplesPerBlock);

    for (auto& fx : fxBusDSP) fx.prepare (processSpec);

    waveformRingSize = juce::jmax (64, (int) std::ceil (
        sampleRate * waveformWindowOptionsMs.back() / 1000.0));
    for (auto& ring : waveformRing)
        ring.assign ((size_t) waveformRingSize, 0.0f);
    waveformRingWritePos.fill (0);

    previewSequence.clear(); // force a rebuild once processSpec.sampleRate is valid

    prepared = true;
    updateEQ();
}

void ViolentAudioProcessor::releaseResources()
{
    allNotesOff();
    prepared = false;
}

bool ViolentAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo()
        || out == juce::AudioChannelSet::mono()
        || out == juce::AudioChannelSet::disabled();
}

//==============================================================================
void ViolentAudioProcessor::loadGeneratorParams (int s)
{
    auto& dsp = generatorDSP[(size_t) s];
    auto& gen  = generators[(size_t) s];

    gen.enabled = apvts.getRawParameterValue (ParamIDs::generatorEn (s))->load() > 0.5f;
    dsp.level  = apvts.getRawParameterValue (ParamIDs::generatorLevel (s))->load();
    dsp.pan    = apvts.getRawParameterValue (ParamIDs::generatorPan   (s))->load();

    auto& o    = dsp.osc;
    o.en          = gen.enabled;
    o.type        = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcType (s))->load());
    o.gainLin     = juce::Decibels::decibelsToGain (
                        apvts.getRawParameterValue (ParamIDs::genSrcGain (s))->load());
    o.octave      = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcOct  (s))->load());
    o.semitone    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcSemi (s))->load());
    o.detune      = apvts.getRawParameterValue (ParamIDs::genSrcDet       (s))->load();
    o.phase       = apvts.getRawParameterValue (ParamIDs::genSrcPhase     (s))->load();
    o.pulseWidth  = apvts.getRawParameterValue (ParamIDs::genSrcPW        (s))->load();
    o.pan         = apvts.getRawParameterValue (ParamIDs::genSrcPan       (s))->load();
    o.velSens     = apvts.getRawParameterValue (ParamIDs::genSrcVel       (s))->load();
    o.unisonVoices= static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcUni (s))->load());
    o.unisonSpread= apvts.getRawParameterValue (ParamIDs::genSrcUniSpread (s))->load();
    o.att         = apvts.getRawParameterValue (ParamIDs::genSrcAtt (s))->load();
    o.dec         = apvts.getRawParameterValue (ParamIDs::genSrcDec (s))->load();
    o.sus         = apvts.getRawParameterValue (ParamIDs::genSrcSus (s))->load();
    o.rel         = apvts.getRawParameterValue (ParamIDs::genSrcRel (s))->load();

    for (int b = 0; b < MAX_FX_BUSES; ++b)
        dsp.sendGain[(size_t) b] = apvts.getRawParameterValue (ParamIDs::genSend (s, b))->load();
}

//==============================================================================
void ViolentAudioProcessor::processMidi (const juce::MidiBuffer& midi)
{
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
        {
            const int   rawNote = msg.getNoteNumber();
            const float vel     = msg.getFloatVelocity();
            for (int s = 0; s < numActiveGenerators; ++s)
            {
                if (!generators[(size_t) s].enabled) continue;
                const int note = applyMidiModifier (s, rawNote);

                if (isArpEnabled (s))
                {
                    midiModState[(size_t) s].heldNotes.push_back (note);
                }
                else
                {
                    int vi = findFreeVoice (s);
                    if (vi < 0) vi = findVoiceToSteal (s);
                    if (vi >= 0) startVoice (vi, note, vel, s);
                }
            }
        }
        else if (msg.isNoteOff())
        {
            const int rawNote = msg.getNoteNumber();
            for (int s = 0; s < numActiveGenerators; ++s)
            {
                const int note = applyMidiModifier (s, rawNote);
                if (isArpEnabled (s))
                {
                    auto& held = midiModState[(size_t) s].heldNotes;
                    auto it = std::find (held.begin(), held.end(), note);
                    if (it != held.end()) held.erase (it);
                }
                else
                {
                    stopNoteForGenerator (s, note);
                }
            }
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            allNotesOff();
            for (auto& st : midiModState) st.heldNotes.clear();
        }
    }
}

int ViolentAudioProcessor::applyMidiModifier (int s, int note) const
{
    if (apvts.getRawParameterValue (ParamIDs::genMidiModEnabled (s))->load() <= 0.5f)
        return note;

    const int transpose = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genMidiTranspose (s))->load());
    const int octave    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genMidiOctave (s))->load());
    int result = note + transpose + octave * 12;

    if (apvts.getRawParameterValue (ParamIDs::genMidiKeyEnabled (s))->load() > 0.5f)
    {
        const int root  = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genMidiKeyRoot (s))->load());
        const int scale = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genMidiKeyScale (s))->load());
        static constexpr int majorIntervals[] { 0, 2, 4, 5, 7, 9, 11 };
        static constexpr int minorIntervals[] { 0, 2, 3, 5, 7, 8, 10 };
        const auto& intervals = (scale == 0) ? majorIntervals : minorIntervals;

        const int pitchClass = ((result - root) % 12 + 12) % 12;
        int bestInterval = intervals[0], bestDist = 999;
        for (int iv : intervals)
        {
            const int dist = juce::jmin (std::abs (pitchClass - iv), 12 - std::abs (pitchClass - iv));
            if (dist < bestDist) { bestDist = dist; bestInterval = iv; }
        }
        result += (bestInterval - pitchClass);
    }

    return juce::jlimit (0, 127, result);
}

bool ViolentAudioProcessor::isArpEnabled (int s) const
{
    if (apvts.getRawParameterValue (ParamIDs::genMidiModEnabled (s))->load() <= 0.5f)
        return false;

    return apvts.getRawParameterValue (ParamIDs::genMidiArpEnabled (s))->load() > 0.5f;
}

void ViolentAudioProcessor::renderMidiModifiers (int numSamples)
{
    for (int s = 0; s < numActiveGenerators; ++s)
    {
        auto& st = midiModState[(size_t) s];

        if (! isArpEnabled (s))
        {
            if (st.arpNoteIsOn)
            {
                stopNoteForGenerator (s, st.arpCurrentNote);
                st.arpNoteIsOn = false;
            }
            st.arpStepIndex = -1;
            st.arpSamplePos = st.arpNextEventSample = 0;
            st.heldNotes.clear();
            continue;
        }

        if (st.heldNotes.empty())
        {
            if (st.arpNoteIsOn)
            {
                stopNoteForGenerator (s, st.arpCurrentNote);
                st.arpNoteIsOn = false;
            }
            st.arpStepIndex = -1;
            st.arpSamplePos = st.arpNextEventSample = 0;
            continue;
        }

        const float rateSeconds  = apvts.getRawParameterValue (ParamIDs::genMidiArpRate (s))->load();
        const int samplesPerStep = juce::jmax (1, static_cast<int> (processSpec.sampleRate * rateSeconds));
        const int gateSamples    = static_cast<int> (samplesPerStep * 0.7f);

        int samplesProcessed = 0;
        while (samplesProcessed < numSamples)
        {
            const int samplesUntilEvent = st.arpNextEventSample - st.arpSamplePos;
            if (samplesUntilEvent > numSamples - samplesProcessed)
            {
                st.arpSamplePos += (numSamples - samplesProcessed);
                break;
            }

            const int advance = juce::jmax (0, samplesUntilEvent);
            st.arpSamplePos    += advance;
            samplesProcessed   += advance;

            if (st.arpNoteIsOn)
            {
                stopNoteForGenerator (s, st.arpCurrentNote);
                st.arpNoteIsOn        = false;
                st.arpNextEventSample = st.arpSamplePos + (samplesPerStep - gateSamples);
            }
            else
            {
                st.arpStepIndex = (st.arpStepIndex + 1) % (int) st.heldNotes.size();
                const int note  = st.heldNotes[(size_t) st.arpStepIndex];

                int vi = findFreeVoice (s);
                if (vi < 0) vi = findVoiceToSteal (s);
                if (vi >= 0) startVoice (vi, note, 1.0f, s);

                st.arpCurrentNote     = note;
                st.arpNoteIsOn        = true;
                st.arpNextEventSample = st.arpSamplePos + gateSamples;
            }
        }
    }
}

int ViolentAudioProcessor::findFreeVoice (int generatorIdx) const noexcept
{
    for (int i = 0; i < GENERATOR_VOICES; ++i)
        if (!voices[(size_t) i].active && voices[(size_t) i].generatorIdx == generatorIdx)
            return i;
    // Also accept unassigned (default generatorIdx == 0)
    for (int i = 0; i < GENERATOR_VOICES; ++i)
        if (!voices[(size_t) i].active) return i;
    return -1;
}

int ViolentAudioProcessor::findVoiceToSteal (int generatorIdx) const noexcept
{
    // Steal the oldest released voice from this generator, else any
    for (int i = 0; i < GENERATOR_VOICES; ++i)
        if (voices[(size_t) i].generatorIdx == generatorIdx && !voices[(size_t) i].isActive())
            return i;
    return 0;
}

void ViolentAudioProcessor::startVoice (int vi, int note, float velocity, int generatorIdx)
{
    auto& v       = voices[(size_t) vi];
    v.note        = note;
    v.generatorIdx   = generatorIdx;
    v.velocity    = velocity;
    v.baseFreqHz  = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (note));
    v.active      = true;

    const auto& o = generatorDSP[(size_t) generatorIdx].osc;
    for (auto& osc : v.osc) osc.reset (o.phase);
    v.adsr.setParams (o.att, o.dec, o.sus, o.rel);
    v.adsr.noteOn();
}

void ViolentAudioProcessor::stopNote (int note)
{
    for (auto& v : voices)
        if (v.active && v.note == note)
            v.adsr.noteOff();
}

void ViolentAudioProcessor::stopNoteForGenerator (int generatorIdx, int note)
{
    for (auto& v : voices)
        if (v.active && v.generatorIdx == generatorIdx && v.note == note)
            v.adsr.noteOff();
}

void ViolentAudioProcessor::allNotesOff()
{
    for (auto& v : voices) v.reset();
}

//==============================================================================
void ViolentAudioProcessor::setPreviewPattern (int pattern)
{
    previewPattern.store (pattern, std::memory_order_relaxed);
    previewSequence.clear();
}

void ViolentAudioProcessor::buildPreviewSequence()
{
    previewSequence.clear();

    constexpr int root = 60; // C4
    const auto pattern = static_cast<PreviewPattern> (previewPattern.load (std::memory_order_relaxed));
    double noteSeconds = 0.15;

    switch (pattern)
    {
        case PreviewPattern::LowNotes:
        {
            noteSeconds = 0.3;
            constexpr int lowRoot = 36; // C2
            constexpr int steps[] { 0, 3, 5, 7, 10, 12 };
            for (int oct = 0; oct < 2; ++oct)
                for (int step : steps)
                    previewSequence.push_back ({ lowRoot + oct * 12 + step });
            break;
        }

        case PreviewPattern::LongSingleNotes:
        {
            noteSeconds = 1.2;
            constexpr int steps[] { 0, 4, 7, 12, 7, 4 }; // root - 3rd - 5th - octave - 5th - 3rd
            for (int step : steps)
                previewSequence.push_back ({ root + step });
            break;
        }

        case PreviewPattern::Chords:
        {
            noteSeconds = 0.6;
            constexpr int progressions[][3] {
                { 0, 4, 7 },    // I   (C major)
                { 5, 9, 12 },   // IV  (F major)
                { 7, 11, 14 },  // V   (G major)
                { -3, 0, 4 },   // vi  (A minor)
            };
            for (auto& chord : progressions)
                previewSequence.push_back ({ root + chord[0], root + chord[1], root + chord[2] });
            break;
        }

        case PreviewPattern::Arpeggios:
        default:
        {
            constexpr int majorScaleSteps[] { 0, 2, 4, 5, 7, 9, 11 };
            constexpr int pentatonicSteps[] { 0, 2, 4, 7, 9 };

            // C major scale, ascending across 3 octaves (plus the top C).
            for (int oct = 0; oct < 3; ++oct)
                for (int step : majorScaleSteps)
                    previewSequence.push_back ({ root + oct * 12 + step });
            previewSequence.push_back ({ root + 3 * 12 });

            // C major pentatonic arpeggio, ascending across the same 3 octaves.
            for (int oct = 0; oct < 3; ++oct)
                for (int step : pentatonicSteps)
                    previewSequence.push_back ({ root + oct * 12 + step });
            previewSequence.push_back ({ root + 3 * 12 });
            break;
        }
    }

    previewSamplesPerNote = static_cast<int> (processSpec.sampleRate * noteSeconds);
    previewGateSamples    = static_cast<int> (previewSamplesPerNote * 0.82);
}

void ViolentAudioProcessor::renderPreviewMidi (juce::MidiBuffer& midi, int numSamples)
{
    if (! previewActive.load (std::memory_order_relaxed))
    {
        if (previewNoteIsOn)
        {
            for (int note : previewSequence[(size_t) previewStepIndex])
                midi.addEvent (juce::MidiMessage::noteOff (1, note), 0);
            previewNoteIsOn = false;
        }
        previewStepIndex       = -1;
        previewSamplePos       = 0;
        previewNextEventSample = 0;
        return;
    }

    if (previewSequence.empty())
        buildPreviewSequence();

    int samplesProcessed = 0;
    while (samplesProcessed < numSamples)
    {
        const int samplesUntilEvent = previewNextEventSample - previewSamplePos;

        if (samplesUntilEvent > numSamples - samplesProcessed)
        {
            previewSamplePos += (numSamples - samplesProcessed);
            break;
        }

        const int advance = juce::jmax (0, samplesUntilEvent);
        previewSamplePos    += advance;
        samplesProcessed    += advance;

        if (previewNoteIsOn)
        {
            for (int note : previewSequence[(size_t) previewStepIndex])
                midi.addEvent (juce::MidiMessage::noteOff (1, note), samplesProcessed);
            previewNoteIsOn         = false;
            previewNextEventSample  = previewSamplePos + (previewSamplesPerNote - previewGateSamples);
        }
        else
        {
            previewStepIndex = (previewStepIndex + 1) % (int) previewSequence.size();
            for (int note : previewSequence[(size_t) previewStepIndex])
                midi.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) 100), samplesProcessed);
            previewNoteIsOn         = true;
            previewNextEventSample  = previewSamplePos + previewGateSamples;
        }
    }
}

//==============================================================================
void ViolentAudioProcessor::renderGenerator (int s, juce::AudioBuffer<float>& master)
{
    auto& dsp = generatorDSP[(size_t) s];
    auto& gen  = generators[(size_t) s];

    const int numSamples = master.getNumSamples();

    // Clear scratch first so a disabled/silent generator always reads back
    // as silence in the later master-mix stage, rather than stale data.
    dsp.scratch.setSize (2, numSamples, false, false, true);
    dsp.scratch.clear();

    if (!gen.enabled)
    {
        generatorLevelMeter[(size_t) s].store (0.0f, std::memory_order_relaxed);
        return;
    }

    const float sr = static_cast<float> (processSpec.sampleRate);
    const auto& o  = dsp.osc;

    float* L = dsp.scratch.getWritePointer (0);
    float* R = dsp.scratch.getWritePointer (1);

    // Render all voices belonging to this generator
    bool anyActive = false;
    for (auto& v : voices)
    {
        if (!v.active || v.generatorIdx != s) continue;
        anyActive = true;

        const float pitchCents = o.detune + (o.octave * 12 + o.semitone) * 100.0f;
        const int   nUni       = o.unisonVoices;
        const float vol        = o.gainLin
                                 * (1.0f - o.velSens * (1.0f - v.velocity));

        for (int n = 0; n < numSamples; ++n)
        {
            const float env = v.adsr.tick();
            float uniL = 0.0f, uniR = 0.0f;

            for (int u = 0; u < nUni; ++u)
            {
                const float uSpread = nUni > 1
                    ? o.unisonSpread * (u / (float)(nUni - 1) - 0.5f) * 2.0f
                    : 0.0f;

                float sample;
                if (o.type == (int) SourceType::Sample)
                {
                    // Sample playback — use the generator's sample slot if loaded
                    // (generator index maps to sample slot index)
                    juce::SpinLock::ScopedTryLockType tryLock (sampleModules[(size_t) s].lock);
                    if (tryLock.isLocked() && sampleModules[(size_t) s].hasData)
                    {
                        const auto& sm   = sampleModules[(size_t) s];
                        const double pr  = std::pow (2.0, (v.note - 60) / 12.0);
                        sample = v.osc[u].phase < (float) sm.buffer.getNumSamples()
                                   ? sm.buffer.getSample (0, (int) v.osc[u].phase)
                                   : 0.0f;
                        v.osc[u].phase += (float) pr;
                        if (v.osc[u].phase >= sm.buffer.getNumSamples()) v.osc[u].phase = 0.0f;
                    }
                    else { sample = 0.0f; }
                }
                else
                {
                    sample = v.osc[u].tick (v.baseFreqHz, pitchCents + uSpread,
                                            sr, o.type, o.pulseWidth);
                }

                const float panL = juce::jlimit (0.0f, 1.0f, 1.0f - o.pan);
                const float panR = juce::jlimit (0.0f, 1.0f, 1.0f + o.pan);
                uniL += sample * panL;
                uniR += sample * panR;
            }

            const float out = (uniL > 0.0f || uniR > 0.0f
                                   ? (uniL + uniR) / (float) nUni : 0.0f);
            L[n] += uniL / (float) nUni * env * vol;
            R[n] += uniR / (float) nUni * env * vol;
        }

        if (!v.isActive()) v.active = false;
    }

    // Roll the raw source waveform (pre-filter/FX) into the ring buffer for
    // the UI scope; see waveformRing's declaration for why this always
    // writes the full history rather than just whatever window is selected.
    if (waveformRingSize > 0)
    {
        auto& ring = waveformRing[(size_t) s];
        int& pos = waveformRingWritePos[(size_t) s];
        for (int i = 0; i < numSamples; ++i)
        {
            ring[(size_t) pos] = L[i];
            pos = (pos + 1 == waveformRingSize) ? 0 : pos + 1;
        }
    }

    if (!anyActive)
    {
        generatorLevelMeter[(size_t) s].store (0.0f, std::memory_order_relaxed);
        return;
    }

    // Generator FX (filters are just another FX-chain entry)
    applyGeneratorFx (s, dsp, gen, dsp.scratch);

    generatorLevelMeter[(size_t) s].store (dsp.scratch.getMagnitude (0, 0, numSamples), std::memory_order_relaxed);

    // Sends — accumulate this generator's (post-FX) signal into shared FX
    // buses, alongside (not instead of) the normal master mix below.
    for (int b = 0; b < numFxBuses; ++b)
    {
        const float send = dsp.sendGain[(size_t) b];
        if (send <= 0.0f) continue;
        fxBusScratch[(size_t) b].addFrom (0, 0, dsp.scratch, 0, 0, numSamples, send);
        fxBusScratch[(size_t) b].addFrom (1, 0, dsp.scratch, 1, 0, numSamples, send);
    }

    // Mixing into the master bus happens afterwards, in mixGeneratorsToMaster() —
    // that's where master filter routing decides how each generator gets summed in.
}

void ViolentAudioProcessor::mixGeneratorsToMaster (juce::AudioBuffer<float>& master)
{
    const int numSamples = master.getNumSamples();

    std::array<bool, MAX_GENERATORS> claimed {};

    for (int f = 0; f < numMasterFilters; ++f)
    {
        auto& mf = masterFilters[(size_t) f];
        if (! mf.enabled) continue;

        masterFilterScratch.setSize (2, numSamples, false, false, true);
        masterFilterScratch.clear();

        bool any = false;
        for (int s = 0; s < numActiveGenerators; ++s)
        {
            if (! mf.routing[(size_t) s]) continue;
            claimed[(size_t) s] = true;
            any = true;

            auto& dsp = generatorDSP[(size_t) s];
            const float mixL = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f - dsp.pan));
            const float mixR = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f + dsp.pan));
            masterFilterScratch.addFrom (0, 0, dsp.scratch, 0, 0, numSamples, mixL);
            masterFilterScratch.addFrom (1, 0, dsp.scratch, 1, 0, numSamples, mixR);
        }
        if (! any) continue;

        const float cutoff = apvts.getRawParameterValue (ParamIDs::masterFltCut  (f))->load();
        const float res    = apvts.getRawParameterValue (ParamIDs::masterFltRes  (f))->load();
        const int   type   = static_cast<int> (apvts.getRawParameterValue (ParamIDs::masterFltType (f))->load());

        using FType = juce::dsp::StateVariableTPTFilterType;
        auto& mfDsp = masterFilterDSP[(size_t) f];
        const bool isNotch = (type == 3);

        mfDsp.filter.setType (isNotch ? FType::lowpass
                                       : (type == 1 ? FType::highpass
                                                     : (type == 2 ? FType::bandpass : FType::lowpass)));
        mfDsp.filter.setCutoffFrequency (cutoff);
        mfDsp.filter.setResonance (res);

        if (isNotch)
        {
            mfDsp.notchHelper.setType (FType::highpass);
            mfDsp.notchHelper.setCutoffFrequency (cutoff);
            mfDsp.notchHelper.setResonance (res);
        }

        for (int ch = 0; ch < 2; ++ch)
        {
            float* data = masterFilterScratch.getWritePointer (ch);
            for (int n = 0; n < numSamples; ++n)
            {
                const float x = data[n];
                float v = mfDsp.filter.processSample (ch, x);
                if (isNotch) v += mfDsp.notchHelper.processSample (ch, x);
                data[n] = v;
            }
        }

        master.addFrom (0, 0, masterFilterScratch, 0, 0, numSamples);
        master.addFrom (1, 0, masterFilterScratch, 1, 0, numSamples);
    }

    // Generators not routed to any master filter mix straight through, unfiltered.
    for (int s = 0; s < numActiveGenerators; ++s)
    {
        if (claimed[(size_t) s]) continue;

        auto& dsp = generatorDSP[(size_t) s];
        const float mixL = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f - dsp.pan));
        const float mixR = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f + dsp.pan));
        master.addFrom (0, 0, dsp.scratch, 0, 0, numSamples, mixL);
        master.addFrom (1, 0, dsp.scratch, 1, 0, numSamples, mixR);
    }
}

void ViolentAudioProcessor::applyGeneratorFx (int s, GeneratorDSP& dsp, const GeneratorState& gen,
                                            juce::AudioBuffer<float>& buf)
{
    juce::dsp::AudioBlock<float> block (buf);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    for (int x = 0; x < gen.numFx; ++x)
    {
        auto& fxdsp = dsp.fxDSP[(size_t) x];
        const FxType type = gen.fxTypes[(size_t) x];

        switch (type)
        {
            case FxType::Distortion:
            {
                const float drive = apvts.getRawParameterValue (ParamIDs::genFxDrive    (s, x))->load();
                const float level = juce::Decibels::decibelsToGain (
                                        apvts.getRawParameterValue (ParamIDs::genFxLevel (s, x))->load());
                const float tone  = apvts.getRawParameterValue (ParamIDs::genFxTone     (s, x))->load();
                const int   dtype = static_cast<int> (
                                        apvts.getRawParameterValue (ParamIDs::genFxDistType (s, x))->load());

                *fxdsp.distToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                    processSpec.sampleRate, static_cast<double> (tone));

                for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                {
                    float* data = buf.getWritePointer (ch);
                    for (int n = 0; n < buf.getNumSamples(); ++n)
                    {
                        float x2 = data[n] * drive;
                        switch (dtype)
                        {
                            case 0: x2 = std::tanh (x2); break;
                            case 1: x2 = juce::jlimit (-1.0f, 1.0f, x2); break;
                            case 2:
                                x2 = std::tanh (x2 * 1.5f) + 0.1f * std::sin (x2 * juce::MathConstants<float>::pi);
                                x2 = juce::jlimit (-1.0f, 1.0f, x2);
                                break;
                            default: x2 = std::tanh (x2); break;
                        }
                        data[n] = x2 * level;
                    }
                }
                fxdsp.distToneFilter.process (ctx);
                break;
            }
            case FxType::Compressor:
                fxdsp.compressor.setThreshold (apvts.getRawParameterValue (ParamIDs::genFxThresh  (s, x))->load());
                fxdsp.compressor.setRatio     (apvts.getRawParameterValue (ParamIDs::genFxRatio   (s, x))->load());
                fxdsp.compressor.setAttack    (apvts.getRawParameterValue (ParamIDs::genFxAttack  (s, x))->load());
                fxdsp.compressor.setRelease   (apvts.getRawParameterValue (ParamIDs::genFxRelease (s, x))->load());
                fxdsp.makeup.setGainDecibels  (apvts.getRawParameterValue (ParamIDs::genFxMakeup  (s, x))->load());
                fxdsp.compressor.process (ctx);
                fxdsp.makeup.process (ctx);
                break;
            case FxType::Gate:
                fxdsp.gate.setThreshold (apvts.getRawParameterValue (ParamIDs::genFxThresh  (s, x))->load());
                fxdsp.gate.setRatio     (apvts.getRawParameterValue (ParamIDs::genFxRatio   (s, x))->load());
                fxdsp.gate.setAttack    (apvts.getRawParameterValue (ParamIDs::genFxAttack  (s, x))->load());
                fxdsp.gate.setRelease   (apvts.getRawParameterValue (ParamIDs::genFxRelease (s, x))->load());
                fxdsp.gate.process (ctx);
                break;
            case FxType::Reverb:
            {
                juce::dsp::Reverb::Parameters p;
                p.roomSize   = apvts.getRawParameterValue (ParamIDs::genFxRoom    (s, x))->load();
                p.damping    = apvts.getRawParameterValue (ParamIDs::genFxDamping (s, x))->load();
                p.wetLevel   = apvts.getRawParameterValue (ParamIDs::genFxWet     (s, x))->load();
                p.dryLevel   = 1.0f - p.wetLevel;
                p.width      = apvts.getRawParameterValue (ParamIDs::genFxWidth   (s, x))->load();
                p.freezeMode = 0.0f;
                fxdsp.reverb.setParameters (p);
                fxdsp.reverb.process (ctx);
                break;
            }
            case FxType::Filter:
            {
                const float cutoff = apvts.getRawParameterValue (ParamIDs::genFxFilterCut  (s, x))->load();
                const float res    = apvts.getRawParameterValue (ParamIDs::genFxFilterRes  (s, x))->load();
                const int   ftype  = static_cast<int> (
                                        apvts.getRawParameterValue (ParamIDs::genFxFilterType (s, x))->load());

                using FType = juce::dsp::StateVariableTPTFilterType;
                const bool isNotch = (ftype == 3);

                fxdsp.filter.setType (isNotch ? FType::lowpass
                                               : (ftype == 1 ? FType::highpass
                                                             : (ftype == 2 ? FType::bandpass : FType::lowpass)));
                fxdsp.filter.setCutoffFrequency (cutoff);
                fxdsp.filter.setResonance (res);

                if (isNotch)
                {
                    fxdsp.filterNotchHelper.setType (FType::highpass);
                    fxdsp.filterNotchHelper.setCutoffFrequency (cutoff);
                    fxdsp.filterNotchHelper.setResonance (res);
                }

                for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                {
                    float* data = buf.getWritePointer (ch);
                    for (int n = 0; n < buf.getNumSamples(); ++n)
                    {
                        const float x2 = data[n];
                        float v = fxdsp.filter.processSample (ch, x2);
                        if (isNotch) v += fxdsp.filterNotchHelper.processSample (ch, x2);
                        data[n] = v;
                    }
                }
                break;
            }
            default: break;
        }
    }
}

void ViolentAudioProcessor::processFxBuses (juce::AudioBuffer<float>& master, int numSamples)
{
    for (int b = 0; b < numFxBuses; ++b)
    {
        auto& buf   = fxBusScratch[(size_t) b];
        auto& fxdsp = fxBusDSP[(size_t) b];
        const FxType type = fxBusTypes[(size_t) b];

        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);

        switch (type)
        {
            case FxType::Distortion:
            {
                const float drive = apvts.getRawParameterValue (ParamIDs::busDrive    (b))->load();
                const float level = juce::Decibels::decibelsToGain (
                                        apvts.getRawParameterValue (ParamIDs::busLevel (b))->load());
                const float tone  = apvts.getRawParameterValue (ParamIDs::busTone     (b))->load();
                const int   dtype = static_cast<int> (
                                        apvts.getRawParameterValue (ParamIDs::busDistType (b))->load());

                *fxdsp.distToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                    processSpec.sampleRate, static_cast<double> (tone));

                for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                {
                    float* data = buf.getWritePointer (ch);
                    for (int n = 0; n < numSamples; ++n)
                    {
                        float x2 = data[n] * drive;
                        switch (dtype)
                        {
                            case 0: x2 = std::tanh (x2); break;
                            case 1: x2 = juce::jlimit (-1.0f, 1.0f, x2); break;
                            case 2:
                                x2 = std::tanh (x2 * 1.5f) + 0.1f * std::sin (x2 * juce::MathConstants<float>::pi);
                                x2 = juce::jlimit (-1.0f, 1.0f, x2);
                                break;
                            default: x2 = std::tanh (x2); break;
                        }
                        data[n] = x2 * level;
                    }
                }
                fxdsp.distToneFilter.process (ctx);
                break;
            }
            case FxType::Compressor:
                fxdsp.compressor.setThreshold (apvts.getRawParameterValue (ParamIDs::busThresh  (b))->load());
                fxdsp.compressor.setRatio     (apvts.getRawParameterValue (ParamIDs::busRatio   (b))->load());
                fxdsp.compressor.setAttack    (apvts.getRawParameterValue (ParamIDs::busAttack  (b))->load());
                fxdsp.compressor.setRelease   (apvts.getRawParameterValue (ParamIDs::busRelease (b))->load());
                fxdsp.makeup.setGainDecibels  (apvts.getRawParameterValue (ParamIDs::busMakeup  (b))->load());
                fxdsp.compressor.process (ctx);
                fxdsp.makeup.process (ctx);
                break;
            case FxType::Gate:
                fxdsp.gate.setThreshold (apvts.getRawParameterValue (ParamIDs::busThresh  (b))->load());
                fxdsp.gate.setRatio     (apvts.getRawParameterValue (ParamIDs::busRatio   (b))->load());
                fxdsp.gate.setAttack    (apvts.getRawParameterValue (ParamIDs::busAttack  (b))->load());
                fxdsp.gate.setRelease   (apvts.getRawParameterValue (ParamIDs::busRelease (b))->load());
                fxdsp.gate.process (ctx);
                break;
            case FxType::Reverb:
            {
                juce::dsp::Reverb::Parameters p;
                p.roomSize   = apvts.getRawParameterValue (ParamIDs::busRoom    (b))->load();
                p.damping    = apvts.getRawParameterValue (ParamIDs::busDamping (b))->load();
                p.wetLevel   = apvts.getRawParameterValue (ParamIDs::busWet     (b))->load();
                p.dryLevel   = 1.0f - p.wetLevel;
                p.width      = apvts.getRawParameterValue (ParamIDs::busWidth   (b))->load();
                p.freezeMode = 0.0f;
                fxdsp.reverb.setParameters (p);
                fxdsp.reverb.process (ctx);
                break;
            }
            case FxType::Filter:
            {
                const float cutoff = apvts.getRawParameterValue (ParamIDs::busFilterCut  (b))->load();
                const float res    = apvts.getRawParameterValue (ParamIDs::busFilterRes  (b))->load();
                const int   ftype  = static_cast<int> (
                                        apvts.getRawParameterValue (ParamIDs::busFilterType (b))->load());

                using FType = juce::dsp::StateVariableTPTFilterType;
                const bool isNotch = (ftype == 3);

                fxdsp.filter.setType (isNotch ? FType::lowpass
                                               : (ftype == 1 ? FType::highpass
                                                             : (ftype == 2 ? FType::bandpass : FType::lowpass)));
                fxdsp.filter.setCutoffFrequency (cutoff);
                fxdsp.filter.setResonance (res);

                if (isNotch)
                {
                    fxdsp.filterNotchHelper.setType (FType::highpass);
                    fxdsp.filterNotchHelper.setCutoffFrequency (cutoff);
                    fxdsp.filterNotchHelper.setResonance (res);
                }

                for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                {
                    float* data = buf.getWritePointer (ch);
                    for (int n = 0; n < numSamples; ++n)
                    {
                        const float x2 = data[n];
                        float v = fxdsp.filter.processSample (ch, x2);
                        if (isNotch) v += fxdsp.filterNotchHelper.processSample (ch, x2);
                        data[n] = v;
                    }
                }
                break;
            }
            default: break;
        }

        master.addFrom (0, 0, buf, 0, 0, numSamples);
        master.addFrom (1, 0, buf, 1, 0, numSamples);
    }
}

//==============================================================================
void ViolentAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    if (paramsDirty.exchange (false))
        updateEQ();

    buffer.clear();

    const int numSamples = buffer.getNumSamples();
    for (int b = 0; b < numFxBuses; ++b)
    {
        fxBusScratch[(size_t) b].setSize (2, numSamples, false, false, true);
        fxBusScratch[(size_t) b].clear();
    }

    // Load params for all active generators
    for (int s = 0; s < numActiveGenerators; ++s)
        loadGeneratorParams (s);

    // MIDI
    renderPreviewMidi (midiMessages, buffer.getNumSamples());
    processMidi (midiMessages);
    renderMidiModifiers (buffer.getNumSamples());

    // Render each generator into its own scratch buffer (also feeding sends
    // into any shared FX buses), then mix — master filters (applied last)
    // decide how each generator gets summed in.
    for (int s = 0; s < numActiveGenerators; ++s)
        renderGenerator (s, buffer);
    mixGeneratorsToMaster (buffer);
    processFxBuses (buffer, numSamples);

    // Global EQ
    if (eqEnabled)
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        for (auto& band : eqBands) band.process (ctx);
    }

    // Level metering
    levelL.store (buffer.getMagnitude (0, 0, buffer.getNumSamples()), std::memory_order_relaxed);
    levelR.store (buffer.getNumChannels() > 1
                      ? buffer.getMagnitude (1, 0, buffer.getNumSamples())
                      : levelL.load (std::memory_order_relaxed),
                  std::memory_order_relaxed);
}

//==============================================================================
void ViolentAudioProcessor::parameterChanged (const juce::String&, float)
{
    paramsDirty = true;
}

void ViolentAudioProcessor::setEQBand (int i, float gainDB)
{
    eqBands[(size_t) i].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
        processSpec.sampleRate,
        static_cast<double> (EQ_FREQUENCIES[i]),
        static_cast<double> (EQ_Q),
        juce::Decibels::decibelsToGain (gainDB));
}

void ViolentAudioProcessor::updateEQ()
{
    if (!prepared) return;
    eqEnabled = apvts.getRawParameterValue (ParamIDs::EQ_ENABLED)->load() > 0.5f;
    for (int i = 0; i < NUM_EQ_BANDS; ++i)
        setEQBand (i, apvts.getRawParameterValue (ParamIDs::eqBand (i))->load());
}

//==============================================================================
juce::AudioProcessorEditor* ViolentAudioProcessor::createEditor()
{
    return new ViolentAudioProcessorEditor (*this);
}

std::unique_ptr<juce::XmlElement> ViolentAudioProcessor::createStateXml()
{
    auto xml = apvts.copyState().createXml();
    if (xml == nullptr) return nullptr;

    xml->setAttribute ("numActiveGenerators", numActiveGenerators);

    for (int s = 0; s < MAX_GENERATORS; ++s)
    {
        const auto& gen = generators[(size_t) s];
        xml->setAttribute ("gen_" + juce::String(s) + "_numFx",      gen.numFx);
        for (int x = 0; x < MAX_GENERATOR_FX; ++x)
            xml->setAttribute ("gen_" + juce::String(s) + "_fx" + juce::String(x) + "_fxtype",
                               static_cast<int> (gen.fxTypes[(size_t) x]));

        // Sample path for sample-mode generators
        juce::SpinLock::ScopedTryLockType tryLock (sampleModules[(size_t) s].lock);
        if (tryLock.isLocked() && sampleModules[(size_t) s].hasData)
            xml->setAttribute ("gen_" + juce::String(s) + "_samplepath",
                               sampleModules[(size_t) s].filePath);
    }

    xml->setAttribute ("numMasterFilters", numMasterFilters);
    for (int f = 0; f < MAX_MASTER_FILTERS; ++f)
    {
        const auto& mf = masterFilters[(size_t) f];
        xml->setAttribute ("mflt_" + juce::String(f) + "_en", mf.enabled);
        juce::String routingStr;
        for (bool r : mf.routing) routingStr += r ? "1" : "0";
        xml->setAttribute ("mflt_" + juce::String(f) + "_routing", routingStr);
    }

    xml->setAttribute ("numFxBuses", numFxBuses);
    for (int b = 0; b < MAX_FX_BUSES; ++b)
        xml->setAttribute ("bus_" + juce::String(b) + "_fxtype", static_cast<int> (fxBusTypes[(size_t) b]));

    return xml;
}

void ViolentAudioProcessor::restoreStateFromXml (const juce::XmlElement& xml)
{
    if (xml.hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (xml));

    numActiveGenerators = juce::jlimit (1, MAX_GENERATORS,
                                     xml.getIntAttribute ("numActiveGenerators", 1));

    for (int s = 0; s < MAX_GENERATORS; ++s)
    {
        auto& gen    = generators[(size_t) s];
        gen.numFx      = juce::jlimit (0, MAX_GENERATOR_FX,
                                      xml.getIntAttribute ("gen_" + juce::String(s) + "_numFx", 0));
        for (int x = 0; x < MAX_GENERATOR_FX; ++x)
            gen.fxTypes[(size_t) x] = static_cast<FxType> (
                juce::jlimit (0, NUM_FX_TYPES - 1,
                              xml.getIntAttribute ("gen_" + juce::String(s) + "_fx" + juce::String(x) + "_fxtype", 0)));

        const juce::String path = xml.getStringAttribute ("gen_" + juce::String(s) + "_samplepath");
        if (path.isNotEmpty())
        {
            const juce::File f (path);
            if (f.existsAsFile()) loadSample (s, f);
        }
    }

    numMasterFilters = juce::jlimit (0, MAX_MASTER_FILTERS,
                                      xml.getIntAttribute ("numMasterFilters", 0));
    for (int f = 0; f < MAX_MASTER_FILTERS; ++f)
    {
        auto& mf = masterFilters[(size_t) f];
        mf.enabled = xml.getBoolAttribute ("mflt_" + juce::String(f) + "_en", true);
        const juce::String routingStr = xml.getStringAttribute ("mflt_" + juce::String(f) + "_routing");
        for (int s = 0; s < MAX_GENERATORS; ++s)
            mf.routing[(size_t) s] = (s < routingStr.length() && routingStr[s] == '1');
    }

    numFxBuses = juce::jlimit (0, MAX_FX_BUSES, xml.getIntAttribute ("numFxBuses", 0));
    for (int b = 0; b < MAX_FX_BUSES; ++b)
        fxBusTypes[(size_t) b] = static_cast<FxType> (
            juce::jlimit (0, NUM_FX_TYPES - 1, xml.getIntAttribute ("bus_" + juce::String(b) + "_fxtype", 0)));
}

void ViolentAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = createStateXml())
        copyXmlToBinary (*xml, destData);
}

void ViolentAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        restoreStateFromXml (*xml);
}

//==============================================================================
juce::File ViolentAudioProcessor::getPresetsDirectory()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("Application Support").getChildFile ("Violent Audio")
                   .getChildFile ("Violent").getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}

juce::StringArray ViolentAudioProcessor::getPresetNames() const
{
    juce::StringArray names;
    for (const auto& f : getPresetsDirectory().findChildFiles (juce::File::findFiles, false, "*.violentpreset"))
        names.add (f.getFileNameWithoutExtension());
    names.sort (true);
    return names;
}

bool ViolentAudioProcessor::savePreset (const juce::String& name)
{
    auto xml = createStateXml();
    if (xml == nullptr) return false;

    const auto file = getPresetsDirectory().getChildFile (juce::File::createLegalFileName (name) + ".violentpreset");
    if (! xml->writeTo (file)) return false;

    currentPresetName = name;
    return true;
}

bool ViolentAudioProcessor::loadPreset (const juce::String& name)
{
    const auto file = getPresetsDirectory().getChildFile (juce::File::createLegalFileName (name) + ".violentpreset");
    if (! file.existsAsFile()) return false;

    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr) return false;

    restoreStateFromXml (*xml);
    currentPresetName = name;
    return true;
}

void ViolentAudioProcessor::loadSample (int slotIndex, const juce::File& file)
{
    auto reader = std::unique_ptr<juce::AudioFormatReader> (
        formatManager.createReaderFor (file));
    if (reader == nullptr) return;

    juce::AudioBuffer<float> tmp (static_cast<int> (reader->numChannels),
                                   static_cast<int> (reader->lengthInSamples));
    reader->read (&tmp, 0, static_cast<int> (reader->lengthInSamples), 0, true, true);

    {
        juce::SpinLock::ScopedLockType lock (sampleModules[(size_t) slotIndex].lock);
        auto& sm    = sampleModules[(size_t) slotIndex];
        sm.buffer   = std::move (tmp);
        sm.filePath = file.getFullPathName();
        sm.hasData  = true;
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViolentAudioProcessor();
}

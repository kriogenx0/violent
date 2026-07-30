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
    }

    // ---- MIDI Modifier Components (shared pool, routed to generators) ----
    for (int m = 0; m < MAX_MIDI_MODIFIERS; ++m)
    {
        const String mn = " MIDI Modifier " + String (m + 1) + " ";
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::midiModTranspose (m), mn + "Transpose", -24, 24, 0));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::midiModOctave (m),    mn + "Octave", -3, 3, 0));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::midiModKeyRoot (m), mn + "Key Root",
            StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::midiModKeyScale (m), mn + "Key Scale",
            StringArray { "Major", "Minor" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::midiModArpRate (m), mn + "Arp Rate",
            NormalisableRange<float> (0.05f, 1.0f, 0.001f, 0.5f), 0.15f,
            AudioParameterFloatAttributes().withLabel ("s")));
    }

    // ---- Effect Components (shared pool, routed to generators; Filter is
    // just one selectable type here, not a separate concept) ----
    for (int x = 0; x < MAX_EFFECTS; ++x)
    {
        const String xn = " Effect " + String (x + 1) + " ";
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectDrive    (x), xn + "Drive",
            NormalisableRange<float> (1.0f, 100.0f, 0.1f, 0.4f), 1.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectTone     (x), xn + "Tone",
            NormalisableRange<float> (200.0f, 8000.0f, 1.0f, 0.5f), 4000.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectLevel    (x), xn + "Level",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::effectDistType (x), xn + "Dist Type",
            StringArray { "Soft Clip", "Hard Clip", "Fuzz" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectThresh   (x), xn + "Threshold",
            NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -12.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectRatio    (x), xn + "Ratio",
            NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.5f), 4.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectAttack   (x), xn + "Attack",
            NormalisableRange<float> (0.1f, 200.0f, 0.1f, 0.5f), 10.0f,
            AudioParameterFloatAttributes().withLabel ("ms")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectRelease  (x), xn + "Release",
            NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.5f), 100.0f,
            AudioParameterFloatAttributes().withLabel ("ms")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectMakeup   (x), xn + "Makeup",
            NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectRoom     (x), xn + "Room",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectDamping  (x), xn + "Damping",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectWet      (x), xn + "Wet",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.33f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectWidth    (x), xn + "Width",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::effectFilterType (x), xn + "Filter Type",
            StringArray { "LP", "HP", "BP", "Notch" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectFilterCut (x), xn + "Filter Cutoff",
            NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 8000.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::effectFilterRes (x), xn + "Filter Resonance",
            NormalisableRange<float> (0.1f, 12.0f, 0.01f, 0.5f), 0.707f));
    }

    // ---- Modulator Components (shared pool; Envelope or LFO source
    // targeting any one modulatable parameter) ----
    for (int m = 0; m < MAX_MODULATORS; ++m)
    {
        const String mn = " Modulator " + String (m + 1) + " ";
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::modAmount (m), mn + "Amount",
            NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::modLfoRate (m), mn + "LFO Rate",
            NormalisableRange<float> (0.02f, 20.0f, 0.01f, 0.3f), 1.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::modLfoShape (m), mn + "LFO Shape",
            StringArray { "Sine", "Triangle", "Square", "Saw" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::modEnvAtt (m), mn + "Env Attack",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.01f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::modEnvDec (m), mn + "Env Decay",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.1f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::modEnvSus (m), mn + "Env Sustain",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.7f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::modEnvRel (m), mn + "Env Release",
            NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.3f), 0.3f,
            AudioParameterFloatAttributes().withLabel ("s")));
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
    sampler.prepare();
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

    synthEngine.prepare (sampleRate);

    for (auto& dsp : generatorDSP)
        dsp.prepare (processSpec);

    for (auto& band : eqBands)
        band.prepare (processSpec);
    setEQBand (0, 0.0f);   // init state

    for (auto& fx : effectDSP) fx.prepare (processSpec);
    effectScratch.setSize (2, samplesPerBlock);

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
    synthEngine.allNotesOff();
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
    o.gainLin     = juce::Decibels::decibelsToGain (getModulatedValue (ParamIDs::genSrcGain (s)));
    o.octave      = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcOct  (s))->load());
    o.semitone    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcSemi (s))->load());
    o.detune      = getModulatedValue (ParamIDs::genSrcDet       (s));
    o.phase       = getModulatedValue (ParamIDs::genSrcPhase     (s));
    o.pulseWidth  = getModulatedValue (ParamIDs::genSrcPW        (s));
    o.pan         = getModulatedValue (ParamIDs::genSrcPan       (s));
    o.velSens     = getModulatedValue (ParamIDs::genSrcVel       (s));
    o.unisonVoices= static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcUni (s))->load());
    o.unisonSpread= getModulatedValue (ParamIDs::genSrcUniSpread (s));
    o.att         = getModulatedValue (ParamIDs::genSrcAtt (s));
    o.dec         = getModulatedValue (ParamIDs::genSrcDec (s));
    o.sus         = getModulatedValue (ParamIDs::genSrcSus (s));
    o.rel         = getModulatedValue (ParamIDs::genSrcRel (s));
}

//==============================================================================
void ViolentAudioProcessor::processMidi (const juce::MidiBuffer& midi)
{
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
        {
            if (heldNoteCount++ == 0)
                triggerModulatorEnvelopes (true);

            const int   rawNote = msg.getNoteNumber();
            const float vel     = msg.getFloatVelocity();
            for (int s = 0; s < numActiveGenerators; ++s)
            {
                if (!generators[(size_t) s].enabled) continue;
                const int note = applyMidiModifier (s, rawNote);

                // If any Arp modifier routes to this generator, the note
                // feeds their held-note sets instead of starting a voice
                // directly — the arp sequencer starts voices itself later.
                bool anyArp = false;
                for (int m = 0; m < numMidiModifiers; ++m)
                {
                    const auto& mm = midiModifiers[(size_t) m];
                    if (mm.enabled && mm.type == MidiModType::Arp && mm.routing[(size_t) s])
                    {
                        midiModState[(size_t) m][(size_t) s].heldNotes.push_back (note);
                        anyArp = true;
                    }
                }
                if (! anyArp)
                {
                    int vi = synthEngine.findFreeVoice (s);
                    if (vi < 0) vi = synthEngine.findVoiceToSteal (s);
                    if (vi >= 0) synthEngine.startVoice (vi, note, vel, s, generatorDSP[(size_t) s].osc);
                }
            }
        }
        else if (msg.isNoteOff())
        {
            if (heldNoteCount > 0 && --heldNoteCount == 0)
                triggerModulatorEnvelopes (false);

            const int rawNote = msg.getNoteNumber();
            for (int s = 0; s < numActiveGenerators; ++s)
            {
                const int note = applyMidiModifier (s, rawNote);

                bool anyArp = false;
                for (int m = 0; m < numMidiModifiers; ++m)
                {
                    const auto& mm = midiModifiers[(size_t) m];
                    if (mm.enabled && mm.type == MidiModType::Arp && mm.routing[(size_t) s])
                    {
                        auto& held = midiModState[(size_t) m][(size_t) s].heldNotes;
                        auto it = std::find (held.begin(), held.end(), note);
                        if (it != held.end()) held.erase (it);
                        anyArp = true;
                    }
                }
                if (! anyArp)
                    synthEngine.stopNoteForGenerator (s, note);
            }
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            synthEngine.allNotesOff();
            for (auto& perModifier : midiModState)
                for (auto& st : perModifier)
                    st.heldNotes.clear();
            if (heldNoteCount != 0)
            {
                heldNoteCount = 0;
                triggerModulatorEnvelopes (false);
            }
        }
    }
}

int ViolentAudioProcessor::applyMidiModifier (int s, int note) const
{
    int result = note;
    for (int m = 0; m < numMidiModifiers; ++m)
    {
        const auto& mm = midiModifiers[(size_t) m];
        if (mm.enabled && mm.routing[(size_t) s])
            result = MidiModifier::applyOne (apvts, m, mm.type, result);
    }
    return juce::jlimit (0, 127, result);
}

bool ViolentAudioProcessor::isArpEnabled (int s) const
{
    for (int m = 0; m < numMidiModifiers; ++m)
    {
        const auto& mm = midiModifiers[(size_t) m];
        if (mm.enabled && mm.type == MidiModType::Arp && mm.routing[(size_t) s])
            return true;
    }
    return false;
}

//==============================================================================
float ViolentAudioProcessor::getModulatedValue (const juce::String& parameterID) const noexcept
{
    auto* raw = apvts.getRawParameterValue (parameterID);
    if (raw == nullptr) return 0.0f;
    const float base = raw->load();

    float offset = 0.0f;
    for (int m = 0; m < numModulators; ++m)
    {
        const auto& mod = modulators[(size_t) m];
        if (! mod.enabled || mod.targetParamID != parameterID) continue;
        const float amount = apvts.getRawParameterValue (ParamIDs::modAmount (m))->load();
        offset += modulatorOutputs[(size_t) m] * amount;
    }
    if (offset == 0.0f) return base;

    if (auto* param = apvts.getParameter (parameterID))
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
        {
            const auto range = ranged->getNormalisableRange();
            const float span = range.end - range.start;
            return juce::jlimit (range.start, range.end, base + offset * span * 0.5f);
        }
    return base;
}

std::vector<std::pair<juce::String, juce::String>> ViolentAudioProcessor::getModulatableParameters() const
{
    std::vector<std::pair<juce::String, juce::String>> result;
    for (auto* p : getParameters())
    {
        auto* floatParam = dynamic_cast<juce::AudioParameterFloat*> (p);
        auto* withID = dynamic_cast<juce::AudioProcessorParameterWithID*> (p);
        if (floatParam == nullptr || withID == nullptr) continue;

        const auto& id = withID->paramID;
        const bool isGeneratorSource = id.startsWith ("gen_") && id.contains ("_src_");
        const bool isEffect = id.startsWith ("fx");
        if (! isGeneratorSource && ! isEffect) continue;

        result.push_back ({ id, p->getName (256).trim() });
    }
    return result;
}

//==============================================================================
void ViolentAudioProcessor::triggerModulatorEnvelopes (bool noteOn) noexcept
{
    for (int m = 0; m < numModulators; ++m)
    {
        auto& mod = modulators[(size_t) m];
        if (! mod.enabled || mod.sourceType != ModulatorSourceType::Envelope) continue;
        modulatorState[(size_t) m].envStage = noteOn
            ? ModulatorState::EnvStage::Attack
            : ModulatorState::EnvStage::Release;
    }
}

void ViolentAudioProcessor::renderModulators (int numSamples)
{
    const float dt = (float) numSamples / (float) juce::jmax (1.0, processSpec.sampleRate);

    for (int m = 0; m < numModulators; ++m)
    {
        auto& mod = modulators[(size_t) m];
        auto& st  = modulatorState[(size_t) m];
        if (! mod.enabled)
        {
            modulatorOutputs[(size_t) m] = 0.0f;
            continue;
        }

        if (mod.sourceType == ModulatorSourceType::LFO)
        {
            const float rate = apvts.getRawParameterValue (ParamIDs::modLfoRate (m))->load();
            const auto shape = static_cast<LfoShape> (
                (int) apvts.getRawParameterValue (ParamIDs::modLfoShape (m))->load());

            st.lfoPhase += rate * dt;
            st.lfoPhase -= std::floor (st.lfoPhase);

            float v;
            switch (shape)
            {
                case LfoShape::Triangle: v = 4.0f * std::abs (st.lfoPhase - 0.5f) - 1.0f; break;
                case LfoShape::Square:   v = st.lfoPhase < 0.5f ? 1.0f : -1.0f; break;
                case LfoShape::Saw:      v = 2.0f * st.lfoPhase - 1.0f; break;
                default:                 v = std::sin (st.lfoPhase * juce::MathConstants<float>::twoPi); break;
            }
            modulatorOutputs[(size_t) m] = v;
        }
        else // Envelope
        {
            const float att = apvts.getRawParameterValue (ParamIDs::modEnvAtt (m))->load();
            const float dec = apvts.getRawParameterValue (ParamIDs::modEnvDec (m))->load();
            const float sus = apvts.getRawParameterValue (ParamIDs::modEnvSus (m))->load();
            const float rel = apvts.getRawParameterValue (ParamIDs::modEnvRel (m))->load();

            switch (st.envStage)
            {
                case ModulatorState::EnvStage::Attack:
                    st.envLevel += dt / juce::jmax (0.001f, att);
                    if (st.envLevel >= 1.0f) { st.envLevel = 1.0f; st.envStage = ModulatorState::EnvStage::Decay; }
                    break;
                case ModulatorState::EnvStage::Decay:
                    st.envLevel -= dt * (1.0f - sus) / juce::jmax (0.001f, dec);
                    if (st.envLevel <= sus) { st.envLevel = sus; st.envStage = ModulatorState::EnvStage::Sustain; }
                    break;
                case ModulatorState::EnvStage::Sustain:
                    st.envLevel = sus;
                    break;
                case ModulatorState::EnvStage::Release:
                    st.envLevel -= dt / juce::jmax (0.001f, rel);
                    if (st.envLevel <= 0.0f) { st.envLevel = 0.0f; st.envStage = ModulatorState::EnvStage::Idle; }
                    break;
                default:
                    st.envLevel = 0.0f;
                    break;
            }
            modulatorOutputs[(size_t) m] = st.envLevel;
        }
    }
}

void ViolentAudioProcessor::renderMidiModifiers (int numSamples)
{
    for (int m = 0; m < MAX_MIDI_MODIFIERS; ++m)
    {
        const bool isArpSlot = (m < numMidiModifiers) && midiModifiers[(size_t) m].enabled
                             && (midiModifiers[(size_t) m].type == MidiModType::Arp);

        for (int s = 0; s < numActiveGenerators; ++s)
        {
            auto& st = midiModState[(size_t) m][(size_t) s];
            const bool routed = isArpSlot && midiModifiers[(size_t) m].routing[(size_t) s];

            if (! routed)
            {
                if (st.arpNoteIsOn)
                {
                    synthEngine.stopNoteForGenerator (s, st.arpCurrentNote);
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
                    synthEngine.stopNoteForGenerator (s, st.arpCurrentNote);
                    st.arpNoteIsOn = false;
                }
                st.arpStepIndex = -1;
                st.arpSamplePos = st.arpNextEventSample = 0;
                continue;
            }

            const float rateSeconds  = apvts.getRawParameterValue (ParamIDs::midiModArpRate (m))->load();
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
                    synthEngine.stopNoteForGenerator (s, st.arpCurrentNote);
                    st.arpNoteIsOn        = false;
                    st.arpNextEventSample = st.arpSamplePos + (samplesPerStep - gateSamples);
                }
                else
                {
                    st.arpStepIndex = (st.arpStepIndex + 1) % (int) st.heldNotes.size();
                    const int note  = st.heldNotes[(size_t) st.arpStepIndex];

                    int vi = synthEngine.findFreeVoice (s);
                    if (vi < 0) vi = synthEngine.findVoiceToSteal (s);
                    if (vi >= 0) synthEngine.startVoice (vi, note, 1.0f, s, generatorDSP[(size_t) s].osc);

                    st.arpCurrentNote     = note;
                    st.arpNoteIsOn        = true;
                    st.arpNextEventSample = st.arpSamplePos + gateSamples;
                }
            }
        }
    }
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
    for (auto& v : synthEngine.voices)
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
                    // (generator index maps to sample slot index). Reuses this
                    // unison voice's oscillator phase as a fractional sample
                    // position (see Sampler::tick).
                    juce::SpinLock::ScopedTryLockType tryLock (sampler.modules[(size_t) s].lock);
                    if (tryLock.isLocked() && sampler.modules[(size_t) s].hasData)
                        sample = Sampler::tick (sampler.modules[(size_t) s].buffer, v.osc[u].phase, v.note);
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

    generatorLevelMeter[(size_t) s].store (dsp.scratch.getMagnitude (0, 0, numSamples), std::memory_order_relaxed);

    // Mixing into the master bus, and all effects processing, happens
    // afterwards in processEffects() — that's where each Effect Component's
    // routing decides how each generator gets summed in.
}

void ViolentAudioProcessor::processEffects (juce::AudioBuffer<float>& master, int numSamples)
{
    std::array<bool, MAX_GENERATORS> claimed {};

    for (int x = 0; x < numEffects; ++x)
    {
        auto& fx = effects[(size_t) x];
        if (! fx.enabled) continue;

        effectScratch.setSize (2, numSamples, false, false, true);
        effectScratch.clear();

        bool any = false;
        for (int s = 0; s < numActiveGenerators; ++s)
        {
            if (! fx.routing[(size_t) s]) continue;
            claimed[(size_t) s] = true;
            any = true;

            auto& dsp = generatorDSP[(size_t) s];
            const float mixL = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f - dsp.pan));
            const float mixR = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f + dsp.pan));
            effectScratch.addFrom (0, 0, dsp.scratch, 0, 0, numSamples, mixL);
            effectScratch.addFrom (1, 0, dsp.scratch, 1, 0, numSamples, mixR);
        }
        if (! any) continue;

        auto& fxdsp = effectDSP[(size_t) x];
        juce::dsp::AudioBlock<float> block (effectScratch);
        juce::dsp::ProcessContextReplacing<float> ctx (block);

        switch (fx.type)
        {
            case FxType::Distortion:
            {
                const float drive = getModulatedValue (ParamIDs::effectDrive (x));
                const float level = juce::Decibels::decibelsToGain (getModulatedValue (ParamIDs::effectLevel (x)));
                const float tone  = getModulatedValue (ParamIDs::effectTone (x));
                const int   dtype = static_cast<int> (
                                        apvts.getRawParameterValue (ParamIDs::effectDistType (x))->load());

                *fxdsp.distToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                    processSpec.sampleRate, static_cast<double> (tone));

                for (int ch = 0; ch < effectScratch.getNumChannels(); ++ch)
                {
                    float* data = effectScratch.getWritePointer (ch);
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
                fxdsp.compressor.setThreshold (getModulatedValue (ParamIDs::effectThresh  (x)));
                fxdsp.compressor.setRatio     (getModulatedValue (ParamIDs::effectRatio   (x)));
                fxdsp.compressor.setAttack    (getModulatedValue (ParamIDs::effectAttack  (x)));
                fxdsp.compressor.setRelease   (getModulatedValue (ParamIDs::effectRelease (x)));
                fxdsp.makeup.setGainDecibels  (getModulatedValue (ParamIDs::effectMakeup  (x)));
                fxdsp.compressor.process (ctx);
                fxdsp.makeup.process (ctx);
                break;
            case FxType::Gate:
                fxdsp.gate.setThreshold (getModulatedValue (ParamIDs::effectThresh  (x)));
                fxdsp.gate.setRatio     (getModulatedValue (ParamIDs::effectRatio   (x)));
                fxdsp.gate.setAttack    (getModulatedValue (ParamIDs::effectAttack  (x)));
                fxdsp.gate.setRelease   (getModulatedValue (ParamIDs::effectRelease (x)));
                fxdsp.gate.process (ctx);
                break;
            case FxType::Reverb:
            {
                juce::dsp::Reverb::Parameters p;
                p.roomSize   = getModulatedValue (ParamIDs::effectRoom    (x));
                p.damping    = getModulatedValue (ParamIDs::effectDamping (x));
                p.wetLevel   = getModulatedValue (ParamIDs::effectWet     (x));
                p.dryLevel   = 1.0f - p.wetLevel;
                p.width      = getModulatedValue (ParamIDs::effectWidth   (x));
                p.freezeMode = 0.0f;
                fxdsp.reverb.setParameters (p);
                fxdsp.reverb.process (ctx);
                break;
            }
            case FxType::Filter:
            {
                const float cutoff = getModulatedValue (ParamIDs::effectFilterCut (x));
                const float res    = getModulatedValue (ParamIDs::effectFilterRes (x));
                const int   ftype  = static_cast<int> (
                                        apvts.getRawParameterValue (ParamIDs::effectFilterType (x))->load());
                fxdsp.filter.setParams (ftype, cutoff, res);

                for (int ch = 0; ch < effectScratch.getNumChannels(); ++ch)
                {
                    float* data = effectScratch.getWritePointer (ch);
                    for (int n = 0; n < numSamples; ++n)
                        data[n] = fxdsp.filter.processSample (ch, data[n]);
                }
                break;
            }
            default: break;
        }

        master.addFrom (0, 0, effectScratch, 0, 0, numSamples);
        master.addFrom (1, 0, effectScratch, 1, 0, numSamples);
    }

    // Generators not routed to any effect mix straight through, unprocessed.
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

//==============================================================================
void ViolentAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    if (paramsDirty.exchange (false))
        updateEQ();

    buffer.clear();

    const int numSamples = buffer.getNumSamples();

    // Modulators run first so getModulatedValue() sees this block's values
    // when generator/effect params are read below.
    renderModulators (numSamples);

    // Load params for all active generators
    for (int s = 0; s < numActiveGenerators; ++s)
        loadGeneratorParams (s);

    // MIDI
    renderPreviewMidi (midiMessages, buffer.getNumSamples());
    processMidi (midiMessages);
    renderMidiModifiers (buffer.getNumSamples());

    // Render each generator into its own scratch buffer, then mix — each
    // Effect Component's routing decides how each generator gets summed in.
    for (int s = 0; s < numActiveGenerators; ++s)
        renderGenerator (s, buffer);
    processEffects (buffer, numSamples);

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

    // Spectrum analyzer
    const bool stereo = buffer.getNumChannels() > 1;
    for (int i = 0; i < numSamples; ++i)
    {
        const float mono = stereo
            ? 0.5f * (buffer.getSample (0, i) + buffer.getSample (1, i))
            : buffer.getSample (0, i);
        pushSpectrumSample (mono);
    }
}

void ViolentAudioProcessor::pushSpectrumSample (float sample) noexcept
{
    spectrumFifo[(size_t) spectrumFifoIndex++] = sample;
    if (spectrumFifoIndex < SPECTRUM_FFT_SIZE)
        return;
    spectrumFifoIndex = 0;

    std::copy (spectrumFifo.begin(), spectrumFifo.end(), spectrumFftData.begin());
    std::fill (spectrumFftData.begin() + SPECTRUM_FFT_SIZE, spectrumFftData.end(), 0.0f);
    spectrumWindow.multiplyWithWindowingTable (spectrumFftData.data(), (size_t) SPECTRUM_FFT_SIZE);
    spectrumFFT.performFrequencyOnlyForwardTransform (spectrumFftData.data());

    for (int i = 0; i < SPECTRUM_FFT_SIZE / 2; ++i)
    {
        const float mag = spectrumFftData[(size_t) i] / (float) SPECTRUM_FFT_SIZE;
        spectrumMagnitudesDb[(size_t) i] = juce::Decibels::gainToDecibels (mag, -100.0f);
    }
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
        xml->setAttribute ("gen_" + juce::String(s) + "_name", gen.name);

        // Sample path for sample-mode generators
        juce::SpinLock::ScopedTryLockType tryLock (sampler.modules[(size_t) s].lock);
        if (tryLock.isLocked() && sampler.modules[(size_t) s].hasData)
            xml->setAttribute ("gen_" + juce::String(s) + "_samplepath",
                               sampler.modules[(size_t) s].filePath);
    }

    xml->setAttribute ("numMidiModifiers", numMidiModifiers);
    for (int m = 0; m < MAX_MIDI_MODIFIERS; ++m)
    {
        const auto& mm = midiModifiers[(size_t) m];
        xml->setAttribute ("mm" + juce::String(m) + "_name", mm.name);
        xml->setAttribute ("mm" + juce::String(m) + "_type", static_cast<int> (mm.type));
        xml->setAttribute ("mm" + juce::String(m) + "_en",   mm.enabled);
        juce::String routingStr;
        for (bool r : mm.routing) routingStr += r ? "1" : "0";
        xml->setAttribute ("mm" + juce::String(m) + "_routing", routingStr);
    }

    xml->setAttribute ("numEffects", numEffects);
    for (int x = 0; x < MAX_EFFECTS; ++x)
    {
        const auto& fx = effects[(size_t) x];
        xml->setAttribute ("fx" + juce::String(x) + "_name", fx.name);
        xml->setAttribute ("fx" + juce::String(x) + "_type", static_cast<int> (fx.type));
        xml->setAttribute ("fx" + juce::String(x) + "_en",   fx.enabled);
        juce::String routingStr;
        for (bool r : fx.routing) routingStr += r ? "1" : "0";
        xml->setAttribute ("fx" + juce::String(x) + "_routing", routingStr);
    }

    xml->setAttribute ("numModulators", numModulators);
    for (int m = 0; m < MAX_MODULATORS; ++m)
    {
        const auto& mod = modulators[(size_t) m];
        xml->setAttribute ("mod" + juce::String(m) + "_name",   mod.name);
        xml->setAttribute ("mod" + juce::String(m) + "_source", static_cast<int> (mod.sourceType));
        xml->setAttribute ("mod" + juce::String(m) + "_en",     mod.enabled);
        xml->setAttribute ("mod" + juce::String(m) + "_target", mod.targetParamID);
    }

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
        auto& gen = generators[(size_t) s];
        gen.name = xml.getStringAttribute ("gen_" + juce::String(s) + "_name");

        const juce::String path = xml.getStringAttribute ("gen_" + juce::String(s) + "_samplepath");
        if (path.isNotEmpty())
        {
            const juce::File f (path);
            if (f.existsAsFile()) loadSample (s, f);
        }
    }

    numMidiModifiers = juce::jlimit (0, MAX_MIDI_MODIFIERS,
                                      xml.getIntAttribute ("numMidiModifiers", 0));
    for (int m = 0; m < MAX_MIDI_MODIFIERS; ++m)
    {
        auto& mm = midiModifiers[(size_t) m];
        mm.name    = xml.getStringAttribute ("mm" + juce::String(m) + "_name");
        mm.type    = static_cast<MidiModType> (
            juce::jlimit (0, NUM_MIDI_MOD_TYPES - 1, xml.getIntAttribute ("mm" + juce::String(m) + "_type", 0)));
        mm.enabled = xml.getBoolAttribute ("mm" + juce::String(m) + "_en", true);
        const juce::String routingStr = xml.getStringAttribute ("mm" + juce::String(m) + "_routing");
        for (int s = 0; s < MAX_GENERATORS; ++s)
            mm.routing[(size_t) s] = (s < routingStr.length() && routingStr[s] == '1');
    }

    numEffects = juce::jlimit (0, MAX_EFFECTS, xml.getIntAttribute ("numEffects", 0));
    for (int x = 0; x < MAX_EFFECTS; ++x)
    {
        auto& fx = effects[(size_t) x];
        fx.name    = xml.getStringAttribute ("fx" + juce::String(x) + "_name");
        fx.type    = static_cast<FxType> (
            juce::jlimit (0, NUM_FX_TYPES - 1, xml.getIntAttribute ("fx" + juce::String(x) + "_type", 0)));
        fx.enabled = xml.getBoolAttribute ("fx" + juce::String(x) + "_en", true);
        const juce::String routingStr = xml.getStringAttribute ("fx" + juce::String(x) + "_routing");
        for (int s = 0; s < MAX_GENERATORS; ++s)
            fx.routing[(size_t) s] = (s < routingStr.length() && routingStr[s] == '1');
    }

    numModulators = juce::jlimit (0, MAX_MODULATORS, xml.getIntAttribute ("numModulators", 0));
    for (int m = 0; m < MAX_MODULATORS; ++m)
    {
        auto& mod = modulators[(size_t) m];
        mod.name       = xml.getStringAttribute ("mod" + juce::String(m) + "_name");
        mod.sourceType = static_cast<ModulatorSourceType> (
            juce::jlimit (0, NUM_MODULATOR_SOURCE_TYPES - 1, xml.getIntAttribute ("mod" + juce::String(m) + "_source", 0)));
        mod.enabled    = xml.getBoolAttribute ("mod" + juce::String(m) + "_en", true);
        mod.targetParamID = xml.getStringAttribute ("mod" + juce::String(m) + "_target");
    }
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
    sampler.loadSample (slotIndex, file);
}

void ViolentAudioProcessor::randomizeAll()
{
    auto& rng = juce::Random::getSystemRandom();

    auto randF = [this, &rng] (const juce::String& id)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterFloat*> (apvts.getParameter (id)))
        {
            const auto range = p->getNormalisableRange();
            *p = range.start + rng.nextFloat() * (range.end - range.start);
        }
    };
    auto randI = [this, &rng] (const juce::String& id)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterInt*> (apvts.getParameter (id)))
            *p = p->getRange().getStart() + rng.nextInt (p->getRange().getLength() + 1);
    };
    auto randC = [this, &rng] (const juce::String& id, int numChoices)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (id)))
            *p = rng.nextInt (numChoices);
    };

    auto randomizeFxParamsAt = [&] (FxType type,
                                     const juce::String& drive, const juce::String& tone, const juce::String& level, const juce::String& distType,
                                     const juce::String& thresh, const juce::String& ratio, const juce::String& attack, const juce::String& release, const juce::String& makeup,
                                     const juce::String& room, const juce::String& damping, const juce::String& wet, const juce::String& width,
                                     const juce::String& filterType, const juce::String& filterCut, const juce::String& filterRes)
    {
        switch (type)
        {
            case FxType::Distortion:
                randF (drive); randF (tone); randF (level); randC (distType, 3);
                break;
            case FxType::Compressor:
                randF (thresh); randF (ratio); randF (attack); randF (release); randF (makeup);
                break;
            case FxType::Gate:
                randF (thresh); randF (ratio); randF (attack); randF (release);
                break;
            case FxType::Reverb:
                randF (room); randF (damping); randF (wet); randF (width);
                break;
            case FxType::Filter:
                randC (filterType, 4); randF (filterCut); randF (filterRes);
                break;
            default: break;
        }
    };

    for (int s = 0; s < numActiveGenerators; ++s)
    {
        // Synth / source — exclude Sample (index 5) from the random waveform
        // choice, since randomizing onto it without a loaded buffer is silence.
        randC (ParamIDs::genSrcType (s), (int) SourceType::Sample);
        randF (ParamIDs::genSrcGain (s));
        randI (ParamIDs::genSrcOct  (s));
        randI (ParamIDs::genSrcSemi (s));
        randF (ParamIDs::genSrcDet  (s));
        randF (ParamIDs::genSrcPhase (s));
        randF (ParamIDs::genSrcPW   (s));
        randF (ParamIDs::genSrcPan  (s));
        randF (ParamIDs::genSrcVel  (s));
        randI (ParamIDs::genSrcUni  (s));
        randF (ParamIDs::genSrcUniSpread (s));
        randF (ParamIDs::genSrcAtt (s));
        randF (ParamIDs::genSrcDec (s));
        randF (ParamIDs::genSrcSus (s));
        randF (ParamIDs::genSrcRel (s));
        randF (ParamIDs::generatorLevel (s));
        randF (ParamIDs::generatorPan   (s));
    }

    // Shared Effect Components — keeps each one's existing type, randomizes
    // only the params that type actually uses.
    for (int x = 0; x < numEffects; ++x)
        randomizeFxParamsAt (effects[(size_t) x].type,
            ParamIDs::effectDrive (x), ParamIDs::effectTone (x), ParamIDs::effectLevel (x), ParamIDs::effectDistType (x),
            ParamIDs::effectThresh (x), ParamIDs::effectRatio (x), ParamIDs::effectAttack (x), ParamIDs::effectRelease (x), ParamIDs::effectMakeup (x),
            ParamIDs::effectRoom (x), ParamIDs::effectDamping (x), ParamIDs::effectWet (x), ParamIDs::effectWidth (x),
            ParamIDs::effectFilterType (x), ParamIDs::effectFilterCut (x), ParamIDs::effectFilterRes (x));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViolentAudioProcessor();
}

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
ViolentAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // ---- Generators ----
    for (int g = 0; g < MAX_GENERATORS; ++g)
    {
        const String sn = " Generator " + String (g + 1) + " ";

        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::genEn (g),    sn + "Enabled", g == 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genLevel (g), sn + "Level",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genPan (g),   sn + "Pan",
            NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));

        // Source (osc/sampler unified)
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::genSrcType (g),      sn + "Source Type",
            StringArray { "Sine", "Saw", "Square", "Triangle", "Noise", "Sample" }, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcGain (g),      sn + "Source Gain",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::genSrcOct (g),       sn + "Source Octave", -3, 3, 0));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::genSrcSemi (g),      sn + "Source Semitone", -12, 12, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcDet (g),       sn + "Source Detune",
            NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("ct")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcPhase (g),     sn + "Source Phase",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcPW (g),        sn + "Source Pulse Width",
            NormalisableRange<float> (0.05f, 0.95f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcPan (g),       sn + "Source Pan",
            NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcVel (g),       sn + "Source Velocity",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::genSrcUni (g),       sn + "Source Unison", 1, 8, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcUniSpread (g), sn + "Source Unison Spread",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 15.0f,
            AudioParameterFloatAttributes().withLabel ("ct")));
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::genSrcLoop (g),      sn + "Source Loop", false));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcAtt (g), sn + "Source Attack",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.01f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcDec (g), sn + "Source Decay",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.1f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcSus (g), sn + "Source Sustain",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.7f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::genSrcRel (g), sn + "Source Release",
            NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.3f), 0.3f,
            AudioParameterFloatAttributes().withLabel ("s")));

        // Filters
        for (int f = 0; f < MAX_GEN_FILTERS; ++f)
        {
            const String fn = sn + "Filter " + String (f + 1) + " ";
            params.push_back (std::make_unique<AudioParameterBool> (
                ParamIDs::genFltEn (g, f), fn + "Enabled", false));
            params.push_back (std::make_unique<AudioParameterChoice> (
                ParamIDs::genFltType (g, f), fn + "Type",
                StringArray { "LP", "HP", "BP", "Notch" }, 0));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFltCut (g, f), fn + "Cutoff",
                NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 8000.0f,
                AudioParameterFloatAttributes().withLabel ("Hz")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genFltRes (g, f), fn + "Resonance",
                NormalisableRange<float> (0.1f, 12.0f, 0.01f, 0.5f), 0.707f));
        }

        // Sends to shared FX buses
        for (int b = 0; b < MAX_FX_BUSES; ++b)
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::genSend (g, b), sn + "Send " + String (b + 1),
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    }

    // ---- Shared FX buses ----
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

    for (auto& fx : fxBusDSP)
        fx.prepare (processSpec);

    for (auto& buf : busBuffers)
        buf.setSize (2, samplesPerBlock, false, true, true);

    for (auto& band : eqBands)
        band.prepare (processSpec);
    setEQBand (0, 0.0f);   // init state

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
void ViolentAudioProcessor::loadGeneratorParams (int g)
{
    auto& dsp = generatorDSP[(size_t) g];
    auto& gs  = generators[(size_t) g];

    gs.enabled = apvts.getRawParameterValue (ParamIDs::genEn (g))->load() > 0.5f;
    dsp.level  = apvts.getRawParameterValue (ParamIDs::genLevel (g))->load();
    dsp.pan    = apvts.getRawParameterValue (ParamIDs::genPan   (g))->load();

    auto& o    = dsp.osc;
    o.en          = gs.enabled;
    o.type        = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcType (g))->load());
    o.gainLin     = juce::Decibels::decibelsToGain (
                        apvts.getRawParameterValue (ParamIDs::genSrcGain (g))->load());
    o.octave      = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcOct  (g))->load());
    o.semitone    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcSemi (g))->load());
    o.detune      = apvts.getRawParameterValue (ParamIDs::genSrcDet       (g))->load();
    o.phase       = apvts.getRawParameterValue (ParamIDs::genSrcPhase     (g))->load();
    o.pulseWidth  = apvts.getRawParameterValue (ParamIDs::genSrcPW        (g))->load();
    o.pan         = apvts.getRawParameterValue (ParamIDs::genSrcPan       (g))->load();
    o.velSens     = apvts.getRawParameterValue (ParamIDs::genSrcVel       (g))->load();
    o.unisonVoices= static_cast<int> (apvts.getRawParameterValue (ParamIDs::genSrcUni (g))->load());
    o.unisonSpread= apvts.getRawParameterValue (ParamIDs::genSrcUniSpread (g))->load();
    o.att         = apvts.getRawParameterValue (ParamIDs::genSrcAtt (g))->load();
    o.dec         = apvts.getRawParameterValue (ParamIDs::genSrcDec (g))->load();
    o.sus         = apvts.getRawParameterValue (ParamIDs::genSrcSus (g))->load();
    o.rel         = apvts.getRawParameterValue (ParamIDs::genSrcRel (g))->load();

    for (int f = 0; f < MAX_GEN_FILTERS; ++f)
    {
        auto& flt   = dsp.filters[(size_t) f];
        flt.en      = apvts.getRawParameterValue (ParamIDs::genFltEn   (g, f))->load() > 0.5f;
        flt.type    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genFltType (g, f))->load());
        flt.cutoff  = apvts.getRawParameterValue (ParamIDs::genFltCut  (g, f))->load();
        flt.res     = apvts.getRawParameterValue (ParamIDs::genFltRes  (g, f))->load();
    }

    for (int b = 0; b < MAX_FX_BUSES; ++b)
        dsp.sendGain[(size_t) b] = apvts.getRawParameterValue (ParamIDs::genSend (g, b))->load();
}

//==============================================================================
void ViolentAudioProcessor::processMidi (const juce::MidiBuffer& midi)
{
    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn())
        {
            const int   note = msg.getNoteNumber();
            const float vel  = msg.getFloatVelocity();
            for (int g = 0; g < numActiveGenerators; ++g)
            {
                if (!generators[(size_t) g].enabled) continue;
                int vi = findFreeVoice (g);
                if (vi < 0) vi = findVoiceToSteal (g);
                if (vi >= 0) startVoice (vi, note, vel, g);
            }
        }
        else if (msg.isNoteOff())
        {
            stopNote (msg.getNoteNumber());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            allNotesOff();
        }
    }
}

int ViolentAudioProcessor::findFreeVoice (int genIdx) const noexcept
{
    for (int i = 0; i < GEN_VOICES; ++i)
        if (!voices[(size_t) i].active && voices[(size_t) i].genIdx == genIdx)
            return i;
    // Also accept unassigned (default genIdx == 0)
    for (int i = 0; i < GEN_VOICES; ++i)
        if (!voices[(size_t) i].active) return i;
    return -1;
}

int ViolentAudioProcessor::findVoiceToSteal (int genIdx) const noexcept
{
    // Steal the oldest released voice from this generator, else any
    for (int i = 0; i < GEN_VOICES; ++i)
        if (voices[(size_t) i].genIdx == genIdx && !voices[(size_t) i].isActive())
            return i;
    return 0;
}

void ViolentAudioProcessor::startVoice (int vi, int note, float velocity, int genIdx)
{
    auto& v       = voices[(size_t) vi];
    v.note        = note;
    v.genIdx      = genIdx;
    v.velocity    = velocity;
    v.baseFreqHz  = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (note));
    v.active      = true;

    const auto& o = generatorDSP[(size_t) genIdx].osc;
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

void ViolentAudioProcessor::allNotesOff()
{
    for (auto& v : voices) v.reset();
}

//==============================================================================
void ViolentAudioProcessor::renderGenerator (int g, juce::AudioBuffer<float>& master)
{
    auto& dsp = generatorDSP[(size_t) g];
    auto& gs  = generators[(size_t) g];
    if (!gs.enabled) return;

    const int   numSamples = master.getNumSamples();
    const float sr         = static_cast<float> (processSpec.sampleRate);
    const auto& o          = dsp.osc;

    // Clear scratch
    dsp.scratch.setSize (2, numSamples, false, false, true);
    dsp.scratch.clear();

    float* L = dsp.scratch.getWritePointer (0);
    float* R = dsp.scratch.getWritePointer (1);

    // Render all voices belonging to this generator
    bool anyActive = false;
    for (auto& v : voices)
    {
        if (!v.active || v.genIdx != g) continue;
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
                    // Sample playback — uses this generator's own sample slot if loaded
                    juce::SpinLock::ScopedTryLockType tryLock (sampleModules[(size_t) g].lock);
                    if (tryLock.isLocked() && sampleModules[(size_t) g].hasData)
                    {
                        const auto& sm   = sampleModules[(size_t) g];
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

            L[n] += uniL / (float) nUni * env * vol;
            R[n] += uniR / (float) nUni * env * vol;
        }

        if (!v.isActive()) v.active = false;
    }

    if (!anyActive) return;

    // Generator filters
    applyGeneratorFilters (dsp, gs, dsp.scratch);

    // Dry mix into master with level and pan
    const float mixL = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f - dsp.pan));
    const float mixR = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f + dsp.pan));
    master.addFromWithRamp (0, 0, L, numSamples, mixL, mixL);
    master.addFromWithRamp (1, 0, R, numSamples, mixR, mixR);

    // Sends — accumulate this generator's (filtered) signal into shared FX buses
    for (int b = 0; b < numFxBuses; ++b)
    {
        const float send = dsp.sendGain[(size_t) b];
        if (send <= 0.0f) continue;
        auto& busBuf = busBuffers[(size_t) b];
        busBuf.addFrom (0, 0, dsp.scratch, 0, 0, numSamples, send);
        busBuf.addFrom (1, 0, dsp.scratch, 1, 0, numSamples, send);
    }
}

void ViolentAudioProcessor::applyGeneratorFilters (GeneratorDSP& dsp, const GeneratorState& gs,
                                                    juce::AudioBuffer<float>& buf)
{
    const int   numSamples = buf.getNumSamples();
    const float sr         = static_cast<float> (processSpec.sampleRate);
    const int   numCh      = buf.getNumChannels();

    for (int f = 0; f < gs.numFilters; ++f)
    {
        const auto& flt = dsp.filters[(size_t) f];
        if (!flt.en) continue;

        dsp.svFilters[(size_t) f].setCoefficients (flt.cutoff, flt.res, sr);

        for (int ch = 0; ch < numCh; ++ch)
        {
            float* data = buf.getWritePointer (ch);
            for (int n = 0; n < numSamples; ++n)
                data[n] = dsp.svFilters[(size_t) f].process (ch, data[n], flt.type);
        }
    }
}

void ViolentAudioProcessor::processFxBuses (juce::AudioBuffer<float>& master, int numSamples)
{
    for (int b = 0; b < numFxBuses; ++b)
    {
        auto& buf   = busBuffers[(size_t) b];
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
        busBuffers[(size_t) b].setSize (2, numSamples, false, false, true);
        busBuffers[(size_t) b].clear();
    }

    // Load params for all active generators
    for (int g = 0; g < numActiveGenerators; ++g)
        loadGeneratorParams (g);

    // MIDI
    processMidi (midiMessages);

    // Render each generator into master (also feeds sends into FX buses)
    for (int g = 0; g < numActiveGenerators; ++g)
        renderGenerator (g, buffer);

    // Process shared FX buses and sum their output back into master
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

void ViolentAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto xml = apvts.copyState().createXml();
    if (xml == nullptr) return;

    xml->setAttribute ("numActiveGenerators", numActiveGenerators);

    for (int g = 0; g < MAX_GENERATORS; ++g)
    {
        const auto& gs = generators[(size_t) g];
        xml->setAttribute ("gen_" + juce::String (g) + "_numFilters", gs.numFilters);

        // Sample path for sample-mode generators
        juce::SpinLock::ScopedTryLockType tryLock (sampleModules[(size_t) g].lock);
        if (tryLock.isLocked() && sampleModules[(size_t) g].hasData)
            xml->setAttribute ("gen_" + juce::String (g) + "_samplepath",
                               sampleModules[(size_t) g].filePath);
    }

    xml->setAttribute ("numFxBuses", numFxBuses);
    for (int b = 0; b < MAX_FX_BUSES; ++b)
        xml->setAttribute ("bus" + juce::String (b) + "_fxtype",
                           static_cast<int> (fxBusTypes[(size_t) b]));

    copyXmlToBinary (*xml, destData);
}

void ViolentAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr) return;

    if (xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));

    numActiveGenerators = juce::jlimit (1, MAX_GENERATORS,
                                     xml->getIntAttribute ("numActiveGenerators", 1));

    for (int g = 0; g < MAX_GENERATORS; ++g)
    {
        auto& gs = generators[(size_t) g];
        gs.numFilters = juce::jlimit (0, MAX_GEN_FILTERS,
                                      xml->getIntAttribute ("gen_" + juce::String (g) + "_numFilters", 0));

        const juce::String path = xml->getStringAttribute ("gen_" + juce::String (g) + "_samplepath");
        if (path.isNotEmpty())
        {
            const juce::File f (path);
            if (f.existsAsFile()) loadSample (g, f);
        }
    }

    numFxBuses = juce::jlimit (0, MAX_FX_BUSES, xml->getIntAttribute ("numFxBuses", 0));
    for (int b = 0; b < MAX_FX_BUSES; ++b)
        fxBusTypes[(size_t) b] = static_cast<FxType> (
            juce::jlimit (0, NUM_FX_TYPES - 1,
                          xml->getIntAttribute ("bus" + juce::String (b) + "_fxtype", 0)));
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

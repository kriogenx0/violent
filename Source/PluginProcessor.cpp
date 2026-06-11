#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
ViolentAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // ---- Streams ----
    for (int s = 0; s < MAX_STREAMS; ++s)
    {
        const String sn = " Stream " + String (s + 1) + " ";

        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::streamEn (s),    sn + "Enabled", s == 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::streamLevel (s), sn + "Level",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::streamPan (s),   sn + "Pan",
            NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));

        // Source (osc/sampler unified)
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::stSrcType (s),      sn + "Source Type",
            StringArray { "Sine", "Saw", "Square", "Triangle", "Noise", "Sample" }, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcGain (s),      sn + "Source Gain",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::stSrcOct (s),       sn + "Source Octave", -3, 3, 0));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::stSrcSemi (s),      sn + "Source Semitone", -12, 12, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcDet (s),       sn + "Source Detune",
            NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("ct")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcPhase (s),     sn + "Source Phase",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcPW (s),        sn + "Source Pulse Width",
            NormalisableRange<float> (0.05f, 0.95f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcPan (s),       sn + "Source Pan",
            NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcVel (s),       sn + "Source Velocity",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::stSrcUni (s),       sn + "Source Unison", 1, 8, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcUniSpread (s), sn + "Source Unison Spread",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 15.0f,
            AudioParameterFloatAttributes().withLabel ("ct")));
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::stSrcLoop (s),      sn + "Source Loop", false));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcAtt (s), sn + "Source Attack",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.01f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcDec (s), sn + "Source Decay",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.1f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcSus (s), sn + "Source Sustain",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.7f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::stSrcRel (s), sn + "Source Release",
            NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.3f), 0.3f,
            AudioParameterFloatAttributes().withLabel ("s")));

        // Filters
        for (int f = 0; f < MAX_STREAM_FILTERS; ++f)
        {
            const String fn = sn + "Filter " + String (f + 1) + " ";
            params.push_back (std::make_unique<AudioParameterBool> (
                ParamIDs::stFltEn (s, f), fn + "Enabled", false));
            params.push_back (std::make_unique<AudioParameterChoice> (
                ParamIDs::stFltType (s, f), fn + "Type",
                StringArray { "LP", "HP", "BP", "Notch" }, 0));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFltCut (s, f), fn + "Cutoff",
                NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 8000.0f,
                AudioParameterFloatAttributes().withLabel ("Hz")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFltRes (s, f), fn + "Resonance",
                NormalisableRange<float> (0.1f, 12.0f, 0.01f, 0.5f), 0.707f));
        }

        // FX
        for (int x = 0; x < MAX_STREAM_FX; ++x)
        {
            const String xn = sn + "FX" + String (x) + " ";
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxDrive    (s, x), xn + "Drive",
                NormalisableRange<float> (1.0f, 100.0f, 0.1f, 0.4f), 1.0f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxTone     (s, x), xn + "Tone",
                NormalisableRange<float> (200.0f, 8000.0f, 1.0f, 0.5f), 4000.0f,
                AudioParameterFloatAttributes().withLabel ("Hz")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxLevel    (s, x), xn + "Level",
                NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
                AudioParameterFloatAttributes().withLabel ("dB")));
            params.push_back (std::make_unique<AudioParameterChoice> (
                ParamIDs::stFxDistType (s, x), xn + "Dist Type",
                StringArray { "Soft Clip", "Hard Clip", "Fuzz" }, 0));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxThresh   (s, x), xn + "Threshold",
                NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -12.0f,
                AudioParameterFloatAttributes().withLabel ("dB")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxRatio    (s, x), xn + "Ratio",
                NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.5f), 4.0f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxAttack   (s, x), xn + "Attack",
                NormalisableRange<float> (0.1f, 200.0f, 0.1f, 0.5f), 10.0f,
                AudioParameterFloatAttributes().withLabel ("ms")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxRelease  (s, x), xn + "Release",
                NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.5f), 100.0f,
                AudioParameterFloatAttributes().withLabel ("ms")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxMakeup   (s, x), xn + "Makeup",
                NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f,
                AudioParameterFloatAttributes().withLabel ("dB")));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxRoom     (s, x), xn + "Room",
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxDamping  (s, x), xn + "Damping",
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxWet      (s, x), xn + "Wet",
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.33f));
            params.push_back (std::make_unique<AudioParameterFloat> (
                ParamIDs::stFxWidth    (s, x), xn + "Width",
                NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        }
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

    for (auto& dsp : streamDSP)
        dsp.prepare (processSpec);

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
void ViolentAudioProcessor::loadStreamParams (int s)
{
    auto& dsp = streamDSP[(size_t) s];
    auto& st  = streams[(size_t) s];

    st.enabled = apvts.getRawParameterValue (ParamIDs::streamEn (s))->load() > 0.5f;
    dsp.level  = apvts.getRawParameterValue (ParamIDs::streamLevel (s))->load();
    dsp.pan    = apvts.getRawParameterValue (ParamIDs::streamPan   (s))->load();

    auto& o    = dsp.osc;
    o.en          = st.enabled;
    o.type        = static_cast<int> (apvts.getRawParameterValue (ParamIDs::stSrcType (s))->load());
    o.gainLin     = juce::Decibels::decibelsToGain (
                        apvts.getRawParameterValue (ParamIDs::stSrcGain (s))->load());
    o.octave      = static_cast<int> (apvts.getRawParameterValue (ParamIDs::stSrcOct  (s))->load());
    o.semitone    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::stSrcSemi (s))->load());
    o.detune      = apvts.getRawParameterValue (ParamIDs::stSrcDet       (s))->load();
    o.phase       = apvts.getRawParameterValue (ParamIDs::stSrcPhase     (s))->load();
    o.pulseWidth  = apvts.getRawParameterValue (ParamIDs::stSrcPW        (s))->load();
    o.pan         = apvts.getRawParameterValue (ParamIDs::stSrcPan       (s))->load();
    o.velSens     = apvts.getRawParameterValue (ParamIDs::stSrcVel       (s))->load();
    o.unisonVoices= static_cast<int> (apvts.getRawParameterValue (ParamIDs::stSrcUni (s))->load());
    o.unisonSpread= apvts.getRawParameterValue (ParamIDs::stSrcUniSpread (s))->load();
    o.att         = apvts.getRawParameterValue (ParamIDs::stSrcAtt (s))->load();
    o.dec         = apvts.getRawParameterValue (ParamIDs::stSrcDec (s))->load();
    o.sus         = apvts.getRawParameterValue (ParamIDs::stSrcSus (s))->load();
    o.rel         = apvts.getRawParameterValue (ParamIDs::stSrcRel (s))->load();

    for (int f = 0; f < MAX_STREAM_FILTERS; ++f)
    {
        auto& flt   = dsp.filters[(size_t) f];
        flt.en      = apvts.getRawParameterValue (ParamIDs::stFltEn   (s, f))->load() > 0.5f;
        flt.type    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::stFltType (s, f))->load());
        flt.cutoff  = apvts.getRawParameterValue (ParamIDs::stFltCut  (s, f))->load();
        flt.res     = apvts.getRawParameterValue (ParamIDs::stFltRes  (s, f))->load();
    }
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
            for (int s = 0; s < numActiveStreams; ++s)
            {
                if (!streams[(size_t) s].enabled) continue;
                int vi = findFreeVoice (s);
                if (vi < 0) vi = findVoiceToSteal (s);
                if (vi >= 0) startVoice (vi, note, vel, s);
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

int ViolentAudioProcessor::findFreeVoice (int streamIdx) const noexcept
{
    for (int i = 0; i < STREAM_VOICES; ++i)
        if (!voices[(size_t) i].active && voices[(size_t) i].streamIdx == streamIdx)
            return i;
    // Also accept unassigned (default streamIdx == 0)
    for (int i = 0; i < STREAM_VOICES; ++i)
        if (!voices[(size_t) i].active) return i;
    return -1;
}

int ViolentAudioProcessor::findVoiceToSteal (int streamIdx) const noexcept
{
    // Steal the oldest released voice from this stream, else any
    for (int i = 0; i < STREAM_VOICES; ++i)
        if (voices[(size_t) i].streamIdx == streamIdx && !voices[(size_t) i].isActive())
            return i;
    return 0;
}

void ViolentAudioProcessor::startVoice (int vi, int note, float velocity, int streamIdx)
{
    auto& v       = voices[(size_t) vi];
    v.note        = note;
    v.streamIdx   = streamIdx;
    v.velocity    = velocity;
    v.baseFreqHz  = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (note));
    v.active      = true;

    const auto& o = streamDSP[(size_t) streamIdx].osc;
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
void ViolentAudioProcessor::renderStream (int s, juce::AudioBuffer<float>& master)
{
    auto& dsp = streamDSP[(size_t) s];
    auto& st  = streams[(size_t) s];
    if (!st.enabled) return;

    const int   numSamples = master.getNumSamples();
    const float sr         = static_cast<float> (processSpec.sampleRate);
    const auto& o          = dsp.osc;

    // Clear scratch
    dsp.scratch.setSize (2, numSamples, false, false, true);
    dsp.scratch.clear();

    float* L = dsp.scratch.getWritePointer (0);
    float* R = dsp.scratch.getWritePointer (1);

    // Render all voices belonging to this stream
    bool anyActive = false;
    for (auto& v : voices)
    {
        if (!v.active || v.streamIdx != s) continue;
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
                    // Sample playback — use the stream's sample slot if loaded
                    // (stream index maps to sample slot index)
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

    if (!anyActive) return;

    // Stream filters
    applyStreamFilters (dsp, st, dsp.scratch);

    // Stream FX
    applyStreamFx (s, dsp, st, dsp.scratch);

    // Mix into master with level and pan
    const float mixL = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f - dsp.pan));
    const float mixR = juce::jlimit (0.0f, 1.0f, dsp.level * (1.0f + dsp.pan));
    master.addFromWithRamp (0, 0, L, numSamples, mixL, mixL);
    master.addFromWithRamp (1, 0, R, numSamples, mixR, mixR);
}

void ViolentAudioProcessor::applyStreamFilters (StreamDSP& dsp, const StreamState& st,
                                                 juce::AudioBuffer<float>& buf)
{
    const int   numSamples = buf.getNumSamples();
    const float sr         = static_cast<float> (processSpec.sampleRate);
    const int   numCh      = buf.getNumChannels();

    for (int f = 0; f < st.numFilters; ++f)
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

void ViolentAudioProcessor::applyStreamFx (int s, StreamDSP& dsp, const StreamState& st,
                                            juce::AudioBuffer<float>& buf)
{
    juce::dsp::AudioBlock<float> block (buf);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    for (int x = 0; x < st.numFx; ++x)
    {
        auto& fxdsp = dsp.fxDSP[(size_t) x];
        const FxType type = st.fxTypes[(size_t) x];

        switch (type)
        {
            case FxType::Distortion:
            {
                const float drive = apvts.getRawParameterValue (ParamIDs::stFxDrive    (s, x))->load();
                const float level = juce::Decibels::decibelsToGain (
                                        apvts.getRawParameterValue (ParamIDs::stFxLevel (s, x))->load());
                const float tone  = apvts.getRawParameterValue (ParamIDs::stFxTone     (s, x))->load();
                const int   dtype = static_cast<int> (
                                        apvts.getRawParameterValue (ParamIDs::stFxDistType (s, x))->load());

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
                fxdsp.compressor.setThreshold (apvts.getRawParameterValue (ParamIDs::stFxThresh  (s, x))->load());
                fxdsp.compressor.setRatio     (apvts.getRawParameterValue (ParamIDs::stFxRatio   (s, x))->load());
                fxdsp.compressor.setAttack    (apvts.getRawParameterValue (ParamIDs::stFxAttack  (s, x))->load());
                fxdsp.compressor.setRelease   (apvts.getRawParameterValue (ParamIDs::stFxRelease (s, x))->load());
                fxdsp.makeup.setGainDecibels  (apvts.getRawParameterValue (ParamIDs::stFxMakeup  (s, x))->load());
                fxdsp.compressor.process (ctx);
                fxdsp.makeup.process (ctx);
                break;
            case FxType::Gate:
                fxdsp.gate.setThreshold (apvts.getRawParameterValue (ParamIDs::stFxThresh  (s, x))->load());
                fxdsp.gate.setRatio     (apvts.getRawParameterValue (ParamIDs::stFxRatio   (s, x))->load());
                fxdsp.gate.setAttack    (apvts.getRawParameterValue (ParamIDs::stFxAttack  (s, x))->load());
                fxdsp.gate.setRelease   (apvts.getRawParameterValue (ParamIDs::stFxRelease (s, x))->load());
                fxdsp.gate.process (ctx);
                break;
            case FxType::Reverb:
            {
                juce::dsp::Reverb::Parameters p;
                p.roomSize   = apvts.getRawParameterValue (ParamIDs::stFxRoom    (s, x))->load();
                p.damping    = apvts.getRawParameterValue (ParamIDs::stFxDamping (s, x))->load();
                p.wetLevel   = apvts.getRawParameterValue (ParamIDs::stFxWet     (s, x))->load();
                p.dryLevel   = 1.0f - p.wetLevel;
                p.width      = apvts.getRawParameterValue (ParamIDs::stFxWidth   (s, x))->load();
                p.freezeMode = 0.0f;
                fxdsp.reverb.setParameters (p);
                fxdsp.reverb.process (ctx);
                break;
            }
            default: break;
        }
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

    // Load params for all active streams
    for (int s = 0; s < numActiveStreams; ++s)
        loadStreamParams (s);

    // MIDI
    processMidi (midiMessages);

    // Render each stream into master
    for (int s = 0; s < numActiveStreams; ++s)
        renderStream (s, buffer);

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

    xml->setAttribute ("numActiveStreams", numActiveStreams);

    for (int s = 0; s < MAX_STREAMS; ++s)
    {
        const auto& st = streams[(size_t) s];
        xml->setAttribute ("st_" + juce::String(s) + "_numFilters", st.numFilters);
        xml->setAttribute ("st_" + juce::String(s) + "_numFx",      st.numFx);
        for (int x = 0; x < MAX_STREAM_FX; ++x)
            xml->setAttribute ("st_" + juce::String(s) + "_fx" + juce::String(x) + "_fxtype",
                               static_cast<int> (st.fxTypes[(size_t) x]));

        // Sample path for sample-mode streams
        juce::SpinLock::ScopedTryLockType tryLock (sampleModules[(size_t) s].lock);
        if (tryLock.isLocked() && sampleModules[(size_t) s].hasData)
            xml->setAttribute ("st_" + juce::String(s) + "_samplepath",
                               sampleModules[(size_t) s].filePath);
    }

    copyXmlToBinary (*xml, destData);
}

void ViolentAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr) return;

    if (xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));

    numActiveStreams = juce::jlimit (1, MAX_STREAMS,
                                     xml->getIntAttribute ("numActiveStreams", 1));

    for (int s = 0; s < MAX_STREAMS; ++s)
    {
        auto& st    = streams[(size_t) s];
        st.numFilters = juce::jlimit (0, MAX_STREAM_FILTERS,
                                      xml->getIntAttribute ("st_" + juce::String(s) + "_numFilters", 0));
        st.numFx      = juce::jlimit (0, MAX_STREAM_FX,
                                      xml->getIntAttribute ("st_" + juce::String(s) + "_numFx", 0));
        for (int x = 0; x < MAX_STREAM_FX; ++x)
            st.fxTypes[(size_t) x] = static_cast<FxType> (
                juce::jlimit (0, NUM_FX_TYPES - 1,
                              xml->getIntAttribute ("st_" + juce::String(s) + "_fx" + juce::String(x) + "_fxtype", 0)));

        const juce::String path = xml->getStringAttribute ("st_" + juce::String(s) + "_samplepath");
        if (path.isNotEmpty())
        {
            const juce::File f (path);
            if (f.existsAsFile()) loadSample (s, f);
        }
    }
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

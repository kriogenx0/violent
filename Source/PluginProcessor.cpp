#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
ViolentAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // ---- Sample slots ----
    for (int i = 0; i < MAX_SAMPLES; ++i)
    {
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::smpEn (i), "Sample " + String (i + 1) + " Enabled", false));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::smpRoot (i), "Sample " + String (i + 1) + " Root Note",
            0, 127, 60));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpGain (i), "Sample " + String (i + 1) + " Gain",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpAtt (i), "Sample " + String (i + 1) + " Attack",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.005f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpDec (i), "Sample " + String (i + 1) + " Decay",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.1f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpSus (i), "Sample " + String (i + 1) + " Sustain",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::smpRel (i), "Sample " + String (i + 1) + " Release",
            NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.3f), 0.2f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::smpLoop (i), "Sample " + String (i + 1) + " Loop", false));
    }

    // ---- Synth slots ----
    for (int i = 0; i < MAX_SYNTHS; ++i)
    {
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::synthEn (i), "Synth " + String (i + 1) + " Enabled", i == 0));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::synthType (i), "Synth " + String (i + 1) + " Wave",
            StringArray { "Sine", "Saw", "Square", "Triangle" }, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthGain (i), "Synth " + String (i + 1) + " Gain",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthDet (i), "Synth " + String (i + 1) + " Detune",
            NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("ct")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthAtt (i), "Synth " + String (i + 1) + " Attack",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.01f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthDec (i), "Synth " + String (i + 1) + " Decay",
            NormalisableRange<float> (0.001f, 4.0f, 0.001f, 0.3f), 0.1f,
            AudioParameterFloatAttributes().withLabel ("s")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthSus (i), "Synth " + String (i + 1) + " Sustain",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.7f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthRel (i), "Synth " + String (i + 1) + " Release",
            NormalisableRange<float> (0.001f, 8.0f, 0.001f, 0.3f), 0.3f,
            AudioParameterFloatAttributes().withLabel ("s")));
    }

    // ---- Filter slots ----
    for (int i = 0; i < MAX_FILTERS; ++i)
    {
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::fltEn (i), "Filter " + String (i + 1) + " Enabled", false));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::fltType (i), "Filter " + String (i + 1) + " Type",
            StringArray { "Low Pass", "High Pass", "Band Pass", "Notch" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fltCut (i), "Filter " + String (i + 1) + " Cutoff",
            NormalisableRange<float> (20.0f, 20000.0f, 0.1f, 0.3f), 8000.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fltRes (i), "Filter " + String (i + 1) + " Resonance",
            NormalisableRange<float> (0.1f, 12.0f, 0.01f, 0.5f), 0.707f));
    }

    // ---- LFO slots ----
    for (int i = 0; i < MAX_LFOS; ++i)
    {
        params.push_back (std::make_unique<AudioParameterBool> (
            ParamIDs::lfoEn (i), "LFO " + String (i + 1) + " Enabled", false));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::lfoShape (i), "LFO " + String (i + 1) + " Shape",
            StringArray { "Sine", "Triangle", "Saw", "Square" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::lfoRate (i), "LFO " + String (i + 1) + " Rate",
            NormalisableRange<float> (0.01f, 20.0f, 0.01f, 0.4f), 1.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::lfoDepth (i), "LFO " + String (i + 1) + " Depth",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
        // target: 0=None 1=Filter Cutoff 2=Osc Volume 3=Osc Detune
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::lfoTarget (i), "LFO " + String (i + 1) + " Target",
            StringArray { "None", "Filter Cutoff", "Osc Volume", "Osc Detune" }, 0));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::lfoTgtSlt (i), "LFO " + String (i + 1) + " Target Slot",
            0, 3, 0));
    }

    // ---- EQ ----
    params.push_back (std::make_unique<AudioParameterBool>  (ParamIDs::EQ_ENABLED, "EQ Enabled", true));
    for (int i = 0; i < NUM_EQ_BANDS; ++i)
    {
        juce::String id   = juce::String ("eq_band_") + juce::String (i);
        juce::String name = juce::String ("EQ Band ")  + juce::String (i);
        params.push_back (std::make_unique<AudioParameterFloat> (
            id, name, NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
    }

    // ---- Reverb ----
    params.push_back (std::make_unique<AudioParameterBool>  (ParamIDs::REVERB_ENABLED, "Reverb Enabled", false));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::REVERB_ROOM,    "Room Size",  NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::REVERB_DAMPING, "Damping",    NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::REVERB_WET,     "Wet Level",  NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.33f));
    params.push_back (std::make_unique<AudioParameterFloat> (ParamIDs::REVERB_WIDTH,   "Width",      NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

    // ---- Distortion ----
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

    // ---- Compressor ----
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

    // ---- Gate ----
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
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats();

    // Register listeners only for FX params (those require pre-computed coeff updates)
    const juce::StringArray fxIDs {
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
    for (const auto& id : fxIDs)
        apvts.addParameterListener (id, this);
}

ViolentAudioProcessor::~ViolentAudioProcessor()
{
    const juce::StringArray fxIDs {
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
    for (const auto& id : fxIDs)
        apvts.removeParameterListener (id, this);
}

//==============================================================================
void ViolentAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    processSpec.sampleRate       = sampleRate;
    processSpec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    processSpec.numChannels      = 2;

    // Prepare synth engine
    for (auto& v : voices)  v.prepare (sampleRate);
    for (auto& f : filters) f.reset();
    for (auto& l : lfos)    l.reset();

    // Prepare FX chain
    for (int i = 0; i < NUM_EQ_BANDS; ++i)
        eqBands[i].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sampleRate, static_cast<double> (EQ_FREQUENCIES[i]),
            static_cast<double> (EQ_Q), 1.0f);

    for (auto& band : eqBands)
        band.prepare (processSpec);

    distToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
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
void ViolentAudioProcessor::loadParams()
{
    const float sr = static_cast<float> (processSpec.sampleRate);

    for (int i = 0; i < MAX_SAMPLES; ++i)
    {
        auto& s   = sampleSlots[i];
        s.en      = apvts.getRawParameterValue (ParamIDs::smpEn   (i))->load() > 0.5f;
        s.root    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::smpRoot (i))->load());
        s.gainLin = juce::Decibels::decibelsToGain (
                       apvts.getRawParameterValue (ParamIDs::smpGain (i))->load());
        s.att     = apvts.getRawParameterValue (ParamIDs::smpAtt  (i))->load();
        s.dec     = apvts.getRawParameterValue (ParamIDs::smpDec  (i))->load();
        s.sus     = apvts.getRawParameterValue (ParamIDs::smpSus  (i))->load();
        s.rel     = apvts.getRawParameterValue (ParamIDs::smpRel  (i))->load();
        s.loop    = apvts.getRawParameterValue (ParamIDs::smpLoop (i))->load() > 0.5f;
    }

    for (int i = 0; i < MAX_SYNTHS; ++i)
    {
        auto& s  = synthSlots[i];
        s.en     = apvts.getRawParameterValue (ParamIDs::synthEn   (i))->load() > 0.5f;
        s.type   = static_cast<int> (apvts.getRawParameterValue (ParamIDs::synthType (i))->load());
        s.gainLin = juce::Decibels::decibelsToGain (
                       apvts.getRawParameterValue (ParamIDs::synthGain (i))->load());
        s.detune = apvts.getRawParameterValue (ParamIDs::synthDet (i))->load();
        s.att    = apvts.getRawParameterValue (ParamIDs::synthAtt  (i))->load();
        s.dec    = apvts.getRawParameterValue (ParamIDs::synthDec  (i))->load();
        s.sus    = apvts.getRawParameterValue (ParamIDs::synthSus  (i))->load();
        s.rel    = apvts.getRawParameterValue (ParamIDs::synthRel  (i))->load();
    }

    for (int i = 0; i < MAX_FILTERS; ++i)
    {
        auto& f  = filterSlots[i];
        f.en     = apvts.getRawParameterValue (ParamIDs::fltEn   (i))->load() > 0.5f;
        f.type   = static_cast<int> (apvts.getRawParameterValue (ParamIDs::fltType (i))->load());
        f.cutoff = apvts.getRawParameterValue (ParamIDs::fltCut  (i))->load();
        f.res    = apvts.getRawParameterValue (ParamIDs::fltRes  (i))->load();
    }

    for (int i = 0; i < MAX_LFOS; ++i)
    {
        auto& l   = lfoSlots[i];
        l.en      = apvts.getRawParameterValue (ParamIDs::lfoEn     (i))->load() > 0.5f;
        l.shape   = static_cast<int> (apvts.getRawParameterValue (ParamIDs::lfoShape  (i))->load());
        l.rate    = apvts.getRawParameterValue (ParamIDs::lfoRate   (i))->load();
        l.depth   = apvts.getRawParameterValue (ParamIDs::lfoDepth  (i))->load();
        l.target  = apvts.getRawParameterValue (ParamIDs::lfoTarget (i))->load();
        l.tgtSlot = apvts.getRawParameterValue (ParamIDs::lfoTgtSlt (i))->load();
    }

    juce::ignoreUnused (sr);
}

void ViolentAudioProcessor::computeLFOs (float sampleRate)
{
    lfoFltMod.fill (0.0f);
    lfoVolMod.fill (0.0f);
    lfoDetMod.fill (0.0f);

    for (int i = 0; i < MAX_LFOS; ++i)
    {
        const auto& l = lfoSlots[i];
        if (!l.en) continue;

        const float val  = lfos[i].tick (l.rate, sampleRate, l.shape) * l.depth;
        const int   slot = static_cast<int> (l.tgtSlot);

        switch (static_cast<int> (l.target))
        {
            case 1: // Filter Cutoff — scale by 3 octaves max
                if (slot < MAX_FILTERS)
                    lfoFltMod[slot] = val * filterSlots[slot].cutoff * 1.5f;
                break;
            case 2: // Osc Volume
                if (slot < MAX_SYNTHS)
                    lfoVolMod[slot] = val;
                break;
            case 3: // Osc Detune
                if (slot < MAX_SYNTHS)
                    lfoDetMod[slot] = val * 100.0f; // ±100 cents
                break;
            default: break;
        }
    }
}

//==============================================================================
void ViolentAudioProcessor::processMidi (const juce::MidiBuffer& midi)
{
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn (true))
            startVoice (findFreeVoice() < 0 ? findVoiceToSteal() : findFreeVoice(),
                        msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff (true))
            stopNote (msg.getNoteNumber());
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
            allNotesOff();
    }
}

int ViolentAudioProcessor::findFreeVoice() const noexcept
{
    for (int i = 0; i < MAX_VOICES; ++i)
        if (!voices[i].active) return i;
    return -1;
}

int ViolentAudioProcessor::findVoiceToSteal() const noexcept
{
    // Prefer a releasing voice; otherwise steal the one with the lowest note (oldest feel)
    for (int i = 0; i < MAX_VOICES; ++i)
        if (voices[i].active && !voices[i].isActive()) return i;
    return 0; // fallback: steal voice 0
}

void ViolentAudioProcessor::startVoice (int vi, int note, float velocity)
{
    if (vi < 0 || vi >= MAX_VOICES) return;
    auto& v      = voices[vi];
    v.note       = note;
    v.velocity   = velocity;
    v.baseFreqHz = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (note));
    v.active     = true;

    for (int s = 0; s < MAX_SYNTHS; ++s)
    {
        v.osc[s].reset();
        v.adsr[s].reset();
        if (synthSlots[s].en)
        {
            const auto& sl = synthSlots[s];
            v.adsr[s].setParams (sl.att, sl.dec, sl.sus, sl.rel);
            v.adsr[s].noteOn();
        }
    }

    for (int s = 0; s < MAX_SAMPLES; ++s)
    {
        v.smpPlayer[s].reset();
        v.smpAdsr[s].reset();
        if (sampleSlots[s].en && sampleModules[s].hasData)
        {
            const auto& sl = sampleSlots[s];
            v.smpAdsr[s].setParams (sl.att, sl.dec, sl.sus, sl.rel);
            v.smpAdsr[s].noteOn();
        }
    }
}

void ViolentAudioProcessor::stopNote (int note)
{
    for (auto& v : voices)
    {
        if (!v.active || v.note != note) continue;
        for (auto& a : v.adsr)    a.noteOff();
        for (auto& a : v.smpAdsr) a.noteOff();
    }
}

void ViolentAudioProcessor::allNotesOff()
{
    for (auto& v : voices) v.reset();
}

//==============================================================================
void ViolentAudioProcessor::renderSynth (juce::AudioBuffer<float>& buffer)
{
    const int   numSamples = buffer.getNumSamples();
    const float sr         = static_cast<float> (processSpec.sampleRate);
    // Per-voice normalization: equal-power mixing across synth layers
    const float norm = 1.0f / static_cast<float> (MAX_SYNTHS);

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : L;

    for (auto& v : voices)
    {
        if (!v.active) continue;

        // Update LFO modulation for this voice
        for (int s = 0; s < MAX_SYNTHS; ++s)
        {
            v.detuneModCents[s] = lfoDetMod[s];
            v.volumeModMult[s]  = lfoVolMod[s];
        }

        for (int n = 0; n < numSamples; ++n)
        {
            float sample = 0.0f;

            for (int s = 0; s < MAX_SYNTHS; ++s)
            {
                if (!synthSlots[s].en) continue;

                const auto& sl  = synthSlots[s];
                const float det = sl.detune + v.detuneModCents[s];
                const float osc = v.osc[s].tick (v.baseFreqHz, det, sr, sl.type);
                const float env = v.adsr[s].tick();
                const float vol = sl.gainLin * juce::jmax (0.0f, 1.0f + v.volumeModMult[s]);
                sample += osc * env * vol;
            }

            sample *= norm * v.velocity;
            L[n] += sample;
            R[n] += sample;
        }

        if (!v.isActive())
            v.active = false;
    }
}

void ViolentAudioProcessor::renderSamples (juce::AudioBuffer<float>& buffer)
{
    const int   numSamples = buffer.getNumSamples();
    const float norm       = 1.0f / static_cast<float> (MAX_SAMPLES);

    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : L;

    for (auto& v : voices)
    {
        if (!v.active) continue;

        for (int s = 0; s < MAX_SAMPLES; ++s)
        {
            if (!sampleSlots[s].en) continue;

            // Try-lock: skip this slot if message thread is loading a new sample
            juce::SpinLock::ScopedTryLockType tryLock (sampleModules[s].lock);
            if (!tryLock.isLocked() || !sampleModules[s].hasData) continue;

            const auto&   sm      = sampleModules[s];
            const auto&   sl      = sampleSlots[s];
            const double  pitchR  = std::pow (2.0, (v.note - sl.root) / 12.0);
            const bool    loop    = sl.loop;
            const int     bufLen  = sm.buffer.getNumSamples();

            for (int n = 0; n < numSamples; ++n)
            {
                if (v.smpPlayer[s].isFinished (bufLen, loop)) break;
                const float smpOut = v.smpPlayer[s].tick (sm.buffer, pitchR, loop);
                const float env    = v.smpAdsr[s].tick();
                const float out    = smpOut * env * sl.gainLin * v.velocity * norm;
                L[n] += out;
                R[n] += out;
            }
        }
    }
}

void ViolentAudioProcessor::loadSample (int slotIndex, const juce::File& file)
{
    if (slotIndex < 0 || slotIndex >= MAX_SAMPLES) return;

    std::unique_ptr<juce::AudioFormatReader> reader (
        formatManager.createReaderFor (file));

    if (reader == nullptr) return;

    // Read into a temp buffer on the message thread
    const int numCh      = static_cast<int> (reader->numChannels);
    const int numSamples = static_cast<int> (reader->lengthInSamples);

    juce::AudioBuffer<float> tempBuf (numCh, numSamples);
    reader->read (&tempBuf, 0, numSamples, 0, true, true);

    // Swap into the sample module under lock
    auto& sm = sampleModules[slotIndex];
    {
        juce::SpinLock::ScopedLockType lock (sm.lock);
        sm.buffer   = std::move (tempBuf);
        sm.filePath = file.getFullPathName();
        sm.hasData  = true;
    }
}

//==============================================================================
void ViolentAudioProcessor::applyFilters (juce::AudioBuffer<float>& buffer)
{
    const int   numSamples = buffer.getNumSamples();
    const float sr         = static_cast<float> (processSpec.sampleRate);
    const int   numCh      = buffer.getNumChannels();

    for (int fi = 0; fi < MAX_FILTERS; ++fi)
    {
        if (!filterSlots[fi].en) continue;

        const auto& f      = filterSlots[fi];
        const float cutoff = juce::jlimit (20.0f, sr * 0.49f,
                                           f.cutoff + lfoFltMod[fi]);

        for (int ch = 0; ch < numCh; ++ch)
        {
            float* data = buffer.getWritePointer (ch);
            for (int n = 0; n < numSamples; ++n)
                data[n] = filters[fi].process (ch, data[n], cutoff, f.res, sr, f.type);
        }
    }
}

//==============================================================================
void ViolentAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    if (paramsDirty.exchange (false))
        updateDSP();

    buffer.clear();

    // 1. Load param snapshots from APVTS (atomic reads, done once per block)
    loadParams();

    // 2. Advance LFOs for this block (block-rate modulation)
    computeLFOs (static_cast<float> (processSpec.sampleRate));

    // 3. Process MIDI events
    processMidi (midiMessages);

    // 4. Render synthesizer voices (oscillators)
    renderSynth (buffer);

    // 5. Render sample voices (pitched sample playback)
    renderSamples (buffer);

    // 6. Apply synth filters (in series)
    applyFilters (buffer);

    // 6. Run existing FX chain
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    if (apvts.getRawParameterValue (ParamIDs::EQ_ENABLED)->load() > 0.5f)
        for (auto& band : eqBands)
            band.process (ctx);

    if (apvts.getRawParameterValue (ParamIDs::DIST_ENABLED)->load() > 0.5f)
    {
        const float drive = apvts.getRawParameterValue (ParamIDs::DIST_DRIVE)->load();
        const float level = juce::Decibels::decibelsToGain (
                                apvts.getRawParameterValue (ParamIDs::DIST_LEVEL)->load());
        const int   type  = static_cast<int> (
                                apvts.getRawParameterValue (ParamIDs::DIST_TYPE)->load());

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            float* data = buffer.getWritePointer (ch);
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float x = data[s] * drive;
                switch (type)
                {
                    case 0: x = std::tanh (x); break;
                    case 1: x = juce::jlimit (-1.0f, 1.0f, x); break;
                    case 2:
                        x = std::tanh (x * 1.5f) + 0.1f * std::sin (x * juce::MathConstants<float>::pi);
                        x = juce::jlimit (-1.0f, 1.0f, x);
                        break;
                    default: x = std::tanh (x); break;
                }
                data[s] = x * level;
            }
        }
        distToneFilter.process (ctx);
    }

    if (apvts.getRawParameterValue (ParamIDs::COMP_ENABLED)->load() > 0.5f)
    {
        compressor.process (ctx);
        makeupGain.process (ctx);
    }

    if (apvts.getRawParameterValue (ParamIDs::GATE_ENABLED)->load() > 0.5f)
        noiseGate.process (ctx);

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
                                processSpec.sampleRate, static_cast<double> (toneHz));
}

void ViolentAudioProcessor::updateCompressor()
{
    if (!prepared) return;
    compressor.setThreshold (apvts.getRawParameterValue (ParamIDs::COMP_THRESHOLD)->load());
    compressor.setRatio      (apvts.getRawParameterValue (ParamIDs::COMP_RATIO)->load());
    compressor.setAttack     (apvts.getRawParameterValue (ParamIDs::COMP_ATTACK)->load());
    compressor.setRelease    (apvts.getRawParameterValue (ParamIDs::COMP_RELEASE)->load());
    makeupGain.setGainDecibels (apvts.getRawParameterValue (ParamIDs::COMP_MAKEUP)->load());
}

void ViolentAudioProcessor::updateGate()
{
    if (!prepared) return;
    noiseGate.setThreshold (apvts.getRawParameterValue (ParamIDs::GATE_THRESHOLD)->load());
    noiseGate.setRatio      (apvts.getRawParameterValue (ParamIDs::GATE_RATIO)->load());
    noiseGate.setAttack     (apvts.getRawParameterValue (ParamIDs::GATE_ATTACK)->load());
    noiseGate.setRelease    (apvts.getRawParameterValue (ParamIDs::GATE_RELEASE)->load());
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
    auto xml = apvts.copyState().createXml();
    if (xml == nullptr) return;

    // Persist sample file paths as child elements
    for (int i = 0; i < MAX_SAMPLES; ++i)
    {
        juce::SpinLock::ScopedTryLockType tryLock (sampleModules[i].lock);
        if (tryLock.isLocked() && sampleModules[i].hasData)
        {
            auto* node = xml->createNewChildElement ("SamplePath");
            node->setAttribute ("slot", i);
            node->setAttribute ("path", sampleModules[i].filePath);
        }
    }

    copyXmlToBinary (*xml, destData);
}

void ViolentAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr) return;

    if (xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));

    // Reload samples from stored paths
    for (auto* node : xml->getChildIterator())
    {
        if (node->hasTagName ("SamplePath"))
        {
            const int    slot = node->getIntAttribute ("slot");
            const juce::File file (node->getStringAttribute ("path"));
            if (file.existsAsFile())
                loadSample (slot, file);
        }
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViolentAudioProcessor();
}

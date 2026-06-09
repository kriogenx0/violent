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
            StringArray { "Sine", "Saw", "Square", "Triangle", "Noise" }, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthGain (i), "Synth " + String (i + 1) + " Gain",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::synthOct (i), "Synth " + String (i + 1) + " Octave", -3, 3, 0));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::synthSemi (i), "Synth " + String (i + 1) + " Semitone", -12, 12, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthDet (i), "Synth " + String (i + 1) + " Detune",
            NormalisableRange<float> (-100.0f, 100.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("ct")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthPhase (i), "Synth " + String (i + 1) + " Phase",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthPW (i), "Synth " + String (i + 1) + " Pulse Width",
            NormalisableRange<float> (0.05f, 0.95f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthPan (i), "Synth " + String (i + 1) + " Pan",
            NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthVelS (i), "Synth " + String (i + 1) + " Velocity Sensitivity",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
        params.push_back (std::make_unique<AudioParameterInt> (
            ParamIDs::synthUni (i), "Synth " + String (i + 1) + " Unison Voices", 1, 8, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::synthUniSpread (i), "Synth " + String (i + 1) + " Unison Spread",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 15.0f,
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
    params.push_back (std::make_unique<AudioParameterBool> (ParamIDs::EQ_ENABLED, "EQ Enabled", true));
    for (int i = 0; i < NUM_EQ_BANDS; ++i)
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::eqBand (i), "EQ Band " + String (i),
            NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));

    // ---- FX chain slots ----
    for (int i = 0; i < MAX_FX; ++i)
    {
        // type stored as int in XML, not as an APVTS param (UI writes it directly)
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxDrive    (i), "FX " + String (i) + " Drive",
            NormalisableRange<float> (1.0f, 100.0f, 0.1f, 0.4f), 1.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxTone     (i), "FX " + String (i) + " Tone",
            NormalisableRange<float> (200.0f, 8000.0f, 1.0f, 0.5f), 4000.0f,
            AudioParameterFloatAttributes().withLabel ("Hz")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxLevel    (i), "FX " + String (i) + " Level",
            NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterChoice> (
            ParamIDs::fxDistType (i), "FX " + String (i) + " Dist Type",
            StringArray { "Soft Clip", "Hard Clip", "Fuzz" }, 0));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxThresh   (i), "FX " + String (i) + " Threshold",
            NormalisableRange<float> (-60.0f, 0.0f, 0.1f), -12.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxRatio    (i), "FX " + String (i) + " Ratio",
            NormalisableRange<float> (1.0f, 20.0f, 0.1f, 0.5f), 4.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxAttack   (i), "FX " + String (i) + " Attack",
            NormalisableRange<float> (0.1f, 200.0f, 0.1f, 0.5f), 10.0f,
            AudioParameterFloatAttributes().withLabel ("ms")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxRelease  (i), "FX " + String (i) + " Release",
            NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.5f), 100.0f,
            AudioParameterFloatAttributes().withLabel ("ms")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxMakeup   (i), "FX " + String (i) + " Makeup",
            NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withLabel ("dB")));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxRoom     (i), "FX " + String (i) + " Room",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxDamping  (i), "FX " + String (i) + " Damping",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxWet      (i), "FX " + String (i) + " Wet",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.33f));
        params.push_back (std::make_unique<AudioParameterFloat> (
            ParamIDs::fxWidth    (i), "FX " + String (i) + " Width",
            NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));
    }

    return { params.begin(), params.end() };
}

//==============================================================================
ViolentAudioProcessor::ViolentAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    formatManager.registerBasicFormats();

    for (const auto& id : fxParamIDs())
        apvts.addParameterListener (id, this);
}

ViolentAudioProcessor::~ViolentAudioProcessor()
{
    for (const auto& id : fxParamIDs())
        apvts.removeParameterListener (id, this);
}

const juce::StringArray& ViolentAudioProcessor::fxParamIDs()
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
        setEQBand (i, 0.0f);

    for (auto& band : eqBands)
        band.prepare (processSpec);

    for (auto& slot : fxDSP)
        slot.prepare (processSpec);

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
        auto& s       = synthSlots[i];
        s.en          = (i < numActiveSynths);
        s.type        = static_cast<int> (apvts.getRawParameterValue (ParamIDs::synthType (i))->load());
        s.gainLin     = juce::Decibels::decibelsToGain (
                           apvts.getRawParameterValue (ParamIDs::synthGain (i))->load());
        s.octave      = static_cast<int> (apvts.getRawParameterValue (ParamIDs::synthOct  (i))->load());
        s.semitone    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::synthSemi (i))->load());
        s.detune      = apvts.getRawParameterValue (ParamIDs::synthDet  (i))->load();
        s.phase       = apvts.getRawParameterValue (ParamIDs::synthPhase(i))->load();
        s.pulseWidth  = apvts.getRawParameterValue (ParamIDs::synthPW   (i))->load();
        s.pan         = apvts.getRawParameterValue (ParamIDs::synthPan  (i))->load();
        s.velSens     = apvts.getRawParameterValue (ParamIDs::synthVelS (i))->load();
        s.unisonVoices= static_cast<int> (apvts.getRawParameterValue (ParamIDs::synthUni (i))->load());
        s.unisonSpread= apvts.getRawParameterValue (ParamIDs::synthUniSpread (i))->load();
        s.att         = apvts.getRawParameterValue (ParamIDs::synthAtt  (i))->load();
        s.dec         = apvts.getRawParameterValue (ParamIDs::synthDec  (i))->load();
        s.sus         = apvts.getRawParameterValue (ParamIDs::synthSus  (i))->load();
        s.rel         = apvts.getRawParameterValue (ParamIDs::synthRel  (i))->load();
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
        for (auto& u : v.osc[s]) u.reset();
        v.adsr[s].reset();
        if (synthSlots[s].en)
        {
            const auto& sl = synthSlots[s];
            // Set phase offset for each unison voice
            for (int u = 0; u < sl.unisonVoices; ++u)
                v.osc[s][u].reset (sl.phase);
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
            float sampleL = 0.0f, sampleR = 0.0f;

            for (int s = 0; s < MAX_SYNTHS; ++s)
            {
                if (!synthSlots[s].en) continue;

                const auto& sl       = synthSlots[s];
                const float pitchCents = sl.detune + v.detuneModCents[s]
                                       + (sl.octave * 12 + sl.semitone) * 100.0f;
                const float vol      = sl.gainLin
                                     * juce::jmax (0.0f, 1.0f + v.volumeModMult[s])
                                     * (1.0f - sl.velSens * (1.0f - v.velocity));
                const float env      = v.adsr[s].tick();

                // Unison: spread voices evenly across unisonSpread cents
                float uniSum = 0.0f;
                const int nUni = sl.unisonVoices;
                for (int u = 0; u < nUni; ++u)
                {
                    const float uSpread = nUni > 1
                        ? sl.unisonSpread * (u / (float)(nUni - 1) - 0.5f) * 2.0f
                        : 0.0f;
                    uniSum += v.osc[s][u].tick (v.baseFreqHz, pitchCents + uSpread,
                                                sr, sl.type, sl.pulseWidth);
                }
                uniSum /= static_cast<float> (nUni);

                const float out  = uniSum * env * vol;
                const float panL = juce::jlimit (0.0f, 1.0f, 1.0f - sl.pan);
                const float panR = juce::jlimit (0.0f, 1.0f, 1.0f + sl.pan);
                sampleL += out * panL;
                sampleR += out * panR;
            }

            sampleL *= norm * v.velocity;
            sampleR *= norm * v.velocity;
            L[n] += sampleL;
            R[n] += sampleR;
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
        filters[fi].setCoefficients (cutoff, f.res, sr);

        for (int ch = 0; ch < numCh; ++ch)
        {
            float* data = buffer.getWritePointer (ch);
            for (int n = 0; n < numSamples; ++n)
                data[n] = filters[fi].process (ch, data[n], f.type);
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

    // 1. Load param snapshots from APVTS (atomic reads, done once per block)
    loadParams();

    // 2. Advance LFOs for this block (block-rate modulation)
    computeLFOs (static_cast<float> (processSpec.sampleRate));

    // 3. Process MIDI events
    processMidi (midiMessages);

    // 4. Render synthesizer voices (oscillators)
    renderSynth (buffer);

    // 5. Render sample voices
    renderSamples (buffer);

    // 6. Apply synth filters
    applyFilters (buffer);

    // 7. EQ
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        eqEnabled = apvts.getRawParameterValue (ParamIDs::EQ_ENABLED)->load() > 0.5f;
        if (eqEnabled)
            for (auto& band : eqBands)
                band.process (ctx);
    }

    // 8. FX chain (dynamic, ordered)
    applyFxChain (buffer);

    // 9. Level metering
    levelL.store (buffer.getMagnitude (0, 0, buffer.getNumSamples()), std::memory_order_relaxed);
    levelR.store (buffer.getNumChannels() > 1
                      ? buffer.getMagnitude (1, 0, buffer.getNumSamples())
                      : levelL.load (std::memory_order_relaxed),
                  std::memory_order_relaxed);
}

//==============================================================================
void ViolentAudioProcessor::applyFxChain (juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    for (int i = 0; i < numActiveFx; ++i)
    {
        auto& dsp = fxDSP[(size_t) i];

        switch (fxChain[(size_t) i])
        {
            case FxType::Distortion:
            {
                const float drive = apvts.getRawParameterValue (ParamIDs::fxDrive (i))->load();
                const float level = juce::Decibels::decibelsToGain (
                                        apvts.getRawParameterValue (ParamIDs::fxLevel (i))->load());
                const int   type  = static_cast<int> (
                                        apvts.getRawParameterValue (ParamIDs::fxDistType (i))->load());
                const float tone  = apvts.getRawParameterValue (ParamIDs::fxTone (i))->load();

                *dsp.distToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                    processSpec.sampleRate, static_cast<double> (tone));

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
                dsp.distToneFilter.process (ctx);
                break;
            }

            case FxType::Compressor:
            {
                dsp.compressor.setThreshold (apvts.getRawParameterValue (ParamIDs::fxThresh  (i))->load());
                dsp.compressor.setRatio     (apvts.getRawParameterValue (ParamIDs::fxRatio   (i))->load());
                dsp.compressor.setAttack    (apvts.getRawParameterValue (ParamIDs::fxAttack  (i))->load());
                dsp.compressor.setRelease   (apvts.getRawParameterValue (ParamIDs::fxRelease (i))->load());
                dsp.makeup.setGainDecibels  (apvts.getRawParameterValue (ParamIDs::fxMakeup  (i))->load());
                dsp.compressor.process (ctx);
                dsp.makeup.process (ctx);
                break;
            }

            case FxType::Gate:
            {
                dsp.gate.setThreshold (apvts.getRawParameterValue (ParamIDs::fxThresh  (i))->load());
                dsp.gate.setRatio     (apvts.getRawParameterValue (ParamIDs::fxRatio   (i))->load());
                dsp.gate.setAttack    (apvts.getRawParameterValue (ParamIDs::fxAttack  (i))->load());
                dsp.gate.setRelease   (apvts.getRawParameterValue (ParamIDs::fxRelease (i))->load());
                dsp.gate.process (ctx);
                break;
            }

            case FxType::Reverb:
            {
                juce::dsp::Reverb::Parameters p;
                p.roomSize   = apvts.getRawParameterValue (ParamIDs::fxRoom    (i))->load();
                p.damping    = apvts.getRawParameterValue (ParamIDs::fxDamping (i))->load();
                p.wetLevel   = apvts.getRawParameterValue (ParamIDs::fxWet     (i))->load();
                p.dryLevel   = 1.0f - p.wetLevel;
                p.width      = apvts.getRawParameterValue (ParamIDs::fxWidth   (i))->load();
                p.freezeMode = 0.0f;
                dsp.reverb.setParameters (p);
                dsp.reverb.process (ctx);
                break;
            }

            default: break;
        }
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

    xml->setAttribute ("numActiveSynths", numActiveSynths);
    xml->setAttribute ("numActiveFx",     numActiveFx);
    for (int i = 0; i < MAX_FX; ++i)
        xml->setAttribute ("fxType_" + juce::String (i), static_cast<int> (fxChain[(size_t) i]));

    copyXmlToBinary (*xml, destData);
}

void ViolentAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr) return;

    if (xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));

    numActiveSynths = juce::jlimit (1, MAX_SYNTHS, xml->getIntAttribute ("numActiveSynths", 1));
    numActiveFx     = juce::jlimit (0, MAX_FX,     xml->getIntAttribute ("numActiveFx", 0));
    for (int i = 0; i < MAX_FX; ++i)
        fxChain[(size_t) i] = static_cast<FxType> (
            juce::jlimit (0, NUM_FX_TYPES - 1,
                          xml->getIntAttribute ("fxType_" + juce::String (i), 0)));

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

#pragma once
#include <JuceHeader.h>
#include "FxChain.h"
#include "GeneratorChain.h"
#include "SoundProcessing/Synth.h"
#include "SoundProcessing/Sampler.h"
#include "SoundProcessing/MidiModifier.h"
#include "SoundProcessing/Filter.h"
#include "SoundProcessing/Effects.h"

static constexpr int NUM_EQ_BANDS = 10;

//==============================================================================
namespace ParamIDs
{
    // ---------- Legacy sample/LFO slots (kept for sample player) ----------
    inline juce::String smpEn   (int n) { return "smp_" + juce::String (n) + "_en"; }
    inline juce::String smpRoot (int n) { return "smp_" + juce::String (n) + "_root"; }
    inline juce::String smpGain (int n) { return "smp_" + juce::String (n) + "_gain"; }
    inline juce::String smpAtt  (int n) { return "smp_" + juce::String (n) + "_att"; }
    inline juce::String smpDec  (int n) { return "smp_" + juce::String (n) + "_dec"; }
    inline juce::String smpSus  (int n) { return "smp_" + juce::String (n) + "_sus"; }
    inline juce::String smpRel  (int n) { return "smp_" + juce::String (n) + "_rel"; }
    inline juce::String smpLoop (int n) { return "smp_" + juce::String (n) + "_loop"; }

    inline juce::String lfoEn     (int n) { return "lfo_" + juce::String (n) + "_en"; }
    inline juce::String lfoShape  (int n) { return "lfo_" + juce::String (n) + "_shp"; }
    inline juce::String lfoRate   (int n) { return "lfo_" + juce::String (n) + "_rate"; }
    inline juce::String lfoDepth  (int n) { return "lfo_" + juce::String (n) + "_dep"; }
    inline juce::String lfoTarget (int n) { return "lfo_" + juce::String (n) + "_tgt"; }
    inline juce::String lfoTgtSlt (int n) { return "lfo_" + juce::String (n) + "_tslot"; }

    // ---------- EQ ----------
    static constexpr auto EQ_ENABLED = "eq_enabled";
    inline juce::String eqBand (int i) { return "eq_band_" + juce::String (i); }
    static constexpr auto EQ_BAND_0 = "eq_band_0";
    static constexpr auto EQ_BAND_1 = "eq_band_1";
    static constexpr auto EQ_BAND_2 = "eq_band_2";
    static constexpr auto EQ_BAND_3 = "eq_band_3";
    static constexpr auto EQ_BAND_4 = "eq_band_4";
    static constexpr auto EQ_BAND_5 = "eq_band_5";
    static constexpr auto EQ_BAND_6 = "eq_band_6";
    static constexpr auto EQ_BAND_7 = "eq_band_7";
    static constexpr auto EQ_BAND_8 = "eq_band_8";
    static constexpr auto EQ_BAND_9 = "eq_band_9";
}

//==============================================================================
class ViolentAudioProcessor : public juce::AudioProcessor,
                               public juce::AudioProcessorValueTreeState::Listener
{
public:
    ViolentAudioProcessor();
    ~ViolentAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged (const juce::String& parameterID, float newValue) override;

    // -----------------------------------------------------------------------
    // Presets
    // -----------------------------------------------------------------------
    std::unique_ptr<juce::XmlElement> createStateXml();
    void restoreStateFromXml (const juce::XmlElement& xml);

    static juce::File getPresetsDirectory();
    juce::StringArray getPresetNames() const;
    bool savePreset (const juce::String& name);
    bool loadPreset (const juce::String& name);

    juce::String currentPresetName;

    juce::AudioProcessorValueTreeState apvts;

    // -----------------------------------------------------------------------
    // Generator state — owned here, written by UI, read by audio thread
    // -----------------------------------------------------------------------
    int numActiveGenerators = 1;

    struct GeneratorState
    {
        bool  enabled = true;
        juce::String name; // editable; UI defaults it to "Synth N"/"Sampler N"
    };
    std::array<GeneratorState, MAX_GENERATORS> generators;

    // -----------------------------------------------------------------------
    // MIDI Modifier Components — a shared pool, each one routed to whichever
    // generators it applies to (like Effects below), rather than being
    // owned by a single generator.
    // -----------------------------------------------------------------------
    struct MidiModifierComponent
    {
        juce::String name;
        MidiModType type = MidiModType::PitchShift;
        bool enabled = true;
        std::array<bool, MAX_GENERATORS> routing {};
    };
    int numMidiModifiers = 0;
    std::array<MidiModifierComponent, MAX_MIDI_MODIFIERS> midiModifiers;

    // -----------------------------------------------------------------------
    // Effect Components (Filter is just one selectable type, not a separate
    // concept) — a shared pool; each one sums whichever generators are
    // routed to it, processes that sum, and mixes the result additively into
    // the master bus. Generators not routed to any effect mix straight
    // through, unfiltered/unprocessed.
    // -----------------------------------------------------------------------
    struct EffectComponent
    {
        juce::String name;
        FxType type = FxType::None;
        bool enabled = true;
        std::array<bool, MAX_GENERATORS> routing {};
    };
    int numEffects = 0;
    std::array<EffectComponent, MAX_EFFECTS> effects;

    // --- Level metering ---
    std::atomic<float> levelL { 0.0f }, levelR { 0.0f };
    std::array<std::atomic<float>, MAX_GENERATORS> generatorLevelMeter {};

    // --- Live waveform (raw source output, before effects, for the UI scope) ---
    // Each generator's raw output rolls continuously into a ring buffer sized
    // for the largest option below; the UI reads back however many of the
    // most recent samples its own time-window setting calls for, so a single
    // capture serves every zoom level without the processor knowing which
    // one is currently selected.
    static constexpr int NUM_WAVEFORM_WINDOWS = 8;
    static constexpr std::array<float, NUM_WAVEFORM_WINDOWS> waveformWindowOptionsMs
        { 5.0f, 10.0f, 25.0f, 50.0f, 100.0f, 250.0f, 500.0f, 1000.0f };

    std::array<std::vector<float>, MAX_GENERATORS> waveformRing;
    std::array<int, MAX_GENERATORS> waveformRingWritePos {};
    int waveformRingSize = 0;

    // --- MIDI preview (plays a demo pattern so the synth can be heard without a keyboard) ---
    enum class PreviewPattern { Arpeggios = 0, LowNotes, LongSingleNotes, Chords };
    std::atomic<bool> previewActive { false };
    std::atomic<int>  previewPattern { (int) PreviewPattern::Arpeggios };
    void setPreviewPattern (int pattern);

    void loadSample (int slotIndex, const juce::File& file);

    // Randomizes every active generator's source/synth settings, plus the
    // params of every existing Effect Component — keeping each one's chosen
    // type as-is and only randomizing the sub-params that type actually
    // uses. Doesn't touch structure (counts, types, or routing) or MIDI
    // modifiers/EQ.
    void randomizeAll();

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static const juce::StringArray& eqParamIDs();
    void setEQBand (int i, float gainDB);

    using StereoFilter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    // -----------------------------------------------------------------------
    // DSP state — one copy per generator. The individual signal-processor
    // types (OscSlot/FxSlotDSP/...) live under Source/SoundProcessing/; this
    // struct is just the per-generator bookkeeping owned by the processor
    // as the orchestrator. Generators no longer own any FX of their own —
    // all effects processing happens in processEffects() below.
    // -----------------------------------------------------------------------
    struct GeneratorDSP
    {
        OscSlot osc;
        float level = 1.0f, pan = 0.0f;
        juce::AudioBuffer<float> scratch;
        bool prepared = false;

        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            scratch.setSize (2, (int) spec.maximumBlockSize, false, true, true);
            prepared = true;
        }
    };

    std::array<GeneratorDSP, MAX_GENERATORS> generatorDSP;

    SynthEngine   synthEngine;
    SamplerEngine sampler;

    // -----------------------------------------------------------------------
    // Internal methods
    // -----------------------------------------------------------------------
    void loadGeneratorParams (int s);
    void processMidi (const juce::MidiBuffer&);
    void renderGenerator (int s, juce::AudioBuffer<float>& master);

    // Sums whichever generators are routed to each Effect Component,
    // processes that sum through the effect's DSP, and mixes the result
    // additively into master; generators claimed by no effect mix straight
    // through. Replaces what used to be three separate mechanisms
    // (per-generator FX chains, shared FX buses, and master filters).
    void processEffects (juce::AudioBuffer<float>& master, int numSamples);

    std::array<FxSlotDSP, MAX_EFFECTS> effectDSP;
    juce::AudioBuffer<float> effectScratch;

public:
    // -----------------------------------------------------------------------
    // Per-generator MIDI modifier application — transpose/octave/key-quantize
    // are plain deterministic transforms recomputed on both note-on and
    // note-off; the arpeggiator is its own tiny sample-accurate step
    // sequencer per (modifier, generator) pair, driven from held
    // (already-transformed) notes. Exposed publicly (rather than just
    // private) since they're pure, const queries that unit tests exercise
    // directly.
    int  applyMidiModifier (int generatorIdx, int note) const;
    bool isArpEnabled (int generatorIdx) const;

private:
    void renderMidiModifiers (int numSamples);

    std::array<std::array<MidiModifierState, MAX_GENERATORS>, MAX_MIDI_MODIFIERS> midiModState;

    // -----------------------------------------------------------------------
    // MIDI preview sequencer — loops one of a few demo patterns (see
    // PreviewPattern) so the synth can be heard without a MIDI keyboard.
    // Each step is a set of notes so chords can fire together.
    // -----------------------------------------------------------------------
    void buildPreviewSequence();
    void renderPreviewMidi (juce::MidiBuffer& midi, int numSamples);

    std::vector<std::vector<int>> previewSequence;
    int  previewStepIndex      = -1;
    int  previewSamplePos      = 0;
    int  previewNextEventSample = 0;
    bool previewNoteIsOn       = false;
    int  previewSamplesPerNote = 0;
    int  previewGateSamples    = 0;

    // -----------------------------------------------------------------------
    // Global EQ (master bus)
    // -----------------------------------------------------------------------
    static constexpr float EQ_FREQUENCIES[NUM_EQ_BANDS] = {
        31.25f, 62.5f, 125.0f, 250.0f, 500.0f,
        1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f
    };
    static constexpr float EQ_Q = 1.41421356f;

    std::array<StereoFilter, NUM_EQ_BANDS> eqBands;
    bool eqEnabled = false;
    void updateEQ();

    juce::dsp::ProcessSpec processSpec { 44100.0, 512, 2 };
    bool prepared = false;
    std::atomic<bool> paramsDirty { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessor)
};

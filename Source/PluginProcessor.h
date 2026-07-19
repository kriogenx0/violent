#pragma once
#include <JuceHeader.h>
#include "SynthEngine.h"
#include "FxChain.h"
#include "GeneratorChain.h"

static constexpr int NUM_EQ_BANDS = 10;
static constexpr int MAX_MASTER_FILTERS = 4;

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

    // ---------- Master filters (applied last, after all generators) ----------
    inline juce::String masterFltType (int f) { return "mflt_" + juce::String (f) + "_type"; }
    inline juce::String masterFltCut  (int f) { return "mflt_" + juce::String (f) + "_cut"; }
    inline juce::String masterFltRes  (int f) { return "mflt_" + juce::String (f) + "_res"; }
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
        bool  enabled      = true;
        int   numFilters   = 0;
        int   numFx        = 0;
        std::array<FxType, MAX_GENERATOR_FX> fxTypes {};
    };
    std::array<GeneratorState, MAX_GENERATORS> generators;

    // -----------------------------------------------------------------------
    // Master filters — applied last, after every generator's own filters/FX.
    // Each one sums a chosen subset of generators, filters that sum, and
    // adds it to the master bus; generators not routed to any master filter
    // are mixed straight through, unfiltered.
    // -----------------------------------------------------------------------
    struct MasterFilterState
    {
        bool enabled = true;
        std::array<bool, MAX_GENERATORS> routing {};
    };
    int numMasterFilters = 0;
    std::array<MasterFilterState, MAX_MASTER_FILTERS> masterFilters;

    // --- Level metering ---
    std::atomic<float> levelL { 0.0f }, levelR { 0.0f };
    std::array<std::atomic<float>, MAX_GENERATORS> generatorLevelMeter {};

    // --- Live waveform (raw source output, before filters/FX, for the UI scope) ---
    static constexpr int WAVEFORM_SAMPLES = 256;
    std::array<std::array<float, WAVEFORM_SAMPLES>, MAX_GENERATORS> waveformSnapshot {};

    // --- MIDI preview (plays a demo pattern so the synth can be heard without a keyboard) ---
    enum class PreviewPattern { Arpeggios = 0, LowNotes, LongSingleNotes, Chords };
    std::atomic<bool> previewActive { false };
    std::atomic<int>  previewPattern { (int) PreviewPattern::Arpeggios };
    void setPreviewPattern (int pattern);

    // --- Sample modules (kept for future sample-player generators) ---
    struct SampleModule
    {
        juce::AudioBuffer<float> buffer;
        juce::String             filePath;
        bool                     hasData = false;
        juce::SpinLock           lock;

        SampleModule() = default;
        JUCE_DECLARE_NON_COPYABLE (SampleModule)
    };

    std::array<SampleModule, MAX_SAMPLES> sampleModules;
    juce::AudioFormatManager             formatManager;

    void loadSample (int slotIndex, const juce::File& file);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    static const juce::StringArray& eqParamIDs();
    void setEQBand (int i, float gainDB);

    using StereoFilter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    // -----------------------------------------------------------------------
    // DSP state — one copy per generator
    // -----------------------------------------------------------------------
    struct OscSlot
    {
        bool  en = false;
        int   type = 1, octave = 0, semitone = 0, unisonVoices = 1;
        float gainLin = 1.0f, detune = 0.0f, phase = 0.0f, pulseWidth = 0.5f;
        float pan = 0.0f, velSens = 1.0f, unisonSpread = 15.0f;
        float att = 0.01f, dec = 0.1f, sus = 0.7f, rel = 0.3f;
    };

    struct FltSlot
    {
        bool  en = false;
        int   type = 0;
        float cutoff = 8000.0f, res = 0.707f;
    };

    // Per-FX-slot DSP objects (one instance per active FX slot in a generator)
    struct FxSlotDSP
    {
        juce::dsp::Compressor<float>  compressor;
        juce::dsp::Gain<float>        makeup;
        juce::dsp::NoiseGate<float>   gate;
        juce::dsp::Reverb             reverb;
        StereoFilter                  distToneFilter;
        bool prepared = false;

        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            compressor.prepare (spec);
            makeup.prepare (spec);
            gate.prepare (spec);
            reverb.prepare (spec);
            distToneFilter.prepare (spec);
            *distToneFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                spec.sampleRate, 4000.0);
            prepared = true;
        }
    };

    struct GeneratorDSP
    {
        OscSlot osc;
        std::array<FltSlot,  MAX_GENERATOR_FILTERS> filters {};
        float level = 1.0f, pan = 0.0f;

        // Per-generator DSP objects
        std::array<SVFilter,  MAX_GENERATOR_FILTERS> svFilters;
        std::array<FxSlotDSP, MAX_GENERATOR_FX>      fxDSP;
        juce::AudioBuffer<float>                   scratch;
        bool prepared = false;

        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            scratch.setSize (2, (int) spec.maximumBlockSize, false, true, true);
            for (auto& f : svFilters) f.reset();
            for (auto& fx : fxDSP)   fx.prepare (spec);
            prepared = true;
        }
    };

    std::array<GeneratorDSP, MAX_GENERATORS> generatorDSP;

    // -----------------------------------------------------------------------
    // Voice pool — tagged by generator
    // -----------------------------------------------------------------------
    struct GeneratorVoice
    {
        bool  active     = false;
        int   note       = -1;
        int   generatorIdx  = 0;
        float velocity   = 1.0f;
        float baseFreqHz = 0.0f;

        OscillatorGen osc[MAX_UNISON];
        ADSREnv       adsr;

        void prepare (double sr) noexcept { adsr.setSampleRate (sr); }
        void reset()  noexcept
        {
            active = false; note = -1;
            for (auto& o : osc) o.reset();
            adsr.reset();
        }
        bool isActive() const noexcept { return adsr.isActive(); }
    };

    static constexpr int GENERATOR_VOICES = 16;
    std::array<GeneratorVoice, GENERATOR_VOICES> voices;

    // -----------------------------------------------------------------------
    // Internal methods
    // -----------------------------------------------------------------------
    void loadGeneratorParams (int s);
    void processMidi (const juce::MidiBuffer&);
    void renderGenerator (int s, juce::AudioBuffer<float>& master);
    void applyGeneratorFilters (GeneratorDSP& dsp, const GeneratorState& gen, juce::AudioBuffer<float>& buf);
    void applyGeneratorFx (int s, GeneratorDSP& dsp, const GeneratorState& gen, juce::AudioBuffer<float>& buf);
    void mixGeneratorsToMaster (juce::AudioBuffer<float>& master);

    // Master filters use JUCE's TPT state-variable filter — unconditionally
    // stable at any cutoff/sample-rate ratio, unlike the naive Chamberlin SVF
    // used per-generator (which can diverge once cutoff approaches ~0.18x
    // the sample rate). "Notch" is synthesised as lowpass + highpass.
    struct MasterFilterDSP
    {
        juce::dsp::StateVariableTPTFilter<float> filter, notchHelper;

        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            filter.prepare (spec);
            notchHelper.prepare (spec);
        }

        void reset()
        {
            filter.reset();
            notchHelper.reset();
        }
    };
    std::array<MasterFilterDSP, MAX_MASTER_FILTERS> masterFilterDSP;
    juce::AudioBuffer<float> masterFilterScratch;

    int  findFreeVoice  (int generatorIdx) const noexcept;
    int  findVoiceToSteal (int generatorIdx) const noexcept;
    void startVoice (int vi, int note, float velocity, int generatorIdx);
    void stopNote   (int note);
    void stopNoteForGenerator (int generatorIdx, int note);
    void allNotesOff();

public:
    // -----------------------------------------------------------------------
    // Per-generator MIDI modifier — transpose/octave/key-quantize are plain
    // deterministic transforms recomputed on both note-on and note-off; the
    // arpeggiator is its own tiny sample-accurate step sequencer per
    // generator, driven from held (already-transformed) notes. Exposed
    // publicly (rather than just private) since they're pure, const queries
    // that unit tests exercise directly.
    int  applyMidiModifier (int generatorIdx, int note) const;
    bool isArpEnabled (int generatorIdx) const;

private:
    void renderMidiModifiers (int numSamples);

    struct MidiModifierState
    {
        std::vector<int> heldNotes;
        int  arpStepIndex      = -1;
        int  arpSamplePos      = 0;
        int  arpNextEventSample = 0;
        bool arpNoteIsOn       = false;
        int  arpCurrentNote    = -1;
    };
    std::array<MidiModifierState, MAX_GENERATORS> midiModState;

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

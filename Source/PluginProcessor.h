#pragma once
#include <JuceHeader.h>
#include "SynthEngine.h"

static constexpr int NUM_EQ_BANDS = 10;
static constexpr int MAX_FX       = 8;

//==============================================================================
namespace ParamIDs
{
    // ---------- Synth slots ----------
    inline juce::String synthEn        (int n) { return "synth_" + juce::String (n) + "_en"; }
    inline juce::String synthType      (int n) { return "synth_" + juce::String (n) + "_type"; }
    inline juce::String synthGain      (int n) { return "synth_" + juce::String (n) + "_gain"; }
    inline juce::String synthOct       (int n) { return "synth_" + juce::String (n) + "_oct"; }
    inline juce::String synthSemi      (int n) { return "synth_" + juce::String (n) + "_semi"; }
    inline juce::String synthDet       (int n) { return "synth_" + juce::String (n) + "_det"; }
    inline juce::String synthPhase     (int n) { return "synth_" + juce::String (n) + "_phase"; }
    inline juce::String synthPW        (int n) { return "synth_" + juce::String (n) + "_pw"; }
    inline juce::String synthPan       (int n) { return "synth_" + juce::String (n) + "_pan"; }
    inline juce::String synthVelS      (int n) { return "synth_" + juce::String (n) + "_vels"; }
    inline juce::String synthUni       (int n) { return "synth_" + juce::String (n) + "_uni"; }
    inline juce::String synthUniSpread (int n) { return "synth_" + juce::String (n) + "_unispread"; }
    inline juce::String synthAtt       (int n) { return "synth_" + juce::String (n) + "_att"; }
    inline juce::String synthDec       (int n) { return "synth_" + juce::String (n) + "_dec"; }
    inline juce::String synthSus       (int n) { return "synth_" + juce::String (n) + "_sus"; }
    inline juce::String synthRel       (int n) { return "synth_" + juce::String (n) + "_rel"; }

    // ---------- Sample slots ----------
    inline juce::String smpEn   (int n) { return "smp_" + juce::String (n) + "_en"; }
    inline juce::String smpRoot (int n) { return "smp_" + juce::String (n) + "_root"; }
    inline juce::String smpGain (int n) { return "smp_" + juce::String (n) + "_gain"; }
    inline juce::String smpAtt  (int n) { return "smp_" + juce::String (n) + "_att"; }
    inline juce::String smpDec  (int n) { return "smp_" + juce::String (n) + "_dec"; }
    inline juce::String smpSus  (int n) { return "smp_" + juce::String (n) + "_sus"; }
    inline juce::String smpRel  (int n) { return "smp_" + juce::String (n) + "_rel"; }
    inline juce::String smpLoop (int n) { return "smp_" + juce::String (n) + "_loop"; }

    // ---------- Filter slots ----------
    inline juce::String fltEn   (int n) { return "flt_" + juce::String (n) + "_en"; }
    inline juce::String fltType (int n) { return "flt_" + juce::String (n) + "_type"; }
    inline juce::String fltCut  (int n) { return "flt_" + juce::String (n) + "_cut"; }
    inline juce::String fltRes  (int n) { return "flt_" + juce::String (n) + "_res"; }

    // ---------- LFO slots ----------
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

    // ---------- FX chain slots (n = 0..MAX_FX-1) ----------
    inline juce::String fxType      (int n) { return "fx_" + juce::String (n) + "_type"; }
    inline juce::String fxDrive     (int n) { return "fx_" + juce::String (n) + "_drive"; }
    inline juce::String fxTone      (int n) { return "fx_" + juce::String (n) + "_tone"; }
    inline juce::String fxLevel     (int n) { return "fx_" + juce::String (n) + "_level"; }
    inline juce::String fxDistType  (int n) { return "fx_" + juce::String (n) + "_dtype"; }
    inline juce::String fxThresh    (int n) { return "fx_" + juce::String (n) + "_thresh"; }
    inline juce::String fxRatio     (int n) { return "fx_" + juce::String (n) + "_ratio"; }
    inline juce::String fxAttack    (int n) { return "fx_" + juce::String (n) + "_attack"; }
    inline juce::String fxRelease   (int n) { return "fx_" + juce::String (n) + "_release"; }
    inline juce::String fxMakeup    (int n) { return "fx_" + juce::String (n) + "_makeup"; }
    inline juce::String fxRoom      (int n) { return "fx_" + juce::String (n) + "_room"; }
    inline juce::String fxDamping   (int n) { return "fx_" + juce::String (n) + "_damp"; }
    inline juce::String fxWet       (int n) { return "fx_" + juce::String (n) + "_wet"; }
    inline juce::String fxWidth     (int n) { return "fx_" + juce::String (n) + "_width"; }
}

//==============================================================================
enum class FxType { None = 0, Distortion, Compressor, Gate, Reverb, Chorus, Delay };
static constexpr int NUM_FX_TYPES = 7;
inline const char* fxTypeName (FxType t)
{
    switch (t) {
        case FxType::Distortion: return "Distortion";
        case FxType::Compressor: return "Compressor";
        case FxType::Gate:       return "Gate";
        case FxType::Reverb:     return "Reverb";
        case FxType::Chorus:     return "Chorus";
        case FxType::Delay:      return "Delay";
        default:                 return "None";
    }
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

    juce::AudioProcessorValueTreeState apvts;

    // --- Active slot counts (persisted in state) ---
    int numActiveSynths = 1;
    int numActiveFx     = 0;

    // --- FX chain types (set by UI, read by DSP) ---
    std::array<FxType, MAX_FX> fxChain {};   // all None by default

    // --- Level metering (written by audio thread, read by UI timer) ---
    std::atomic<float> levelL { 0.0f }, levelR { 0.0f };

    // --- Sample modules ---
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
    static const juce::StringArray& fxParamIDs();
    void setEQBand (int i, float gainDB);

    // --- Synth engine ---
    std::array<Voice,     MAX_VOICES>  voices;
    std::array<SVFilter,  MAX_FILTERS> filters;
    std::array<LFOGen,    MAX_LFOS>    lfos;

    struct SynthSlot  { bool en; int type, octave, semitone, unisonVoices;
                        float gainLin, detune, phase, pulseWidth, pan, velSens, unisonSpread,
                              att, dec, sus, rel; };
    struct FilterSlot { bool en; int type; float cutoff, res; };
    struct LFOSlot    { bool en; int shape; float rate, depth, target, tgtSlot; };
    struct SampleSlot { bool en, loop; int root; float gainLin, att, dec, sus, rel; };

    std::array<SynthSlot,  MAX_SYNTHS>  synthSlots;
    std::array<FilterSlot, MAX_FILTERS> filterSlots;
    std::array<LFOSlot,    MAX_LFOS>    lfoSlots;
    std::array<SampleSlot, MAX_SAMPLES> sampleSlots;

    std::array<float, MAX_FILTERS> lfoFltMod {};
    std::array<float, MAX_SYNTHS>  lfoVolMod {};
    std::array<float, MAX_SYNTHS>  lfoDetMod {};

    void loadParams();
    void computeLFOs (float sampleRate);
    void processMidi (const juce::MidiBuffer&);
    void renderSynth (juce::AudioBuffer<float>&);
    void renderSamples (juce::AudioBuffer<float>&);
    void applyFilters (juce::AudioBuffer<float>&);
    void applyFxChain (juce::AudioBuffer<float>&);

    int  findFreeVoice()    const noexcept;
    int  findVoiceToSteal() const noexcept;
    void startVoice (int vi, int note, float velocity);
    void stopNote   (int note);
    void allNotesOff();

    // --- EQ ---
    void updateEQ();
    static constexpr float EQ_FREQUENCIES[NUM_EQ_BANDS] = {
        31.25f, 62.5f, 125.0f, 250.0f, 500.0f,
        1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f
    };
    static constexpr float EQ_Q = 1.41421356f;

    using StereoFilter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    std::array<StereoFilter, NUM_EQ_BANDS> eqBands;
    bool eqEnabled = false;

    // --- FX chain DSP objects (pre-allocated, used based on fxChain type) ---
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
    std::array<FxSlotDSP, MAX_FX> fxDSP;

    juce::dsp::ProcessSpec processSpec { 44100.0, 512, 2 };
    bool prepared = false;
    std::atomic<bool> paramsDirty { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessor)
};

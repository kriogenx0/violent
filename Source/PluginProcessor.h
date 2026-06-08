#pragma once
#include <JuceHeader.h>
#include "SynthEngine.h"

static constexpr int NUM_EQ_BANDS = 10;

//==============================================================================
namespace ParamIDs
{
    // ---------- Synth slots (n = 0..MAX_SYNTHS-1) ----------
    inline juce::String synthEn  (int n) { return "synth_" + juce::String (n) + "_en"; }
    inline juce::String synthType(int n) { return "synth_" + juce::String (n) + "_type"; }
    inline juce::String synthGain(int n) { return "synth_" + juce::String (n) + "_gain"; }
    inline juce::String synthDet (int n) { return "synth_" + juce::String (n) + "_det"; }
    inline juce::String synthAtt (int n) { return "synth_" + juce::String (n) + "_att"; }
    inline juce::String synthDec (int n) { return "synth_" + juce::String (n) + "_dec"; }
    inline juce::String synthSus (int n) { return "synth_" + juce::String (n) + "_sus"; }
    inline juce::String synthRel (int n) { return "synth_" + juce::String (n) + "_rel"; }

    // ---------- Sample slots (n = 0..MAX_SAMPLES-1) ----------
    inline juce::String smpEn  (int n) { return "smp_" + juce::String (n) + "_en"; }
    inline juce::String smpRoot(int n) { return "smp_" + juce::String (n) + "_root"; }
    inline juce::String smpGain(int n) { return "smp_" + juce::String (n) + "_gain"; }
    inline juce::String smpAtt (int n) { return "smp_" + juce::String (n) + "_att"; }
    inline juce::String smpDec (int n) { return "smp_" + juce::String (n) + "_dec"; }
    inline juce::String smpSus (int n) { return "smp_" + juce::String (n) + "_sus"; }
    inline juce::String smpRel (int n) { return "smp_" + juce::String (n) + "_rel"; }
    inline juce::String smpLoop(int n) { return "smp_" + juce::String (n) + "_loop"; }

    // ---------- Filter slots (n = 0..MAX_FILTERS-1) ----------
    inline juce::String fltEn  (int n) { return "flt_" + juce::String (n) + "_en"; }
    inline juce::String fltType(int n) { return "flt_" + juce::String (n) + "_type"; }
    inline juce::String fltCut (int n) { return "flt_" + juce::String (n) + "_cut"; }
    inline juce::String fltRes (int n) { return "flt_" + juce::String (n) + "_res"; }

    // ---------- LFO slots (n = 0..MAX_LFOS-1) ----------
    inline juce::String lfoEn     (int n) { return "lfo_" + juce::String (n) + "_en"; }
    inline juce::String lfoShape  (int n) { return "lfo_" + juce::String (n) + "_shp"; }
    inline juce::String lfoRate   (int n) { return "lfo_" + juce::String (n) + "_rate"; }
    inline juce::String lfoDepth  (int n) { return "lfo_" + juce::String (n) + "_dep"; }
    inline juce::String lfoTarget (int n) { return "lfo_" + juce::String (n) + "_tgt"; }
    inline juce::String lfoTgtSlt (int n) { return "lfo_" + juce::String (n) + "_tslot"; }

    // ---------- FX ----------
    static constexpr auto EQ_ENABLED      = "eq_enabled";
    static constexpr auto EQ_BAND_0       = "eq_band_0";
    static constexpr auto EQ_BAND_1       = "eq_band_1";
    static constexpr auto EQ_BAND_2       = "eq_band_2";
    static constexpr auto EQ_BAND_3       = "eq_band_3";
    static constexpr auto EQ_BAND_4       = "eq_band_4";
    static constexpr auto EQ_BAND_5       = "eq_band_5";
    static constexpr auto EQ_BAND_6       = "eq_band_6";
    static constexpr auto EQ_BAND_7       = "eq_band_7";
    static constexpr auto EQ_BAND_8       = "eq_band_8";
    static constexpr auto EQ_BAND_9       = "eq_band_9";

    static constexpr auto REVERB_ENABLED  = "reverb_enabled";
    static constexpr auto REVERB_ROOM     = "reverb_room";
    static constexpr auto REVERB_DAMPING  = "reverb_damping";
    static constexpr auto REVERB_WET      = "reverb_wet";
    static constexpr auto REVERB_WIDTH    = "reverb_width";

    static constexpr auto DIST_ENABLED    = "dist_enabled";
    static constexpr auto DIST_DRIVE      = "dist_drive";
    static constexpr auto DIST_TONE       = "dist_tone";
    static constexpr auto DIST_LEVEL      = "dist_level";
    static constexpr auto DIST_TYPE       = "dist_type";

    static constexpr auto COMP_ENABLED    = "comp_enabled";
    static constexpr auto COMP_THRESHOLD  = "comp_threshold";
    static constexpr auto COMP_RATIO      = "comp_ratio";
    static constexpr auto COMP_ATTACK     = "comp_attack";
    static constexpr auto COMP_RELEASE    = "comp_release";
    static constexpr auto COMP_MAKEUP     = "comp_makeup";

    static constexpr auto GATE_ENABLED    = "gate_enabled";
    static constexpr auto GATE_THRESHOLD  = "gate_threshold";
    static constexpr auto GATE_ATTACK     = "gate_attack";
    static constexpr auto GATE_RELEASE    = "gate_release";
    static constexpr auto GATE_RATIO      = "gate_ratio";
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

    // --- Sample modules: public so editor can access for file loading and labels ---
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

    /** Called from message thread. Loads file into slot; thread-safe. */
    void loadSample (int slotIndex, const juce::File& file);

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // --- Synth engine ---
    std::array<Voice,     MAX_VOICES>  voices;
    std::array<SVFilter,  MAX_FILTERS> filters;
    std::array<LFOGen,    MAX_LFOS>    lfos;

    // Cached per-block param snapshots
    struct SynthSlot  { bool en; int type; float gainLin, detune, att, dec, sus, rel; };
    struct FilterSlot { bool en; int type; float cutoff, res; };
    struct LFOSlot    { bool en; int shape; float rate, depth, target, tgtSlot; };
    struct SampleSlot { bool en, loop; int root; float gainLin, att, dec, sus, rel; };

    std::array<SynthSlot,  MAX_SYNTHS>  synthSlots;
    std::array<FilterSlot, MAX_FILTERS> filterSlots;
    std::array<LFOSlot,    MAX_LFOS>    lfoSlots;
    std::array<SampleSlot, MAX_SAMPLES> sampleSlots;

    // LFO modulation output (applied across block)
    std::array<float, MAX_FILTERS> lfoFltMod  {};   // Hz offset for filter cutoff
    std::array<float, MAX_SYNTHS>  lfoVolMod  {};   // additive gain multiplier
    std::array<float, MAX_SYNTHS>  lfoDetMod  {};   // additive cents

    void loadParams();
    void computeLFOs (float sampleRate);
    void processMidi (const juce::MidiBuffer&);
    void renderSynth (juce::AudioBuffer<float>&);
    void renderSamples (juce::AudioBuffer<float>&);
    void applyFilters (juce::AudioBuffer<float>&);

    int  findFreeVoice()  const noexcept;
    int  findVoiceToSteal() const noexcept;
    void startVoice (int vi, int note, float velocity);
    void stopNote   (int note);
    void allNotesOff();

    // --- Existing FX chain ---
    void updateDSP();
    void updateEQ();
    void updateDistortion();
    void updateCompressor();
    void updateGate();
    void updateReverb();

    static constexpr float EQ_FREQUENCIES[NUM_EQ_BANDS] = {
        31.25f, 62.5f, 125.0f, 250.0f, 500.0f,
        1000.0f, 2000.0f, 4000.0f, 8000.0f, 16000.0f
    };
    static constexpr float EQ_Q = 1.41421356f;

    using StereoFilter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    std::array<StereoFilter, NUM_EQ_BANDS> eqBands;
    StereoFilter distToneFilter;

    juce::dsp::Compressor<float>  compressor;
    juce::dsp::Gain<float>        makeupGain;
    juce::dsp::NoiseGate<float>   noiseGate;
    juce::dsp::Reverb             reverb;

    juce::dsp::ProcessSpec processSpec { 44100.0, 512, 2 };
    bool prepared = false;
    std::atomic<bool> paramsDirty { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessor)
};

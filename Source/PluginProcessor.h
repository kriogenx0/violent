#pragma once
#include <JuceHeader.h>
#include "SynthEngine.h"
#include "FxChain.h"
#include "StreamChain.h"

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

    juce::AudioProcessorValueTreeState apvts;

    // -----------------------------------------------------------------------
    // Stream state — owned here, written by UI, read by audio thread
    // -----------------------------------------------------------------------
    int numActiveStreams = 1;

    struct StreamState
    {
        bool  enabled      = true;
        int   numFilters   = 0;
        int   numFx        = 0;
        std::array<FxType, MAX_STREAM_FX> fxTypes {};
    };
    std::array<StreamState, MAX_STREAMS> streams;

    // --- Level metering ---
    std::atomic<float> levelL { 0.0f }, levelR { 0.0f };

    // --- Sample modules (kept for future sample-player streams) ---
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
    // DSP state — one copy per stream
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

    // Per-FX-slot DSP objects (one instance per active FX slot in a stream)
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

    struct StreamDSP
    {
        OscSlot osc;
        std::array<FltSlot,  MAX_STREAM_FILTERS> filters {};
        float level = 1.0f, pan = 0.0f;

        // Per-stream DSP objects
        std::array<SVFilter,  MAX_STREAM_FILTERS> svFilters;
        std::array<FxSlotDSP, MAX_STREAM_FX>      fxDSP;
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

    std::array<StreamDSP, MAX_STREAMS> streamDSP;

    // -----------------------------------------------------------------------
    // Voice pool — tagged by stream
    // -----------------------------------------------------------------------
    struct StreamVoice
    {
        bool  active     = false;
        int   note       = -1;
        int   streamIdx  = 0;
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

    static constexpr int STREAM_VOICES = 16;
    std::array<StreamVoice, STREAM_VOICES> voices;

    // -----------------------------------------------------------------------
    // Internal methods
    // -----------------------------------------------------------------------
    void loadStreamParams (int s);
    void processMidi (const juce::MidiBuffer&);
    void renderStream (int s, juce::AudioBuffer<float>& master);
    void applyStreamFilters (StreamDSP& dsp, const StreamState& st, juce::AudioBuffer<float>& buf);
    void applyStreamFx (int s, StreamDSP& dsp, const StreamState& st, juce::AudioBuffer<float>& buf);

    int  findFreeVoice  (int streamIdx) const noexcept;
    int  findVoiceToSteal (int streamIdx) const noexcept;
    void startVoice (int vi, int note, float velocity, int streamIdx);
    void stopNote   (int note);
    void allNotesOff();

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

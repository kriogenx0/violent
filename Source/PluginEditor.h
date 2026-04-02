#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
namespace Colours
{
    static const juce::Colour background  { 0xff1e1e2e };
    static const juce::Colour surface     { 0xff313244 };
    static const juce::Colour overlay     { 0xff45475a };
    static const juce::Colour text        { 0xffcdd6f4 };
    static const juce::Colour subtext     { 0xff7f849c };
    static const juce::Colour accent      { 0xffcba6f7 };
    static const juce::Colour green       { 0xffa6e3a1 };
    static const juce::Colour red         { 0xfff38ba8 };
    static const juce::Colour yellow      { 0xfff9e2af };
    static const juce::Colour blue        { 0xff89b4fa };
    static const juce::Colour teal        { 0xff94e2d5 };
}

//==============================================================================
class ViolentLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ViolentLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont (juce::Label&) override;
};

//==============================================================================
class LabelledKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  nameLabel;
    juce::Label  valueLabel;

    LabelledKnob (const juce::String& name, juce::Colour colour = Colours::accent);

    void resized() override;
    void paint (juce::Graphics&) override {}

private:
    juce::Colour knobColour;
};

//==============================================================================
/** One synth oscillator slot strip. Always visible; enable toggle controls DSP. */
class SynthStrip : public juce::Component
{
public:
    SynthStrip (ViolentAudioProcessor& p, int slotIndex);
    ~SynthStrip() override;

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    ViolentAudioProcessor& processor;
    int slot;

    juce::ToggleButton enableBtn;
    juce::Label        nameLabel;
    juce::ComboBox     waveBox;
    juce::Label        waveLabel;

    LabelledKnob gainKnob    { "Gain",    Colours::accent  };
    LabelledKnob detuneKnob  { "Detune",  Colours::yellow  };
    LabelledKnob attackKnob  { "Attack",  Colours::green   };
    LabelledKnob decayKnob   { "Decay",   Colours::teal    };
    LabelledKnob sustainKnob { "Sustain", Colours::blue    };
    LabelledKnob releaseKnob { "Release", Colours::red     };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   enableAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        gainAtt, detuneAtt, attackAtt, decayAtt, sustainAtt, releaseAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthStrip)
};

//==============================================================================
/** One filter slot strip. */
class FilterStrip : public juce::Component
{
public:
    FilterStrip (ViolentAudioProcessor& p, int slotIndex);
    ~FilterStrip() override;

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    ViolentAudioProcessor& processor;
    int slot;

    juce::ToggleButton enableBtn;
    juce::Label        nameLabel;
    juce::ComboBox     typeBox;
    juce::Label        typeLabel;

    LabelledKnob cutoffKnob    { "Cutoff",    Colours::blue   };
    LabelledKnob resonanceKnob { "Resonance", Colours::accent };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   enableAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        cutoffAtt, resonanceAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterStrip)
};

//==============================================================================
/** One LFO slot strip. */
class LFOStrip : public juce::Component
{
public:
    LFOStrip (ViolentAudioProcessor& p, int slotIndex);
    ~LFOStrip() override;

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    ViolentAudioProcessor& processor;
    int slot;

    juce::ToggleButton enableBtn;
    juce::Label        nameLabel;
    juce::ComboBox     shapeBox;
    juce::Label        shapeLabel;
    juce::ComboBox     targetBox;
    juce::Label        targetLabel;
    juce::ComboBox     tgtSlotBox;
    juce::Label        tgtSlotLabel;

    LabelledKnob rateKnob  { "Rate",  Colours::teal   };
    LabelledKnob depthKnob { "Depth", Colours::accent };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>   enableAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        shapeAtt, targetAtt, tgtSlotAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        rateAtt, depthAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LFOStrip)
};

//==============================================================================
/** One sample slot strip with file-load button and pitched playback controls. */
class SampleStrip : public juce::Component
{
public:
    SampleStrip (ViolentAudioProcessor& p, int slotIndex);
    ~SampleStrip() override;

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    ViolentAudioProcessor& processor;
    int slot;

    juce::ToggleButton enableBtn;
    juce::Label        nameLabel;
    juce::TextButton   loadBtn   { "Load..." };
    juce::Label        fileLabel;
    juce::ToggleButton loopBtn;

    juce::Slider rootSlider;
    juce::Label  rootLabel;
    LabelledKnob gainKnob    { "Gain",    Colours::accent  };
    LabelledKnob attackKnob  { "Attack",  Colours::green   };
    LabelledKnob decayKnob   { "Decay",   Colours::teal    };
    LabelledKnob sustainKnob { "Sustain", Colours::blue    };
    LabelledKnob releaseKnob { "Release", Colours::red     };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>  enableAtt, loopAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        rootAtt, gainAtt, attackAtt, decayAtt, sustainAtt, releaseAtt;

    std::unique_ptr<juce::FileChooser> fileChooser;
    void openFileChooser();
    void updateFileLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleStrip)
};

//==============================================================================
class SampleTabPanel : public juce::Component
{
public:
    explicit SampleTabPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (Colours::background); }

private:
    std::array<SampleStrip, MAX_SAMPLES> strips;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTabPanel)
};

//==============================================================================
/** Tab panel holding MAX_SYNTHS SynthStrips stacked vertically. */
class SynthTabPanel : public juce::Component
{
public:
    explicit SynthTabPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (Colours::background); }

private:
    std::array<SynthStrip, MAX_SYNTHS> strips;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthTabPanel)
};

//==============================================================================
class FilterTabPanel : public juce::Component
{
public:
    explicit FilterTabPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (Colours::background); }

private:
    std::array<FilterStrip, MAX_FILTERS> strips;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterTabPanel)
};

//==============================================================================
class ModTabPanel : public juce::Component
{
public:
    explicit ModTabPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (Colours::background); }

private:
    std::array<LFOStrip, MAX_LFOS> strips;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModTabPanel)
};

//==============================================================================
// --- Existing FX panels (declarations only; implementations carry over) ---

class EQPanel : public juce::Component
{
public:
    EQPanel (ViolentAudioProcessor&);
    ~EQPanel() override;
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    ViolentAudioProcessor& processor;
    juce::ToggleButton enableButton { "EQ" };
    static constexpr int NUM_BANDS = 10;
    std::array<juce::Slider, NUM_BANDS> bandSliders;
    std::array<juce::Label,  NUM_BANDS> freqLabels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, NUM_BANDS> sliderAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EQPanel)
};

class ReverbPanel : public juce::Component
{
public:
    ReverbPanel (ViolentAudioProcessor&);
    ~ReverbPanel() override;
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    ViolentAudioProcessor& processor;
    juce::ToggleButton enableButton { "Reverb" };
    LabelledKnob roomKnob    { "Room",    Colours::blue  };
    LabelledKnob dampingKnob { "Damping", Colours::teal  };
    LabelledKnob wetKnob     { "Wet",     Colours::accent };
    LabelledKnob widthKnob   { "Width",   Colours::green };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> roomAtt, dampingAtt, wetAtt, widthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbPanel)
};

class DistortionPanel : public juce::Component
{
public:
    DistortionPanel (ViolentAudioProcessor&);
    ~DistortionPanel() override;
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    ViolentAudioProcessor& processor;
    juce::ToggleButton enableButton { "Distortion" };
    LabelledKnob driveKnob { "Drive", Colours::red    };
    LabelledKnob toneKnob  { "Tone",  Colours::yellow };
    LabelledKnob levelKnob { "Level", Colours::green  };
    juce::ComboBox typeBox;
    juce::Label    typeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  driveAtt, toneAtt, levelAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>  enableAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionPanel)
};

class CompressorPanel : public juce::Component
{
public:
    CompressorPanel (ViolentAudioProcessor&);
    ~CompressorPanel() override;
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    ViolentAudioProcessor& processor;
    juce::ToggleButton enableButton { "Compressor" };
    LabelledKnob threshKnob   { "Threshold", Colours::red    };
    LabelledKnob ratioKnob    { "Ratio",     Colours::yellow };
    LabelledKnob attackKnob   { "Attack",    Colours::green  };
    LabelledKnob releaseKnob  { "Release",   Colours::blue   };
    LabelledKnob makeupKnob   { "Makeup",    Colours::accent };
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        threshAtt, ratioAtt, attackAtt, releaseAtt, makeupAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorPanel)
};

class GatePanel : public juce::Component
{
public:
    GatePanel (ViolentAudioProcessor&);
    ~GatePanel() override;
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    ViolentAudioProcessor& processor;
    juce::ToggleButton enableButton { "Gate" };
    LabelledKnob threshKnob  { "Threshold", Colours::red   };
    LabelledKnob attackKnob  { "Attack",    Colours::green };
    LabelledKnob releaseKnob { "Release",   Colours::blue  };
    LabelledKnob ratioKnob   { "Ratio",     Colours::yellow};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        threshAtt, attackAtt, releaseAtt, ratioAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GatePanel)
};

//==============================================================================
/** Wraps the five existing FX panels with their own sub-tab bar. */
class FxTabPanel : public juce::Component
{
public:
    explicit FxTabPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics&) override;

private:
    ViolentAudioProcessor& processor;

    juce::TextButton tabEQ    { "EQ"   };
    juce::TextButton tabDist  { "DIST" };
    juce::TextButton tabComp  { "COMP" };
    juce::TextButton tabGate  { "GATE" };
    juce::TextButton tabVerb  { "VERB" };

    EQPanel         eqPanel;
    DistortionPanel distPanel;
    CompressorPanel compPanel;
    GatePanel       gatePanel;
    ReverbPanel     reverbPanel;

    int currentFxTab = 0;
    void showFxTab (int idx);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxTabPanel)
};

//==============================================================================
class ViolentAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ViolentAudioProcessorEditor (ViolentAudioProcessor&);
    ~ViolentAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ViolentAudioProcessor& processor;
    ViolentLookAndFeel     laf;

    // Main tabs
    juce::TextButton tabSynth   { "SYNTH"   };
    juce::TextButton tabSamples { "SAMPLES" };
    juce::TextButton tabFilters { "FILTERS" };
    juce::TextButton tabMod     { "MOD"     };
    juce::TextButton tabFx      { "FX"      };

    SynthTabPanel  synthPanel;
    SampleTabPanel samplePanel;
    FilterTabPanel filterPanel;
    ModTabPanel    modPanel;
    FxTabPanel     fxPanel;

    int currentTab = 0;
    void showTab (int tabIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessorEditor)
};

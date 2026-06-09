#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
namespace ViolentColours
{
    static const juce::Colour background  { 0xff111111 };
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

    LabelledKnob (const juce::String& name, juce::Colour colour = ViolentColours::accent);

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

    juce::TextButton   removeBtn { "×" };
    juce::Label        nameLabel;
    juce::ComboBox     waveBox;
    juce::Label        waveLabel;

    std::function<void()> onRemove;

    // Row 1: pitch + character
    LabelledKnob gainKnob    { "Gain",    ViolentColours::accent  };
    LabelledKnob octKnob     { "Octave",  ViolentColours::text    };
    LabelledKnob semiKnob    { "Semi",    ViolentColours::subtext };
    LabelledKnob detuneKnob  { "Detune",  ViolentColours::yellow  };
    LabelledKnob phaseKnob   { "Phase",   ViolentColours::teal    };
    LabelledKnob pwKnob      { "PW",      ViolentColours::blue    };
    LabelledKnob panKnob     { "Pan",     ViolentColours::accent  };
    LabelledKnob velKnob     { "Vel",     ViolentColours::green   };
    LabelledKnob uniKnob     { "Unison",  ViolentColours::red     };
    LabelledKnob uniSpreadKnob { "Spread",ViolentColours::yellow  };

    // Row 2: ADSR
    LabelledKnob attackKnob  { "Attack",  ViolentColours::green   };
    LabelledKnob decayKnob   { "Decay",   ViolentColours::teal    };
    LabelledKnob sustainKnob { "Sustain", ViolentColours::blue    };
    LabelledKnob releaseKnob { "Release", ViolentColours::red     };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        gainAtt, octAtt, semiAtt, detuneAtt, phaseAtt, pwAtt, panAtt,
        velAtt, uniAtt, uniSpreadAtt, attackAtt, decayAtt, sustainAtt, releaseAtt;

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

    LabelledKnob cutoffKnob    { "Cutoff",    ViolentColours::blue   };
    LabelledKnob resonanceKnob { "Resonance", ViolentColours::accent };

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

    LabelledKnob rateKnob  { "Rate",  ViolentColours::teal   };
    LabelledKnob depthKnob { "Depth", ViolentColours::accent };

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
    LabelledKnob gainKnob    { "Gain",    ViolentColours::accent  };
    LabelledKnob attackKnob  { "Attack",  ViolentColours::green   };
    LabelledKnob decayKnob   { "Decay",   ViolentColours::teal    };
    LabelledKnob sustainKnob { "Sustain", ViolentColours::blue    };
    LabelledKnob releaseKnob { "Release", ViolentColours::red     };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>  enableAtt, loopAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        rootAtt, gainAtt, attackAtt, decayAtt, sustainAtt, releaseAtt;

    std::unique_ptr<juce::FileChooser> fileChooser;
    void openFileChooser();
    void updateFileLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleStrip)
};

//==============================================================================
template<typename StripT, int N>
class SlotTabPanel : public juce::Component
{
public:
    template<std::size_t... I>
    SlotTabPanel (ViolentAudioProcessor& p, std::index_sequence<I...>)
        : strips { StripT (p, (int) I)... }
    { for (auto& s : strips) addAndMakeVisible (s); }

    explicit SlotTabPanel (ViolentAudioProcessor& p)
        : SlotTabPanel (p, std::make_index_sequence<N>{}) {}

    void resized() override
    {
        auto area = getLocalBounds().reduced (8, 4);
        const int h = area.getHeight() / N;
        for (auto& s : strips)
            s.setBounds (area.removeFromTop (h));
    }
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::array<StripT, N> strips;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlotTabPanel)
};

using SampleTabPanel = SlotTabPanel<SampleStrip, MAX_SAMPLES>;
using FilterTabPanel = SlotTabPanel<FilterStrip,  MAX_FILTERS>;
using ModTabPanel    = SlotTabPanel<LFOStrip,     MAX_LFOS>;

//==============================================================================
class SynthTabPanel : public juce::Component
{
public:
    static constexpr int STRIP_H = 130;
    static constexpr int BUTTON_H = 36;

    explicit SynthTabPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onCountChanged;

private:
    ViolentAudioProcessor& processor;
    std::array<SynthStrip, MAX_SYNTHS> strips;
    juce::TextButton addBtn { "+" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthTabPanel)
};

//==============================================================================
/** One card in the FX chain — shows knobs for whichever FxType is active. */
class FxCard : public juce::Component
{
public:
    static constexpr int CARD_H = 110;

    FxCard (ViolentAudioProcessor& p, int slotIndex);
    ~FxCard() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;

private:
    ViolentAudioProcessor& processor;
    int slot;

    juce::TextButton removeBtn { "×" };
    juce::Label      titleLabel;

    // Distortion
    LabelledKnob driveKnob  { "Drive",  ViolentColours::red    };
    LabelledKnob toneKnob   { "Tone",   ViolentColours::yellow };
    LabelledKnob levelKnob  { "Level",  ViolentColours::green  };
    juce::ComboBox distTypeBox;

    // Compressor / Gate shared
    LabelledKnob threshKnob   { "Thresh",  ViolentColours::red    };
    LabelledKnob ratioKnob    { "Ratio",   ViolentColours::yellow };
    LabelledKnob attackKnob   { "Attack",  ViolentColours::green  };
    LabelledKnob releaseKnob  { "Release", ViolentColours::blue   };
    LabelledKnob makeupKnob   { "Makeup",  ViolentColours::accent };

    // Reverb
    LabelledKnob roomKnob    { "Room",    ViolentColours::blue   };
    LabelledKnob dampingKnob { "Damping", ViolentColours::teal   };
    LabelledKnob wetKnob     { "Wet",     ViolentColours::accent };
    LabelledKnob widthKnob   { "Width",   ViolentColours::green  };

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAtt> driveAtt, toneAtt, levelAtt;
    std::unique_ptr<ComboAtt>  distTypeAtt;
    std::unique_ptr<SliderAtt> threshAtt, ratioAtt, attackAtt, releaseAtt, makeupAtt;
    std::unique_ptr<SliderAtt> roomAtt, dampingAtt, wetAtt, widthAtt;

    void layoutKnobs (std::initializer_list<LabelledKnob*> knobs, juce::Rectangle<int> area);
    void setAllInvisible();
    void showForType (FxType t);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxCard)
};

//==============================================================================
/** The FX tab: a scrollable vertical chain of FxCards + a catalog "+" button. */
class FxTabPanel : public juce::Component
{
public:
    explicit FxTabPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onCountChanged;

private:
    ViolentAudioProcessor& processor;
    std::array<FxCard, MAX_FX> cards;
    juce::TextButton addBtn { "+ Add Effect" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxTabPanel)
};

//==============================================================================
/** Level meter bar (L+R). Polls processor.levelL/R via a timer. */
class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter() { startTimerHz (30); }
    void setLevels (float l, float r) { peakL = l; peakR = r; }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        const float w = b.getWidth() * 0.45f;
        const float gap = b.getWidth() - 2.0f * w;

        auto drawBar = [&] (float x, float level)
        {
            const float db   = juce::Decibels::gainToDecibels (juce::jmax (0.0001f, level));
            const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
            g.setColour (ViolentColours::surface);
            g.fillRect (x, b.getY(), w, b.getHeight());
            const juce::Colour c = norm > 0.85f ? ViolentColours::red
                                 : norm > 0.6f  ? ViolentColours::yellow
                                                : ViolentColours::green;
            g.setColour (c.withAlpha (0.85f));
            g.fillRect (x, b.getBottom() - b.getHeight() * norm, w, b.getHeight() * norm);
        };

        drawBar (b.getX(), peakL);
        drawBar (b.getX() + w + gap, peakR);
    }

private:
    float peakL = 0.0f, peakR = 0.0f;
    void timerCallback() override { repaint(); }
};

//==============================================================================
class ViolentAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ViolentAudioProcessorEditor (ViolentAudioProcessor&);
    ~ViolentAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void updateMeter (float l, float r) { meter.setLevels (l, r); }

private:
    ViolentAudioProcessor& processor;
    ViolentLookAndFeel     laf;

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

    LevelMeter meter;

    int currentTab = 0;
    void showTab (int tabIndex);
    int  editorHeight() const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessorEditor)
};

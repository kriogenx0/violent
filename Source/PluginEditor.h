#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
namespace ViolentColours
{
    static const juce::Colour background { 0xff111111 };
    static const juce::Colour surface    { 0xff1e1e2e };
    static const juce::Colour overlay    { 0xff313244 };
    static const juce::Colour text       { 0xffcdd6f4 };
    static const juce::Colour subtext    { 0xff7f849c };
    static const juce::Colour accent     { 0xffcba6f7 };
    static const juce::Colour green      { 0xffa6e3a1 };
    static const juce::Colour red        { 0xfff38ba8 };
    static const juce::Colour yellow     { 0xfff9e2af };
    static const juce::Colour blue       { 0xff89b4fa };
    static const juce::Colour teal       { 0xff94e2d5 };
}

//==============================================================================
class ViolentLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ViolentLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float, float,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    juce::Font getLabelFont (juce::Label&) override;
};

//==============================================================================
/** Rotary knob with name above and value below, always visible. */
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
/** Level meter bar (L+R), polled via timer. */
class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter() { startTimerHz (30); }
    void setLevels (float l, float r) noexcept { peakL = l; peakR = r; }
    void paint (juce::Graphics& g) override;

private:
    float peakL = 0.0f, peakR = 0.0f;
    void timerCallback() override { repaint(); }
};

//==============================================================================
/** One FX card inside a stream's effect chain. */
class StreamFxCard : public juce::Component
{
public:
    static constexpr int CARD_H = 100;

    StreamFxCard (ViolentAudioProcessor& p, int streamIdx, int fxSlot);
    ~StreamFxCard() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForType (FxType t);

private:
    ViolentAudioProcessor& processor;
    int stream, slot;

    juce::TextButton removeBtn { "×" };
    juce::Label      titleLabel;

    LabelledKnob driveKnob  { "Drive",  ViolentColours::red    };
    LabelledKnob toneKnob   { "Tone",   ViolentColours::yellow };
    LabelledKnob levelKnob  { "Level",  ViolentColours::green  };
    juce::ComboBox distTypeBox;

    LabelledKnob threshKnob  { "Thresh",  ViolentColours::red    };
    LabelledKnob ratioKnob   { "Ratio",   ViolentColours::yellow };
    LabelledKnob attackKnob  { "Attack",  ViolentColours::green  };
    LabelledKnob releaseKnob { "Release", ViolentColours::blue   };
    LabelledKnob makeupKnob  { "Makeup",  ViolentColours::accent };

    LabelledKnob roomKnob    { "Room",    ViolentColours::blue   };
    LabelledKnob dampingKnob { "Damping", ViolentColours::teal   };
    LabelledKnob wetKnob     { "Wet",     ViolentColours::accent };
    LabelledKnob widthKnob   { "Width",   ViolentColours::green  };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SA> driveAtt, toneAtt, levelAtt;
    std::unique_ptr<CA> distTypeAtt;
    std::unique_ptr<SA> threshAtt, ratioAtt, attackKnobAtt, releaseAtt, makeupAtt;
    std::unique_ptr<SA> roomAtt, dampingAtt, wetAtt, widthAtt;

    void setAllInvisible();
    void layoutKnobs (std::initializer_list<LabelledKnob*>, juce::Rectangle<int>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StreamFxCard)
};

//==============================================================================
/** One filter row inside a stream. */
class StreamFilterRow : public juce::Component
{
public:
    static constexpr int ROW_H = 70;

    StreamFilterRow (ViolentAudioProcessor& p, int streamIdx, int filterSlot);
    ~StreamFilterRow() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;

private:
    ViolentAudioProcessor& processor;
    int stream, slot;

    juce::TextButton removeBtn { "×" };
    juce::Label      nameLabel;
    juce::ComboBox   typeBox;
    LabelledKnob     cutoffKnob { "Cutoff",    ViolentColours::blue   };
    LabelledKnob     resKnob    { "Resonance", ViolentColours::accent };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<CA> typeAtt;
    std::unique_ptr<SA> cutoffAtt, resAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StreamFilterRow)
};

//==============================================================================
/** One complete stream card: source + filters + FX + level/pan. */
class StreamCard : public juce::Component
{
public:
    static constexpr int SOURCE_H  = 150;  // source section height
    static constexpr int HEADER_H  = 36;
    static constexpr int FOOTER_H  = 44;

    StreamCard (ViolentAudioProcessor& p, int streamIdx);
    ~StreamCard() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

private:
    ViolentAudioProcessor& processor;
    int stream;

    // Header
    juce::TextButton removeBtn    { "×" };
    juce::Label      nameLabel;
    juce::ToggleButton enableBtn;

    // Source section
    juce::ComboBox   srcTypeBox;
    juce::Label      srcTypeLabel;
    juce::TextButton loadSampleBtn { "Load…" };
    juce::Label      sampleFileLabel;

    LabelledKnob gainKnob    { "Gain",    ViolentColours::accent  };
    LabelledKnob octKnob     { "Oct",     ViolentColours::text    };
    LabelledKnob semiKnob    { "Semi",    ViolentColours::subtext };
    LabelledKnob detKnob     { "Detune",  ViolentColours::yellow  };
    LabelledKnob phaseKnob   { "Phase",   ViolentColours::teal    };
    LabelledKnob pwKnob      { "PW",      ViolentColours::blue    };
    LabelledKnob panKnob     { "Pan",     ViolentColours::accent  };
    LabelledKnob velKnob     { "Vel",     ViolentColours::green   };
    LabelledKnob uniKnob     { "Unison",  ViolentColours::red     };
    LabelledKnob uniSpreadKnob { "Spread",ViolentColours::yellow  };

    // ADSR box
    LabelledKnob attKnob { "Attack",  ViolentColours::green };
    LabelledKnob decKnob { "Decay",   ViolentColours::teal  };
    LabelledKnob susKnob { "Sustain", ViolentColours::blue  };
    LabelledKnob relKnob { "Release", ViolentColours::red   };

    // Footer: level + pan
    LabelledKnob levelKnob { "Level", ViolentColours::accent };
    LabelledKnob streamPan { "Pan",   ViolentColours::yellow };

    // Filter chain
    std::array<std::unique_ptr<StreamFilterRow>, MAX_STREAM_FILTERS> filterRows;
    juce::TextButton addFilterBtn { "+ Filter" };

    // FX chain
    std::array<std::unique_ptr<StreamFxCard>, MAX_STREAM_FX> fxCards;
    juce::TextButton addFxBtn { "+ Effect" };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<BA> enableAtt;
    std::unique_ptr<CA> srcTypeAtt;
    std::unique_ptr<SA> gainAtt, octAtt, semiAtt, detAtt, phaseAtt, pwAtt, panSrcAtt,
                        velAtt, uniAtt, uniSpreadAtt, attAtt, decAtt, susAtt, relAtt;
    std::unique_ptr<SA> levelAtt, streamPanAtt;

    std::unique_ptr<juce::FileChooser> fileChooser;
    void openFilePicker();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StreamCard)
};

//==============================================================================
/** Main panel: vertical list of StreamCards. */
class StreamPanel : public juce::Component
{
public:
    explicit StreamPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

private:
    ViolentAudioProcessor& processor;
    std::array<std::unique_ptr<StreamCard>, MAX_STREAMS> cards;
    juce::TextButton addBtn { "+ Add Stream" };

    void rebuild();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StreamPanel)
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

    StreamPanel streamPanel;
    LevelMeter  meter;

    int editorHeight() const noexcept;
    void updateHeight();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessorEditor)
};

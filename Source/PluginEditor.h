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

//==============================================================================
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

//==============================================================================
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

//==============================================================================
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

//==============================================================================
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
class ViolentAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ViolentAudioProcessorEditor (ViolentAudioProcessor&);
    ~ViolentAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ViolentAudioProcessor& processor;
    ViolentLookAndFeel laf;

    // Tab buttons
    juce::TextButton tabEQ         { "EQ"    };
    juce::TextButton tabReverb     { "REVERB" };
    juce::TextButton tabDistortion { "DIST"  };
    juce::TextButton tabCompressor { "COMP"  };
    juce::TextButton tabGate       { "GATE"  };

    // Panels
    EQPanel         eqPanel;
    ReverbPanel     reverbPanel;
    DistortionPanel distPanel;
    CompressorPanel compPanel;
    GatePanel       gatePanel;

    int currentTab = 0;
    void showTab (int tabIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessorEditor)
};

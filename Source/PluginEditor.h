#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Views/SharedComponents.h"
#include "Views/RackView.h"

//==============================================================================
class ViolentAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit ViolentAudioProcessorEditor (ViolentAudioProcessor&);
    ~ViolentAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int HEADER_H = 52;
    // Content taller than this scrolls instead of growing the window further.
    static constexpr int MAX_WINDOW_H = 800;

    ViolentAudioProcessor& processor;
    ViolentUI laf;

    ScalableRackComponent rack;
    RackScaler rackScaler { rack };
    juce::Viewport rackViewport;
    NavPanel navPanel;
    juce::Viewport navViewport;

    LevelMeter  meter;
    juce::TextButton previewBtn;
    juce::ComboBox   previewPatternBox;

    juce::ComboBox   presetBox;
    juce::TextButton savePresetBtn { "Save" };

    juce::TextButton zoomOutBtn { "-" };
    juce::TextButton zoomInBtn  { "+" };
    juce::Label      zoomLabel;

    juce::TextButton randomizeBtn { "Randomize" };

    float uiScale = 1.0f;
    void setUiScale (float newScale);

    void timerCallback() override;

    void refreshPresetList();
    void loadSelectedPreset();
    void promptAndSavePreset();

    void updateHeight();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessorEditor)
};

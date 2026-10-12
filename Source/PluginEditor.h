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

    ViolentAudioProcessor& audioProcessor;
    ViolentUI laf;

    ScalableRackComponent rack;
    RackScaler rackScaler { rack };
    juce::Viewport rackViewport;
    Minimap minimap;
    juce::Viewport navViewport;

    LevelMeter  meter { audioProcessor };
    MidiActivityLight midiLight { audioProcessor };
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

    // Re-lays out the rack for its current contents/zoom. Never resizes the
    // window - see the comment on its definition for why that matters.
    void updateLayout();

    // Sizes the window for the current zoom level (width), and for the screen
    // (height) the first time only.
    void updateWindowSize();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessorEditor)
};

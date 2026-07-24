#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** One complete generator card: a name/enable/delete header (top-left, like
    every other component), a source section, a wave-shape section (Detune/
    Phase/PW/Unison/Spread — everything that shapes the raw waveform), and an
    ADSR envelope. MIDI modifiers and effects are no longer hosted here —
    they're shared, routed Components that live in their own panels elsewhere
    in the rack; Level/Pan live in the Mixer at the bottom of the rack. */
class GeneratorCard : public juce::Component
{
public:
    static constexpr int HEADER_H     = 36;
    static constexpr int SOURCE_H     = 96;   // source knobs row
    static constexpr int WAVESHAPE_H  = 76;   // detune/phase/pw/unison/spread box
    static constexpr int ADSR_H       = 76;   // envelope box

    GeneratorCard (ViolentAudioProcessor& p, int generatorIdx);
    ~GeneratorCard() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;

    int preferredHeight() const noexcept;

    // For the minimap.
    juce::String getDisplayName() const { return nameLabel.getText(); }

private:
    ViolentAudioProcessor& processor;
    int generator;

    // Header (top-left: delete, enable, name)
    DeleteButton        removeBtn;
    juce::ToggleButton   enableBtn;
    juce::Label          nameLabel;
    WaveformView         waveformView;

    // Source section
    juce::TextButton synthModeBtn   { "Synth" };
    juce::TextButton samplerModeBtn { "Sampler" };
    juce::ComboBox   srcTypeBox;
    juce::TextButton loadSampleBtn { "Load..." };
    juce::Label      sampleFileLabel;
    int lastWaveformIndex = 0;

    juce::Label sourceLabel, waveShapeLabel, adsrLabel;

    LabelledKnob gainKnob { "Gain", ViolentColours::accent };
    LabelledKnob panKnob  { "Pan",  ViolentColours::accent };
    LabelledKnob velKnob  { "Vel",  ViolentColours::green  };

    // Wave-shape section — anything that modifies the raw wave signal
    LabelledKnob detKnob       { "Detune", ViolentColours::yellow };
    LabelledKnob phaseKnob     { "Phase",  ViolentColours::teal   };
    LabelledKnob pwKnob        { "PW",     ViolentColours::blue   };
    LabelledKnob uniKnob       { "Unison", ViolentColours::red    };
    LabelledKnob uniSpreadKnob { "Spread", ViolentColours::yellow };

    // ADSR box
    LabelledKnob attKnob { "Attack",  ViolentColours::green };
    LabelledKnob decKnob { "Decay",   ViolentColours::teal  };
    LabelledKnob susKnob { "Sustain", ViolentColours::blue  };
    LabelledKnob relKnob { "Release", ViolentColours::red   };

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<BA> enableAtt;
    std::unique_ptr<CA> srcTypeAtt;

    std::unique_ptr<juce::FileChooser> fileChooser;
    void openFilePicker();

    // Bounding boxes drawn as insets in paint().
    juce::Rectangle<int> waveShapeBoxBounds, adsrBoxBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorCard)
};

//==============================================================================
/** Main panel: vertical list of GeneratorCards. */
class GeneratorPanel : public juce::Component
{
public:
    explicit GeneratorPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every card from scratch, e.g. after a preset load
    // changes generator counts/modes behind the UI's back.
    void refreshFromState();

    // For the minimap.
    int getNumCards() const noexcept { return processor.numActiveGenerators; }
    GeneratorCard* getCard (int i) const { return cards[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    std::array<std::unique_ptr<GeneratorCard>, MAX_GENERATORS> cards;
    juce::TextButton addBtn { "+ Add Generator" };

    void rebuild (bool forceRecreate = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorPanel)
};

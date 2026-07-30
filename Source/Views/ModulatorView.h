#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** One shared Modulator Component: a delete/enable/name header (top-left,
    like every other component), its source-specific controls (LFO rate+
    shape, or Envelope ADSR), an Amount knob, and a target-parameter picker —
    the parameter this modulator nudges up/down by up to Amount around
    whatever it's currently set to. */
class ModulatorRow : public juce::Component
{
public:
    static constexpr int ROW_H = 132;

    ModulatorRow (ViolentAudioProcessor& p, int slot);

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForSource (ModulatorSourceType t);

private:
    ViolentAudioProcessor& processor;
    int slot;

    DeleteButton        removeBtn;
    juce::ToggleButton   enableBtn { "ON" };
    juce::Label          nameLabel;

    LabelledKnob amountKnob { "Amount", ViolentColours::accent };

    LabelledKnob   lfoRateKnob { "Rate", ViolentColours::yellow };
    juce::ComboBox lfoShapeBox;

    LabelledKnob envAttKnob { "Attack",  ViolentColours::green };
    LabelledKnob envDecKnob { "Decay",   ViolentColours::teal  };
    LabelledKnob envSusKnob { "Sustain", ViolentColours::blue  };
    LabelledKnob envRelKnob { "Release", ViolentColours::red   };

    juce::Label    targetLabel;
    juce::ComboBox targetBox;

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CA> lfoShapeAtt;

    void updateEnabledLook();
    void rebuildTargetBox();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulatorRow)
};

//==============================================================================
/** Shared Modulator rack — sits between Generators and Effects. */
class ModulatorPanel : public juce::Component
{
public:
    explicit ModulatorPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every row from scratch, e.g. after a preset load.
    void refreshFromState();

    // For the minimap.
    int getNumRows() const noexcept { return processor.numModulators; }
    ModulatorRow* getRow (int i) const { return rows[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<ModulatorRow>, MAX_MODULATORS> rows;

    // One box per source type — clicking a box adds that type directly,
    // rather than a single "+" button opening a dropdown menu.
    std::array<juce::TextButton, NUM_MODULATOR_SOURCE_TYPES> addTypeBtns;

    void rebuild (bool forceRecreate = false);
    void addRow (int arrayIndex, ModulatorSourceType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulatorPanel)
};

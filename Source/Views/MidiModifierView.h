#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** One shared MIDI Modifier Component: a delete/enable/name header (top-left,
    like every other component), its type-specific controls (pitch shift,
    key shift, or arpeggiator), and a routing row selecting which generators
    it applies to — attached directly below its own controls, per-component,
    rather than a separate standalone router panel. */
class MidiModifierRow : public juce::Component
{
public:
    static constexpr int ROW_H = 96;

    MidiModifierRow (ViolentAudioProcessor& p, int slot);

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForType (MidiModType t);

private:
    ViolentAudioProcessor& processor;
    int slot;

    DeleteButton       removeBtn;
    juce::ToggleButton  enableBtn { "ON" };
    juce::Label         nameLabel;

    LabelledKnob     transposeKnob { "Transpose", ViolentColours::teal   };
    LabelledKnob     octaveKnob    { "Octave",    ViolentColours::blue   };
    juce::ComboBox   keyRootBox;
    juce::ComboBox   keyScaleBox;
    LabelledKnob     arpRateKnob  { "Arp Rate", ViolentColours::yellow };

    juce::Label routingLabel;
    std::array<juce::TextButton, MAX_GENERATORS> routingBtns;

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CA> keyRootAtt, keyScaleAtt;

    void setAllInvisible();
    void updateEnabledLook();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiModifierRow)
};

//==============================================================================
/** Shared MIDI Modifier rack — sits before Generators; each modifier applies
    to whichever generators its routing row selects. */
class MidiModifierPanel : public juce::Component
{
public:
    explicit MidiModifierPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every row from scratch, e.g. after a preset load.
    void refreshFromState();

    // For the minimap.
    int getNumRows() const noexcept { return processor.numMidiModifiers; }
    MidiModifierRow* getRow (int i) const { return rows[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<MidiModifierRow>, MAX_MIDI_MODIFIERS> rows;

    // One box per type — clicking a box adds that type directly, rather
    // than a single "+" button opening a dropdown menu.
    std::array<juce::TextButton, NUM_MIDI_MOD_TYPES> addTypeBtns;

    void rebuild (bool forceRecreate = false);
    void addRow (int arrayIndex, MidiModType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiModifierPanel)
};

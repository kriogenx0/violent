#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** One shared Effect Component (Filter is just one selectable type here, not
    a separate concept): a delete/enable/name header (top-left, like every
    other component), its type-specific controls, and a routing row selecting
    which generators sum into it — attached directly below its own controls. */
class EffectRow : public juce::Component
{
public:
    static constexpr int CONTROLS_H        = 84;
    static constexpr int FILTER_CONTROLS_H = 134;
    static constexpr int ROUTING_H         = 30;

    EffectRow (ViolentAudioProcessor& p, int slot);
    ~EffectRow() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForType (FxType t);
    int preferredHeight() const noexcept;

private:
    ViolentAudioProcessor& processor;
    int slot;

    DeleteButton        removeBtn;
    juce::ToggleButton   enableBtn { "ON" };
    juce::Label          nameLabel;

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

    FilterTypeSelector filterTypeSelector;
    LabelledKnob     filterCutoffKnob { "Cutoff",    ViolentColours::blue   };
    LabelledKnob     filterResKnob    { "Resonance", ViolentColours::accent };
    FilterResponseView filterResponseView;

    juce::Label routingLabel;
    std::array<juce::TextButton, MAX_GENERATORS> routingBtns;

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CA> distTypeAtt;

    void setAllInvisible();
    void updateEnabledLook();
    void layoutKnobs (std::initializer_list<LabelledKnob*>, juce::Rectangle<int>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectRow)
};

//==============================================================================
/** Shared Effect rack — sits after Generators/Modulators; each effect sums
    whichever generators its routing row selects, processes that sum, and
    mixes the result additively into master. Generators claimed by no effect
    mix straight through dry. */
class EffectPanel : public juce::Component
{
public:
    explicit EffectPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every row from scratch, e.g. after a preset load.
    void refreshFromState();

    // For the minimap.
    int getNumRows() const noexcept { return processor.numEffects; }
    EffectRow* getRow (int i) const { return rows[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<EffectRow>, MAX_EFFECTS> rows;
    juce::TextButton addBtn { "+ Add Effect" };

    void rebuild (bool forceRecreate = false);
    void addRow (int arrayIndex, FxType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EffectPanel)
};

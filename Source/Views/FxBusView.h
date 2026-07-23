#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** One shared FX bus: a single chosen effect that any generator can send a
    portion of its (post-FX) signal into via a per-generator send knob (see
    GeneratorCard's Sends row). An alternative routing path alongside each
    generator's own private FX chain, not a replacement for it. */
class FxBusRow : public juce::Component
{
public:
    static constexpr int ROW_H        = 100;
    static constexpr int FILTER_ROW_H = 150;

    FxBusRow (ViolentAudioProcessor& p, int busSlot);
    ~FxBusRow() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForType (FxType t);
    int preferredHeight() const noexcept;

private:
    ViolentAudioProcessor& processor;
    int bus;

    TrashButton      removeBtn;
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

    FilterTypeSelector filterTypeSelector;
    LabelledKnob     filterCutoffKnob { "Cutoff",    ViolentColours::blue   };
    LabelledKnob     filterResKnob    { "Resonance", ViolentColours::accent };
    FilterResponseView filterResponseView;

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CA> distTypeAtt;

    void setAllInvisible();
    void layoutKnobs (std::initializer_list<LabelledKnob*>, juce::Rectangle<int>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxBusRow)
};

//==============================================================================
/** Shared FX bus rack — generators send into these; each bus applies a
    single chosen effect and mixes its result additively into the master. */
class FxBusPanel : public juce::Component
{
public:
    explicit FxBusPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every row from scratch, e.g. after a preset load.
    void refreshFromState();

    // For the nav panel.
    int getNumBuses() const noexcept { return processor.numFxBuses; }
    FxBusRow* getRow (int i) const { return rows[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<FxBusRow>, MAX_FX_BUSES> rows;
    juce::TextButton addBtn { "+ Add Bus" };

    void rebuild (bool forceRecreate = false);
    void addRow (int arrayIndex, FxType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxBusPanel)
};


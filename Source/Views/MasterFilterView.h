#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** One master filter: type/cutoff/resonance plus which generators route into it.
    Master filters are applied last, after every generator's own filters/FX. */
class MasterFilterRow : public juce::Component
{
public:
    static constexpr int ROW_H = 136;

    MasterFilterRow (ViolentAudioProcessor& p, int filterSlot);

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;

private:
    ViolentAudioProcessor& processor;
    int slot;

    FilterControlsBlock block;

    juce::Label routingLabel;
    std::array<juce::TextButton, MAX_GENERATORS> routingBtns;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterFilterRow)
};

//==============================================================================
/** Master filter chain — added last, after all generators. */
class MasterFilterPanel : public juce::Component
{
public:
    explicit MasterFilterPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every row from scratch, e.g. after a preset load.
    void refreshFromState();

    // For the nav panel.
    int getNumRows() const noexcept { return processor.numMasterFilters; }
    MasterFilterRow* getRow (int i) const { return rows[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<MasterFilterRow>, MAX_MASTER_FILTERS> rows;
    juce::TextButton addBtn { "+ Add Filter" };

    void rebuild (bool forceRecreate = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterFilterPanel)
};


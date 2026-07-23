#include "MasterFilterView.h"

//==============================================================================
// MasterFilterRow
//==============================================================================
MasterFilterRow::MasterFilterRow (ViolentAudioProcessor& p, int filterSlot)
    : processor (p), slot (filterSlot),
      block (p.apvts, "Filter " + juce::String (filterSlot + 1),
             ParamIDs::masterFltType (filterSlot),
             ParamIDs::masterFltCut  (filterSlot),
             ParamIDs::masterFltRes  (filterSlot))
{
    addAndMakeVisible (block);
    block.onRemove = [this] { if (onRemove) onRemove(); };

    routingLabel.setText ("Applies to:", juce::dontSendNotification);
    routingLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    routingLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f)));
    addAndMakeVisible (routingLabel);

    auto& mf = processor.masterFilters[(size_t) slot];
    for (int g = 0; g < MAX_GENERATORS; ++g)
    {
        auto& btn = routingBtns[(size_t) g];
        btn.setButtonText (juce::String (g + 1));
        btn.setClickingTogglesState (true);
        btn.setToggleState (mf.routing[(size_t) g], juce::dontSendNotification);
        btn.onClick = [this, g]
        {
            processor.masterFilters[(size_t) slot].routing[(size_t) g] = routingBtns[(size_t) g].getToggleState();
        };
        addAndMakeVisible (btn);
    }
}

void MasterFilterRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
}

void MasterFilterRow::resized()
{
    auto a = getLocalBounds();

    auto routingRow = a.removeFromBottom (30).reduced (4, 2);
    block.setBounds (a);

    routingLabel.setBounds (routingRow.removeFromLeft (70));
    for (auto& btn : routingBtns)
        btn.setBounds (routingRow.removeFromLeft (26).reduced (2, 2));
}

//==============================================================================
// MasterFilterPanel
//==============================================================================
MasterFilterPanel::MasterFilterPanel (ViolentAudioProcessor& p) : processor (p)
{
    sectionLabel.setText ("MASTER FILTERS - applied last, after every generator",
                           juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    addAndMakeVisible (addBtn);
    addBtn.onClick = [this]
    {
        if (processor.numMasterFilters >= MAX_MASTER_FILTERS) return;
        ++processor.numMasterFilters;
        rebuild();
        if (onLayoutChanged) onLayoutChanged();
    };
    rebuild();
}

void MasterFilterPanel::refreshFromState()
{
    rebuild (true);
}

void MasterFilterPanel::rebuild (bool forceRecreate)
{
    const int n = processor.numMasterFilters;
    for (int f = 0; f < MAX_MASTER_FILTERS; ++f)
    {
        if (f < n)
        {
            if (!rows[(size_t) f] || forceRecreate)
            {
                rows[(size_t) f] = std::make_unique<MasterFilterRow> (processor, f);
                addAndMakeVisible (*rows[(size_t) f]);
                rows[(size_t) f]->onRemove = [this, f]
                {
                    for (int j = f; j < processor.numMasterFilters - 1; ++j)
                        processor.masterFilters[(size_t) j] = processor.masterFilters[(size_t) (j + 1)];
                    processor.masterFilters[(size_t) (--processor.numMasterFilters)] = {};
                    rebuild();
                    if (onLayoutChanged) onLayoutChanged();
                };
            }
        }
        else
        {
            rows[(size_t) f] = nullptr;
        }
    }
    resized();
}

int MasterFilterPanel::preferredHeight() const noexcept
{
    int h = 8 + 24; // section label
    for (int f = 0; f < processor.numMasterFilters; ++f)
        if (rows[(size_t) f]) h += MasterFilterRow::ROW_H + 8;
    h += 40; // add button
    return h;
}

void MasterFilterPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    sectionLabel.setBounds (a.removeFromTop (24));
    for (int f = 0; f < processor.numMasterFilters; ++f)
    {
        if (!rows[(size_t) f]) continue;
        rows[(size_t) f]->setBounds (a.removeFromTop (MasterFilterRow::ROW_H));
        a.removeFromTop (8);
    }
    if (processor.numMasterFilters < MAX_MASTER_FILTERS)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numMasterFilters < MAX_MASTER_FILTERS);
}


#include "RackView.h"

//==============================================================================
// ScalableRackComponent
//==============================================================================
ScalableRackComponent::ScalableRackComponent (ViolentAudioProcessor& p)
    : processor (p), generatorPanel (p), fxBusPanel (p), masterFilterPanel (p), mixerPanel (p)
{
    addAndMakeVisible (generatorPanel);
    addAndMakeVisible (fxBusPanel);
    addAndMakeVisible (masterFilterPanel);
    addAndMakeVisible (mixerPanel);

    generatorPanel.onLayoutChanged = [this] { mixerPanel.rebuild(); if (onLayoutChanged) onLayoutChanged(); };
    // Adding/removing a shared FX bus changes how many send knobs every
    // generator card should show, so every generator card needs rebuilding.
    fxBusPanel.onLayoutChanged = [this] { generatorPanel.refreshFromState(); if (onLayoutChanged) onLayoutChanged(); };
    masterFilterPanel.onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
}

int ScalableRackComponent::preferredHeight() const noexcept
{
    return generatorPanel.preferredHeight() + fxBusPanel.preferredHeight()
         + masterFilterPanel.preferredHeight() + MixerPanel::PANEL_H;
}

void ScalableRackComponent::refreshFromState()
{
    generatorPanel.refreshFromState();
    fxBusPanel.refreshFromState();
    masterFilterPanel.refreshFromState();
    mixerPanel.rebuild();
    resized();
}

void ScalableRackComponent::resized()
{
    const int genH = generatorPanel.preferredHeight();
    generatorPanel.setBounds (0, 0, BASE_WIDTH, genH);
    generatorPanel.resized();

    const int fxBusH = fxBusPanel.preferredHeight();
    fxBusPanel.setBounds (0, genH, BASE_WIDTH, fxBusH);
    fxBusPanel.resized();

    const int mfH = masterFilterPanel.preferredHeight();
    masterFilterPanel.setBounds (0, genH + fxBusH, BASE_WIDTH, mfH);
    masterFilterPanel.resized();

    mixerPanel.setBounds (0, genH + fxBusH + mfH, BASE_WIDTH, MixerPanel::PANEL_H);
    mixerPanel.resized();
}

//==============================================================================
// NavPanel
//==============================================================================
NavPanel::NavPanel (ViolentAudioProcessor& p, ScalableRackComponent& r,
                     juce::Component& scrollSpaceIn, juce::Viewport& viewportIn)
    : processor (p), rack (r), scrollSpace (scrollSpaceIn), viewport (viewportIn)
{
    refreshFromState();
}

void NavPanel::addEntry (const juce::String& label, juce::Component* target)
{
    auto btn = std::make_unique<juce::TextButton> (label);
    btn->setRepaintsOnMouseActivity (true);
    btn->onClick = [this, target]
    {
        if (target == nullptr) return;
        const auto pos = scrollSpace.getLocalPoint (target, juce::Point<int> (0, 0));
        viewport.setViewPosition (0, juce::jmax (0, pos.y - 8));
    };
    addAndMakeVisible (*btn);
    entries.push_back ({ std::move (btn), juce::Component::SafePointer<juce::Component> (target) });
}

void NavPanel::refreshFromState()
{
    entries.clear();

    auto& generatorPanel = rack.getGeneratorPanel();
    for (int g = 0; g < generatorPanel.getNumUnits(); ++g)
    {
        auto* unit = generatorPanel.getUnit (g);
        if (unit == nullptr) continue;

        const auto& genForMidi = processor.generators[(size_t) g];
        for (int m = 0; m < unit->getNumMidiMods(); ++m)
            addEntry (midiModTypeName (genForMidi.midiModTypes[(size_t) m]), unit->getMidiRow (m));

        auto& card = unit->getCard();
        addEntry (card.getSourceTypeLabel(), &card);

        const auto& gen = processor.generators[(size_t) g];
        for (int x = 0; x < gen.numFx; ++x)
            addEntry (gen.fxTypes[(size_t) x] == FxType::Filter ? "Filter" : "Effect", card.getFxCard (x));
    }

    auto& fxBusPanel = rack.getFxBusPanel();
    for (int b = 0; b < fxBusPanel.getNumBuses(); ++b)
        addEntry ("FX Bus", fxBusPanel.getRow (b));

    auto& masterFilterPanel = rack.getMasterFilterPanel();
    for (int m = 0; m < masterFilterPanel.getNumRows(); ++m)
        addEntry ("Master Filter", masterFilterPanel.getRow (m));

    setSize (WIDTH, preferredHeight());
    resized();
}

int NavPanel::preferredHeight() const noexcept
{
    return 4 + (int) entries.size() * 28;
}

void NavPanel::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);
}

void NavPanel::resized()
{
    auto a = getLocalBounds().reduced (4, 0);
    for (auto& e : entries)
        e.button->setBounds (a.removeFromTop (24).reduced (0, 1));
}


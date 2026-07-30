#include "RackView.h"

//==============================================================================
// ScalableRackComponent
//==============================================================================
ScalableRackComponent::ScalableRackComponent (ViolentAudioProcessor& p)
    : processor (p), midiModifierPanel (p), generatorPanel (p), modulatorPanel (p), effectPanel (p),
      mixerPanel (p), spectrumPanel (p)
{
    addAndMakeVisible (midiModifierPanel);
    addAndMakeVisible (generatorPanel);
    addAndMakeVisible (modulatorPanel);
    addAndMakeVisible (effectPanel);
    addAndMakeVisible (mixerPanel);
    addAndMakeVisible (spectrumPanel);

    midiModifierPanel.onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
    generatorPanel.onLayoutChanged = [this] { mixerPanel.rebuild(); if (onLayoutChanged) onLayoutChanged(); };
    modulatorPanel.onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
    effectPanel.onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
    spectrumPanel.onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
}

int ScalableRackComponent::preferredHeight() const noexcept
{
    return midiModifierPanel.preferredHeight() + generatorPanel.preferredHeight()
         + modulatorPanel.preferredHeight() + effectPanel.preferredHeight()
         + MixerPanel::PANEL_H + spectrumPanel.preferredHeight();
}

void ScalableRackComponent::refreshFromState()
{
    midiModifierPanel.refreshFromState();
    generatorPanel.refreshFromState();
    modulatorPanel.refreshFromState();
    effectPanel.refreshFromState();
    mixerPanel.rebuild();
    resized();
}

void ScalableRackComponent::resized()
{
    const int mmH = midiModifierPanel.preferredHeight();
    midiModifierPanel.setBounds (0, 0, BASE_WIDTH, mmH);
    midiModifierPanel.resized();

    const int genH = generatorPanel.preferredHeight();
    generatorPanel.setBounds (0, mmH, BASE_WIDTH, genH);
    generatorPanel.resized();

    const int modH = modulatorPanel.preferredHeight();
    modulatorPanel.setBounds (0, mmH + genH, BASE_WIDTH, modH);
    modulatorPanel.resized();

    const int fxH = effectPanel.preferredHeight();
    effectPanel.setBounds (0, mmH + genH + modH, BASE_WIDTH, fxH);
    effectPanel.resized();

    mixerPanel.setBounds (0, mmH + genH + modH + fxH, BASE_WIDTH, MixerPanel::PANEL_H);
    mixerPanel.resized();

    const int specY = mmH + genH + modH + fxH + MixerPanel::PANEL_H;
    spectrumPanel.setBounds (0, specY, BASE_WIDTH, spectrumPanel.preferredHeight());
    spectrumPanel.resized();
}

//==============================================================================
// Minimap
//==============================================================================
Minimap::Minimap (ViolentAudioProcessor& p, ScalableRackComponent& r,
                   juce::Component& scrollSpaceIn, juce::Viewport& viewportIn)
    : processor (p), rack (r), scrollSpace (scrollSpaceIn), viewport (viewportIn)
{
    refreshFromState();
}

void Minimap::addEntry (const juce::String& label, juce::Component* target)
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

void Minimap::refreshFromState()
{
    entries.clear();

    auto& midiModifierPanel = rack.getMidiModifierPanel();
    for (int m = 0; m < midiModifierPanel.getNumRows(); ++m)
        addEntry (processor.midiModifiers[(size_t) m].name, midiModifierPanel.getRow (m));

    auto& generatorPanel = rack.getGeneratorPanel();
    for (int g = 0; g < generatorPanel.getNumCards(); ++g)
        addEntry (generatorPanel.getCard (g)->getDisplayName(), generatorPanel.getCard (g));

    auto& modulatorPanel = rack.getModulatorPanel();
    for (int m = 0; m < modulatorPanel.getNumRows(); ++m)
        addEntry (processor.modulators[(size_t) m].name, modulatorPanel.getRow (m));

    auto& effectPanel = rack.getEffectPanel();
    for (int x = 0; x < effectPanel.getNumRows(); ++x)
        addEntry (processor.effects[(size_t) x].name, effectPanel.getRow (x));

    addEntry ("Mixer", &rack.getMixerPanel());
    addEntry ("Spectrum", &rack.getSpectrumPanel());

    setSize (WIDTH, preferredHeight());
    resized();
}

int Minimap::preferredHeight() const noexcept
{
    return 4 + (int) entries.size() * 28;
}

void Minimap::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);
}

void Minimap::resized()
{
    auto a = getLocalBounds().reduced (4, 0);
    for (auto& e : entries)
        e.button->setBounds (a.removeFromTop (24).reduced (0, 1));
}

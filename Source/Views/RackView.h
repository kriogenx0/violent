#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "Generators.h"
#include "FxBusView.h"
#include "MasterFilterView.h"
#include "MixerView.h"

//==============================================================================
/** The generator rack, master filters, and mixer — laid out at a fixed logical
    width (BASE_WIDTH). The owning editor scales this whole component uniformly
    via a transform so '+'/'-' zoom controls can grow or shrink the rack without
    every child needing to know about the current zoom level. The header toolbar
    (title, presets, preview, meter) lives on the editor itself and stays fixed
    size regardless of zoom, so it never collides with the zoom controls. */
class ScalableRackComponent : public juce::Component
{
public:
    static constexpr int BASE_WIDTH = 960;

    explicit ScalableRackComponent (ViolentAudioProcessor& p);

    void resized() override;

    int preferredHeight() const noexcept;

    // Tears every panel down and rebuilds it from processor state, e.g. after a preset load.
    void refreshFromState();

    // Fired when preferredHeight() changes (generator/filter added or removed),
    // so the owning editor can re-run its own zoom-aware sizing.
    std::function<void()> onLayoutChanged;

    // For the nav panel.
    GeneratorPanel& getGeneratorPanel() { return generatorPanel; }
    FxBusPanel& getFxBusPanel() { return fxBusPanel; }
    MasterFilterPanel& getMasterFilterPanel() { return masterFilterPanel; }

private:
    ViolentAudioProcessor& processor;

    GeneratorPanel generatorPanel;
    FxBusPanel fxBusPanel;
    MasterFilterPanel masterFilterPanel;
    MixerPanel mixerPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScalableRackComponent)
};

//==============================================================================
/** Sizes itself to the rack's scaled (post-zoom) dimensions and applies the
    zoom transform to the rack, so a juce::Viewport wrapped around this sees
    the true on-screen size for its scrollbar range — a Viewport reads its
    content's getWidth()/getHeight() directly and knows nothing about
    transforms applied further down the tree. */
class RackScaler : public juce::Component
{
public:
    explicit RackScaler (ScalableRackComponent& r) : rack (r) { addAndMakeVisible (rack); }

    void updateLayout (float scale)
    {
        const int rackH = rack.preferredHeight();
        rack.setBounds (0, 0, ScalableRackComponent::BASE_WIDTH, rackH);
        rack.setTransform (juce::AffineTransform::scale (scale));
        setSize (juce::roundToInt (ScalableRackComponent::BASE_WIDTH * scale),
                 juce::roundToInt (rackH * scale));
    }

private:
    ScalableRackComponent& rack;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RackScaler)
};

//==============================================================================
/** Left-side outline of every module in the rack — MIDI modifiers,
    generators, filters, and effects — labelled by type only. Clicking an
    entry scrolls the rack viewport so that module comes into view. */
class NavPanel : public juce::Component
{
public:
    static constexpr int WIDTH = 132;

    NavPanel (ViolentAudioProcessor& p, ScalableRackComponent& rack,
              juce::Component& scrollSpace, juce::Viewport& viewport);

    void resized() override;
    void paint (juce::Graphics& g) override;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds the entry list from the current rack structure.
    void refreshFromState();

private:
    ViolentAudioProcessor& processor;
    ScalableRackComponent& rack;
    juce::Component& scrollSpace; // coordinate space scroll targets are resolved in (the viewport's content)
    juce::Viewport& viewport;

    struct Entry
    {
        std::unique_ptr<juce::TextButton> button;
        juce::Component::SafePointer<juce::Component> target;
    };
    std::vector<Entry> entries;

    void addEntry (const juce::String& label, juce::Component* target);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavPanel)
};


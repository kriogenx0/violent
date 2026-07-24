#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "MidiModifierView.h"
#include "Generators.h"
#include "EffectView.h"
#include "MixerView.h"

//==============================================================================
/** The whole rack, laid out at a fixed logical width (BASE_WIDTH), top to
    bottom: MIDI Modifiers, Generators, (Phase 2: Modulators), Effects, then
    the Mixer. Routing lives directly on each MIDI Modifier/Effect row rather
    than as a separate router panel. The owning editor scales this whole
    component uniformly via a transform so '+'/'-' zoom controls can grow or
    shrink the rack without every child needing to know about the current
    zoom level. The header toolbar (title, presets, preview, meter) lives on
    the editor itself and stays fixed size regardless of zoom, so it never
    collides with the zoom controls. */
class ScalableRackComponent : public juce::Component
{
public:
    static constexpr int BASE_WIDTH = 960;

    explicit ScalableRackComponent (ViolentAudioProcessor& p);

    void resized() override;

    int preferredHeight() const noexcept;

    // Tears every panel down and rebuilds it from processor state, e.g. after a preset load.
    void refreshFromState();

    // Fired when preferredHeight() changes (a component added or removed),
    // so the owning editor can re-run its own zoom-aware sizing.
    std::function<void()> onLayoutChanged;

    // For the minimap.
    MidiModifierPanel& getMidiModifierPanel() { return midiModifierPanel; }
    GeneratorPanel& getGeneratorPanel() { return generatorPanel; }
    EffectPanel& getEffectPanel() { return effectPanel; }
    MixerPanel& getMixerPanel() { return mixerPanel; }

private:
    ViolentAudioProcessor& processor;

    MidiModifierPanel midiModifierPanel;
    GeneratorPanel generatorPanel;
    EffectPanel effectPanel;
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
/** Left-side outline of every component in the rack — MIDI modifiers,
    generators, effects, and the mixer — labelled by name. Clicking an entry
    scrolls the rack viewport so that component comes into view. */
class Minimap : public juce::Component
{
public:
    static constexpr int WIDTH = 132;

    Minimap (ViolentAudioProcessor& p, ScalableRackComponent& rack,
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Minimap)
};

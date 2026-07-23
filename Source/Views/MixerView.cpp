#include "MixerView.h"

//==============================================================================
// MixerChannel
//==============================================================================
MixerChannel::MixerChannel (ViolentAudioProcessor& p, int generatorIdx)
    : processor (p), generator (generatorIdx)
{
    nameLabel.setText ("Gen " + juce::String (generator + 1), juce::dontSendNotification);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (nameLabel);

    levelSlider.setSliderStyle (juce::Slider::LinearVertical);
    levelSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible (levelSlider);
    levelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, ParamIDs::generatorLevel (generator), levelSlider);

    startTimerHz (20);
}

void MixerChannel::resized()
{
    auto a = getLocalBounds().reduced (4, 4);
    nameLabel.setBounds (a.removeFromTop (16));
    a.removeFromTop (2);
    levelSlider.setBounds (a);
}

void MixerChannel::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (b.reduced (1.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (b.reduced (1.0f), 5.0f, 1.0f);

    const bool enabled = processor.generators[(size_t) generator].enabled;
    const float level  = processor.generatorLevelMeter[(size_t) generator].load (std::memory_order_relaxed);
    const float db     = juce::Decibels::gainToDecibels (juce::jmax (0.0001f, level));
    const float norm   = juce::jlimit (0.0f, 1.0f, (db + 48.0f) / 48.0f);

    auto meterArea = b.removeFromRight (7.0f).reduced (1.5f, 22.0f);
    g.setColour (ViolentColours::background);
    g.fillRoundedRectangle (meterArea, 2.0f);
    g.setColour (enabled ? ViolentColours::green : ViolentColours::overlay);
    g.fillRoundedRectangle (meterArea.withTop (meterArea.getBottom() - meterArea.getHeight() * norm), 2.0f);
}

//==============================================================================
// MixerPanel
//==============================================================================
MixerPanel::MixerPanel (ViolentAudioProcessor& p) : processor (p)
{
    sectionLabel.setText ("MIXER", juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    rebuild();
}

void MixerPanel::rebuild()
{
    for (int s = 0; s < MAX_GENERATORS; ++s)
    {
        if (s < processor.numActiveGenerators)
        {
            if (!channels[(size_t) s])
            {
                channels[(size_t) s] = std::make_unique<MixerChannel> (processor, s);
                addAndMakeVisible (*channels[(size_t) s]);
            }
        }
        else
        {
            channels[(size_t) s] = nullptr;
        }
    }
    resized();
}

void MixerPanel::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);
}

void MixerPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    sectionLabel.setBounds (a.removeFromTop (20));
    a.removeFromTop (2);

    constexpr int chanW = 64;
    for (int s = 0; s < processor.numActiveGenerators; ++s)
    {
        if (!channels[(size_t) s]) continue;
        channels[(size_t) s]->setBounds (a.removeFromLeft (chanW));
        a.removeFromLeft (6);
    }
}


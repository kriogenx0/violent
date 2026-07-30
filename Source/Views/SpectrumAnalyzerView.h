#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** Live magnitude spectrum of the master output (log frequency, 20Hz-20kHz,
    on the x-axis; dB on the y-axis), polled from the processor's FFT
    analysis the same way WaveformView polls its ring buffer. */
class SpectrumAnalyzerView : public juce::Component, private juce::Timer
{
public:
    explicit SpectrumAnalyzerView (ViolentAudioProcessor& p) : processor (p) { startTimerHz (30); }

    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override { repaint(); }

    ViolentAudioProcessor& processor;
};

//==============================================================================
/** Collapsible panel wrapping the analyzer — sits after the Mixer at the
    bottom of the rack. */
class SpectrumAnalyzerPanel : public juce::Component
{
public:
    static constexpr int HEADER_H  = 28;
    static constexpr int CONTENT_H = 160;

    explicit SpectrumAnalyzerPanel (ViolentAudioProcessor& p);

    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    int preferredHeight() const noexcept;

    std::function<void()> onLayoutChanged;

private:
    void updateHeaderText();

    ViolentAudioProcessor& processor;
    juce::TextButton headerBtn;
    SpectrumAnalyzerView analyzerView;
    bool collapsed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumAnalyzerPanel)
};

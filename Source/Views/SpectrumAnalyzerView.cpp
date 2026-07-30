#include "SpectrumAnalyzerView.h"

//==============================================================================
// SpectrumAnalyzerView
//==============================================================================
void SpectrumAnalyzerView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.setColour (ViolentColours::background);
    g.fillRoundedRectangle (b, 4.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

    auto plot = b.reduced (6.0f, 6.0f);

    constexpr float minDb  = -80.0f, maxDb = 0.0f;
    constexpr float logMin = 1.30103f, logMax = 4.30103f; // log10(20), log10(20000)
    const double sampleRate = processor.getSampleRate() > 0.0 ? processor.getSampleRate() : 44100.0;
    constexpr int numBins = ViolentAudioProcessor::SPECTRUM_FFT_SIZE / 2;

    juce::Path path;
    constexpr int steps = 128;
    for (int i = 0; i <= steps; ++i)
    {
        const float freq = std::pow (10.0f, logMin + (logMax - logMin) * (float) i / (float) steps);
        const int bin = juce::jlimit (0, numBins - 1,
            (int) std::round (freq * (float) ViolentAudioProcessor::SPECTRUM_FFT_SIZE / (float) sampleRate));
        const float db   = processor.spectrumMagnitudesDb[(size_t) bin];
        const float norm = juce::jlimit (0.0f, 1.0f, (db - minDb) / (maxDb - minDb));
        const float px = plot.getX() + plot.getWidth() * (float) i / (float) steps;
        const float py = plot.getBottom() - norm * plot.getHeight();
        if (i == 0) path.startNewSubPath (px, py); else path.lineTo (px, py);
    }
    g.setColour (ViolentColours::accent);
    g.strokePath (path, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved));
}

//==============================================================================
// SpectrumAnalyzerPanel
//==============================================================================
SpectrumAnalyzerPanel::SpectrumAnalyzerPanel (ViolentAudioProcessor& p)
    : processor (p), analyzerView (p)
{
    updateHeaderText();
    headerBtn.onClick = [this]
    {
        collapsed = ! collapsed;
        updateHeaderText();
        analyzerView.setVisible (! collapsed);
        if (onLayoutChanged) onLayoutChanged();
    };
    addAndMakeVisible (headerBtn);

    addAndMakeVisible (analyzerView);
}

void SpectrumAnalyzerPanel::updateHeaderText()
{
    const juce::String chevron = collapsed ? juce::String (juce::CharPointer_UTF8 ("\xE2\x96\xB8"))
                                            : juce::String (juce::CharPointer_UTF8 ("\xE2\x96\xBE"));
    headerBtn.setButtonText ("SPECTRUM ANALYZER  " + chevron);
}

int SpectrumAnalyzerPanel::preferredHeight() const noexcept
{
    return 8 + HEADER_H + (collapsed ? 0 : (4 + CONTENT_H));
}

void SpectrumAnalyzerPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    headerBtn.setBounds (a.removeFromTop (HEADER_H));

    if (! collapsed)
    {
        a.removeFromTop (4);
        analyzerView.setBounds (a.removeFromTop (CONTENT_H));
    }
}

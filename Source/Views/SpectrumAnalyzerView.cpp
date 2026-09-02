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
    // Leave room below the plot for frequency labels, and shrink the plot's
    // left/top so dB gridline labels don't sit on top of the curve.
    plot.removeFromBottom (11.0f);

    constexpr float minDb  = -80.0f, maxDb = 0.0f;
    constexpr float logMin = 1.30103f, logMax = 4.30103f; // log10(20), log10(20000)
    const double sampleRate = processor.getSampleRate() > 0.0 ? processor.getSampleRate() : 44100.0;
    constexpr int numBins = ViolentAudioProcessor::SPECTRUM_FFT_SIZE / 2;

    g.setFont (juce::Font (juce::FontOptions().withHeight (9.0f)));

    // dB gridlines + labels (horizontal)
    for (float db = maxDb; db >= minDb; db -= 20.0f)
    {
        const float y = plot.getBottom() - (db - minDb) / (maxDb - minDb) * plot.getHeight();
        g.setColour (ViolentColours::overlay.withAlpha (0.6f));
        g.drawHorizontalLine ((int) y, plot.getX(), plot.getRight());
        g.setColour (ViolentColours::subtext);
        g.drawText (juce::String ((int) db), (int) plot.getX() + 2, (int) y - 10, 34, 10,
                    juce::Justification::left);
    }

    // Frequency gridlines + labels (vertical, log-scaled)
    struct Tick { float hz; bool labelled; };
    const Tick ticks[] = { { 20.0f, true }, { 50.0f, false }, { 100.0f, true }, { 200.0f, false },
                            { 500.0f, false }, { 1000.0f, true }, { 2000.0f, false }, { 5000.0f, false },
                            { 10000.0f, true }, { 20000.0f, true } };
    for (auto& t : ticks)
    {
        const float x = plot.getX() + plot.getWidth() * (std::log10 (t.hz) - logMin) / (logMax - logMin);
        g.setColour (ViolentColours::overlay.withAlpha (0.6f));
        g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());

        if (! t.labelled) continue;
        const juce::String label = t.hz >= 1000.0f
            ? juce::String (t.hz / 1000.0f, 0) + "k"
            : juce::String ((int) t.hz);
        g.setColour (ViolentColours::subtext);
        g.drawText (label, (int) x - 15, (int) plot.getBottom() + 1, 30, 10, juce::Justification::centred);
    }

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
    : processor (p), analyzerView (p), masterMeter (p)
{
    updateHeaderText();
    headerBtn.onClick = [this]
    {
        collapsed = ! collapsed;
        updateHeaderText();
        analyzerView.setVisible (! collapsed);
        masterMeter.setVisible (! collapsed);
        if (onLayoutChanged) onLayoutChanged();
    };
    addAndMakeVisible (headerBtn);

    addAndMakeVisible (analyzerView);
    addAndMakeVisible (masterMeter);
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
        masterMeter.setBounds (a.removeFromRight (60).reduced (4, 0));
        a.removeFromRight (4);
        analyzerView.setBounds (a.removeFromTop (CONTENT_H));
    }
}

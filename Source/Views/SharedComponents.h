#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"

//==============================================================================
namespace ViolentColours
{
    static const juce::Colour background { 0xff0a0a0a };
    static const juce::Colour surface    { 0xff1a1a1a };
    static const juce::Colour overlay    { 0xff2c2c2c };
    static const juce::Colour text       { 0xffe8e8e8 };
    static const juce::Colour subtext    { 0xff8c8c8c };
    static const juce::Colour accent     { 0xffff7a29 };
    static const juce::Colour green      { 0xffa6e3a1 };
    static const juce::Colour red        { 0xfff38ba8 };
    static const juce::Colour yellow     { 0xfff9e2af };
    static const juce::Colour blue       { 0xff89b4fa };
    static const juce::Colour teal       { 0xff94e2d5 };

    static constexpr float cornerRadius = 6.0f;
}

//==============================================================================
class ViolentUI : public juce::LookAndFeel_V4, private juce::Timer
{
public:
    ViolentUI();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float, float,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                        int buttonX, int buttonY, int buttonW, int buttonH,
                        juce::ComboBox&) override;
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;

private:
    /** Shared background+border rendering for TextButton/ToggleButton/ComboBox,
        so all three read as one consistent control style, including hover. */
    void paintControlShape (juce::Graphics&, juce::Component&, juce::Rectangle<float> bounds,
                             bool isOn, bool isHovered);

    // Hover/glow states fade in and out over ~100ms rather than snapping, so
    // every control that reads hover state goes through this: it returns a
    // 0..1 "how hovered" amount for `c` that eases towards isHovered's target,
    // storing its animation state directly on the component (so it survives
    // regardless of which LookAndFeel method last touched it) and keeping
    // this timer running only while at least one control is mid-transition.
    float animatedHoverAmount (juce::Component& c, bool isHovered);
    void registerForHoverAnimation (juce::Component& c);
    void timerCallback() override;

    juce::Array<juce::Component::SafePointer<juce::Component>> animatingComponents;
};

//==============================================================================
/** A standard button (same rounded-rect chrome as everything else) with a
    drawn trash-can glyph instead of text — used for destructive "remove". */
class TrashButton : public juce::TextButton
{
public:
    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        getLookAndFeel().drawButtonBackground (g, *this,
            findColour (juce::TextButton::buttonColourId), highlighted, down);

        auto sq = getLocalBounds().toFloat().withSizeKeepingCentre (
            juce::jmin (getWidth(), getHeight()) * 0.55f, juce::jmin (getWidth(), getHeight()) * 0.55f);
        auto b = sq.reduced (sq.getWidth() * 0.24f, sq.getHeight() * 0.14f);
        g.setColour (ViolentColours::red);

        juce::Path p;
        const float lidY = b.getY() + b.getHeight() * 0.16f;
        p.addRoundedRectangle (b.getX() - 1.0f, lidY, b.getWidth() + 2.0f, b.getHeight() * 0.14f, 1.0f);
        p.addRoundedRectangle (b.getX() + b.getWidth() * 0.28f, b.getY(),
                                b.getWidth() * 0.44f, b.getHeight() * 0.16f, 1.0f);
        const auto body = b.withY (lidY + b.getHeight() * 0.18f)
                            .withHeight (b.getHeight() * 0.7f);
        p.addRoundedRectangle (body, 1.2f);
        g.strokePath (p, juce::PathStrokeType (1.1f));

        for (float fx : { 0.32f, 0.5f, 0.68f })
            g.drawLine (b.getX() + b.getWidth() * fx, body.getY() + 3.0f,
                        b.getX() + b.getWidth() * fx, body.getBottom() - 3.0f, 0.9f);
    }
};

//==============================================================================
/** LP / HP / BP / Notch selector — four buttons standing in for what used to
    be a dropdown, bound directly to an AudioParameterChoice via a
    ParameterAttachment (so it works like any other custom parameter control). */
class FilterTypeSelector : public juce::Component
{
public:
    FilterTypeSelector (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
        : attachment (*apvts.getParameter (paramID),
                       [this] (float v) { setSelectedIndex ((int) std::round (v)); },
                       apvts.undoManager)
    {
        static const char* names[] = { "LP", "HP", "BP", "Notch" };
        for (int i = 0; i < 4; ++i)
        {
            auto& b = buttons[(size_t) i];
            b.setButtonText (names[i]);
            b.setClickingTogglesState (false);
            b.onClick = [this, i] { attachment.setValueAsCompleteGesture ((float) i); };
            addAndMakeVisible (b);
        }
        attachment.sendInitialUpdate();
    }

    void resized() override
    {
        auto a = getLocalBounds();
        const int w = a.getWidth() / 4;
        for (int i = 0; i < 4; ++i)
            buttons[(size_t) i].setBounds ((i < 3 ? a.removeFromLeft (w) : a).reduced (1, 0));
    }

private:
    void setSelectedIndex (int idx)
    {
        for (int i = 0; i < 4; ++i)
            buttons[(size_t) i].setToggleState (i == idx, juce::dontSendNotification);
    }

    std::array<juce::TextButton, 4> buttons;
    juce::ParameterAttachment attachment;
};

//==============================================================================
/** Draws the approximate magnitude response of a 2nd-order LP/HP/BP/Notch
    filter (20Hz-20kHz, log frequency) with the cutoff marked, polling the
    live parameter values so it tracks knob moves and automation alike. */
class FilterResponseView : public juce::Component, private juce::Timer
{
public:
    FilterResponseView (juce::AudioProcessorValueTreeState& s, juce::String typeID,
                         juce::String cutID, juce::String resID)
        : apvts (s), typeParamID (std::move (typeID)),
          cutParamID (std::move (cutID)), resParamID (std::move (resID))
    {
        startTimerHz (20);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (ViolentColours::background);
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (ViolentColours::overlay);
        g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

        const int   type   = (int) apvts.getRawParameterValue (typeParamID)->load();
        const float cutoff = juce::jlimit (20.0f, 20000.0f, apvts.getRawParameterValue (cutParamID)->load());
        const float q      = juce::jmax (0.1f, apvts.getRawParameterValue (resParamID)->load());

        auto plot = b.reduced (4.0f);
        constexpr float minDb = -24.0f, maxDb = 6.0f;
        constexpr float logMin = 1.30103f, logMax = 4.30103f; // log10(20), log10(20000)

        auto dbAt = [&] (float freq)
        {
            const float x  = freq / cutoff;
            const float x2 = x * x;
            const float denom = (1.0f - x2) * (1.0f - x2) + (x2 / (q * q));
            float magSq;
            switch (type)
            {
                case 1:  magSq = (x2 * x2) / denom; break;                    // HP
                case 2:  magSq = (x2 / (q * q)) / denom; break;               // BP
                case 3:  magSq = ((1.0f - x2) * (1.0f - x2)) / denom; break;  // Notch
                default: magSq = 1.0f / denom; break;                        // LP
            }
            return 10.0f * std::log10 (juce::jmax (1.0e-6f, magSq));
        };

        juce::Path path;
        constexpr int steps = 96;
        for (int i = 0; i <= steps; ++i)
        {
            const float freq = std::pow (10.0f, logMin + (logMax - logMin) * (float) i / (float) steps);
            const float norm = juce::jlimit (0.0f, 1.0f, (dbAt (freq) - minDb) / (maxDb - minDb));
            const float px = plot.getX() + plot.getWidth() * (float) i / (float) steps;
            const float py = plot.getBottom() - norm * plot.getHeight();
            if (i == 0) path.startNewSubPath (px, py); else path.lineTo (px, py);
        }
        g.setColour (ViolentColours::accent);
        g.strokePath (path, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved));

        const float cutNorm = (std::log10 (cutoff) - logMin) / (logMax - logMin);
        const float cutX = plot.getX() + plot.getWidth() * cutNorm;
        g.setColour (ViolentColours::accent.withAlpha (0.4f));
        g.drawVerticalLine ((int) cutX, plot.getY(), plot.getBottom());
    }

private:
    void timerCallback() override { repaint(); }

    juce::AudioProcessorValueTreeState& apvts;
    juce::String typeParamID, cutParamID, resParamID;
};

//==============================================================================
/** Live scope of a generator's raw source waveform (pre-filter/FX), polled
    from the audio-thread ring buffer the processor keeps per generator. The
    small button in the corner cycles how much time the scope displays. */
class WaveformView : public juce::Component, private juce::Timer
{
public:
    WaveformView (ViolentAudioProcessor& p, int generatorIdx)
        : processor (p), generator (generatorIdx)
    {
        windowBtn.setButtonText (windowLabel());
        windowBtn.onClick = [this]
        {
            windowIndex = (windowIndex + 1) % ViolentAudioProcessor::NUM_WAVEFORM_WINDOWS;
            windowBtn.setButtonText (windowLabel());
            repaint();
        };
        addAndMakeVisible (windowBtn);

        startTimerHz (24);
    }

    void resized() override
    {
        windowBtn.setBounds (getLocalBounds().removeFromRight (38).removeFromTop (14).reduced (1));
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (ViolentColours::background);
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (ViolentColours::overlay);
        g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

        auto plot = b.reduced (5.0f, 4.0f);
        g.setColour (ViolentColours::overlay);
        g.drawHorizontalLine ((int) plot.getCentreY(), plot.getX(), plot.getRight());

        if (processor.waveformRingSize <= 0)
            return;

        const auto& ring = processor.waveformRing[(size_t) generator];
        const int   ringSize = processor.waveformRingSize;
        const int   writePos = processor.waveformRingWritePos[(size_t) generator];
        const float windowMs = ViolentAudioProcessor::waveformWindowOptionsMs[(size_t) windowIndex];
        const int   n = juce::jlimit (2, ringSize,
            (int) std::round (windowMs * 0.001 * processor.getSampleRate()));

        // Cap plotted points to roughly 2 per pixel so long windows at high
        // sample rates don't build paths with tens of thousands of segments.
        const int step = juce::jmax (1, n / juce::jmax (1, (int) plot.getWidth() * 2));

        juce::Path path;
        bool first = true;
        for (int i = 0; i < n; i += step)
        {
            const int ringIdx = ((writePos - n + i) % ringSize + ringSize) % ringSize;
            const float x = plot.getX() + plot.getWidth() * (float) i / (float) (n - 1);
            const float y = plot.getCentreY() - juce::jlimit (-1.0f, 1.0f, ring[(size_t) ringIdx]) * plot.getHeight() * 0.5f;
            if (first) { path.startNewSubPath (x, y); first = false; } else path.lineTo (x, y);
        }
        g.setColour (ViolentColours::accent);
        g.strokePath (path, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved));
    }

private:
    void timerCallback() override { repaint(); }

    juce::String windowLabel() const
    {
        const float ms = ViolentAudioProcessor::waveformWindowOptionsMs[(size_t) windowIndex];
        if (ms < 1000.0f)
            return juce::String ((int) ms) + "ms";

        const float s = ms / 1000.0f;
        const bool  isWhole = juce::approximatelyEqual (s, std::round (s));
        return (isWhole ? juce::String ((int) s) : juce::String (s, 1)) + "s";
    }

    ViolentAudioProcessor& processor;
    int generator;
    int windowIndex = 1; // 10ms by default
    juce::TextButton windowBtn;
};

//==============================================================================
/** Rotary knob with name above and value below, always visible. Owns its own
    APVTS attachment (bind with attachTo()) so every call site gets consistent
    wiring instead of each owner hand-rolling a SliderAttachment. */
class LabelledKnob : public juce::Component
{
public:
    juce::Slider slider;
    juce::Label  nameLabel;
    juce::Label  valueLabel;

    LabelledKnob (const juce::String& name, juce::Colour colour = ViolentColours::accent);
    void resized() override;
    void paint (juce::Graphics&) override {}

    // Binds this knob to an APVTS parameter and primes the value label with
    // the current value. JUCE's SliderAttachment silently skips its initial
    // change notification when the parameter's default already equals the
    // slider's pre-attachment value (e.g. 0), which otherwise leaves the
    // label blank until the user first drags the knob — so the label is set
    // explicitly here rather than relying on that notification.
    void attachTo (juce::AudioProcessorValueTreeState& apvts, const juce::String& parameterID);

private:
    juce::Colour knobColour;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

//==============================================================================
/** Level meter bar (L+R), polled via timer. */
class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter() { startTimerHz (30); }
    void setLevels (float l, float r) noexcept { peakL = l; peakR = r; }
    void paint (juce::Graphics& g) override;

private:
    float peakL = 0.0f, peakR = 0.0f;
    void timerCallback() override { repaint(); }
};

//==============================================================================
/** The controls shared by every filter row: remove button, name, type
    selector, cutoff/resonance knobs, and a live frequency-response view.
    Used by MasterFilterRow (master chain, which adds its own routing row
    below); per-generator filters are just another GeneratorFxCard type. */
class FilterControlsBlock : public juce::Component
{
public:
    static constexpr int HEIGHT = 108;

    FilterControlsBlock (juce::AudioProcessorValueTreeState& apvts, const juce::String& name,
                          const juce::String& typeParamID,
                          const juce::String& cutoffParamID,
                          const juce::String& resParamID);

    void resized() override;

    std::function<void()> onRemove;

private:
    TrashButton      removeBtn;
    juce::Label      nameLabel;
    FilterTypeSelector typeSelector;
    LabelledKnob     cutoffKnob { "Cutoff",    ViolentColours::blue   };
    LabelledKnob     resKnob    { "Resonance", ViolentColours::accent };
    FilterResponseView responseView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterControlsBlock)
};


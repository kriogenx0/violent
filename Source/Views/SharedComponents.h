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

    static constexpr float cornerRadius = 8.0f;
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
    drawn "X" glyph instead of text — used for destructive "remove", in the
    top-left corner of every component. */
class DeleteButton : public juce::TextButton
{
public:
    void paintButton (juce::Graphics& g, bool highlighted, bool down) override
    {
        getLookAndFeel().drawButtonBackground (g, *this,
            findColour (juce::TextButton::buttonColourId), highlighted, down);

        const float side = (float) juce::jmin (getWidth(), getHeight()) * 0.4f;
        auto b = getLocalBounds().toFloat().withSizeKeepingCentre (side, side);
        g.setColour (ViolentColours::red);
        g.drawLine (b.getX(), b.getY(), b.getRight(), b.getBottom(), 1.8f);
        g.drawLine (b.getX(), b.getBottom(), b.getRight(), b.getY(), 1.8f);
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
        // Visual boost so quiet/attenuated signals are still clearly readable
        // as a scope trace - this is a preview, not a level meter.
        constexpr float visualGain = 3.0f;
        for (int i = 0; i < n; i += step)
        {
            const int ringIdx = ((writePos - n + i) % ringSize + ringSize) % ringSize;
            const float x = plot.getX() + plot.getWidth() * (float) i / (float) (n - 1);
            const float y = plot.getCentreY() - juce::jlimit (-1.0f, 1.0f, ring[(size_t) ringIdx] * visualGain) * plot.getHeight() * 0.5f;
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
/** Static overview of a generator's loaded sample file — a min/max outline
    of the whole buffer, unlike WaveformView's live scrolling scope, since a
    sample's shape doesn't change while it plays. Rebuilt only when a new
    sample is loaded (refresh()), not on a timer. */
class SampleWaveformView : public juce::Component
{
public:
    SampleWaveformView (ViolentAudioProcessor& p, int generatorIdx)
        : processor (p), generator (generatorIdx)
    {
        refresh();
    }

    void refresh()
    {
        outline.clear();

        auto& module = processor.getSampler().modules[(size_t) generator];
        const juce::SpinLock::ScopedLockType sl (module.lock);

        const int numSamples = module.buffer.getNumSamples();
        if (module.hasData && numSamples > 0)
        {
            const int numChannels = module.buffer.getNumChannels();
            constexpr int numColumns = 128;
            outline.reserve ((size_t) numColumns);

            for (int c = 0; c < numColumns; ++c)
            {
                const int start = (int) ((int64_t) c * numSamples / numColumns);
                const int end   = juce::jmax (start + 1, (int) ((int64_t) (c + 1) * numSamples / numColumns));
                float lo = 0.0f, hi = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    const auto range = module.buffer.findMinMax (ch, start, end - start);
                    lo = juce::jmin (lo, range.getStart());
                    hi = juce::jmax (hi, range.getEnd());
                }
                outline.push_back ({ lo, hi });
            }
        }

        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (ViolentColours::background);
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (ViolentColours::overlay);
        g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

        auto plot = b.reduced (5.0f, 4.0f);

        if (outline.empty())
        {
            g.setColour (ViolentColours::subtext);
            g.setFont (juce::Font (juce::FontOptions().withHeight (10.0f)));
            g.drawText ("No sample loaded", plot.toNearestInt(), juce::Justification::centred);
            return;
        }

        g.setColour (ViolentColours::overlay);
        g.drawHorizontalLine ((int) plot.getCentreY(), plot.getX(), plot.getRight());

        const int n = (int) outline.size();
        juce::Path path;
        for (int i = 0; i < n; ++i)
        {
            const float x = plot.getX() + plot.getWidth() * (float) i / (float) (n - 1);
            const float y = plot.getCentreY() - outline[(size_t) i].second * plot.getHeight() * 0.5f;
            if (i == 0) path.startNewSubPath (x, y); else path.lineTo (x, y);
        }
        for (int i = n - 1; i >= 0; --i)
        {
            const float x = plot.getX() + plot.getWidth() * (float) i / (float) (n - 1);
            path.lineTo (x, plot.getCentreY() - outline[(size_t) i].first * plot.getHeight() * 0.5f);
        }
        path.closeSubPath();

        g.setColour (ViolentColours::accent);
        g.fillPath (path);
    }

private:
    ViolentAudioProcessor& processor;
    int generator;
    std::vector<std::pair<float, float>> outline;
};

//==============================================================================
/** A rotary Slider whose effective drag distance is clamped to a small
    margin beyond its own bounds — dragging the cursor further away has no
    additional effect, so the cursor never wanders far from the knob it's
    adjusting, even across a long drag. */
class ConstrainedKnobSlider : public juce::Slider
{
public:
    void resized() override
    {
        Slider::resized();

        // Slider's default drag distance for the full 0..1 range is 250px
        // (see Slider::Pimpl::pixelsForFullDragExtent) — far more than the
        // small area mouseDrag() clamps the cursor to below. Without this,
        // the clamp leaves only a small fraction of the range reachable
        // before the knob stops responding, which feels like the cursor has
        // come loose from the control. Matching the sensitivity to the
        // clamped travel distance keeps the whole range reachable within it.
        setMouseDragSensitivity (juce::jmax (10, juce::jmin (getWidth(), getHeight())
                                                      + (int) (margin * 2.0f)));
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const auto bounds = getLocalBounds().toFloat().expanded (margin);
        Slider::mouseDrag (e.withNewPosition (bounds.getConstrainedPoint (e.position)));
    }

private:
    static constexpr float margin = 10.0f;
};

//==============================================================================
/** Rotary knob with name above and value below, always visible. Owns its own
    APVTS attachment (bind with attachTo()) so every call site gets consistent
    wiring instead of each owner hand-rolling a SliderAttachment. Also polls
    whether it's the current target of any enabled Modulator and, if so,
    renders its fill in a vibrant purple instead of its usual colour. */
class LabelledKnob : public juce::Component, private juce::Timer
{
public:
    ConstrainedKnobSlider slider;
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
    juce::String paramID;
    ViolentAudioProcessor* ownerProcessor = nullptr;

    void timerCallback() override;
};

//==============================================================================
/** ADSR curve with a playhead that sweeps left to right in real time and
    loops continuously, so the envelope's shape is visible without needing a
    live note. Reads the four sliders fresh on every repaint (there's no
    fixed "note held" duration to sync to, so the sustain segment uses a
    constant visual hold instead). */
/** Snapshot of an envelope's real activity, so EnvelopeGraphView's playhead
    can sync to actual note timing instead of animating a demo loop
    regardless of whether anything is playing. */
struct EnvelopeActivity
{
    bool active = false;            // any voice/state still in attack/decay/sustain/release
    bool held   = false;            // still before note-off (attack/decay/sustain)
    juce::uint32 startMs = 0;        // when it last entered Attack (meaningful if held)
    juce::uint32 releaseStartMs = 0; // when it last entered Release (meaningful if !held)
};

class EnvelopeGraphView : public juce::Component, private juce::Timer
{
public:
    EnvelopeGraphView (juce::Slider& att, juce::Slider& dec, juce::Slider& sus, juce::Slider& rel,
                        std::function<EnvelopeActivity()> activityIn)
        : attSlider (att), decSlider (dec), susSlider (sus), relSlider (rel), getActivity (std::move (activityIn))
    {
        startTimerHz (30);
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (ViolentColours::background);
        g.fillRoundedRectangle (b, 4.0f);
        g.setColour (ViolentColours::overlay);
        g.drawRoundedRectangle (b.reduced (0.5f), 4.0f, 1.0f);

        auto plot = b.reduced (5.0f, 4.0f);

        const float att = (float) attSlider.getValue();
        const float dec = (float) decSlider.getValue();
        const float sus = (float) susSlider.getValue();
        const float rel = (float) relSlider.getValue();
        constexpr float holdS = 0.6f;
        const float total = juce::jmax (0.05f, att + dec + holdS + rel);

        auto levelAt = [&] (float t) -> float
        {
            if (t < att) return att > 0.0f ? t / att : 1.0f;
            t -= att;
            if (t < dec) return dec > 0.0f ? 1.0f - (1.0f - sus) * (t / dec) : sus;
            t -= dec;
            if (t < holdS) return sus;
            t -= holdS;
            if (t < rel) return rel > 0.0f ? sus * (1.0f - t / rel) : 0.0f;
            return 0.0f;
        };

        juce::Path path;
        constexpr int steps = 96;
        for (int i = 0; i <= steps; ++i)
        {
            const float t = total * (float) i / (float) steps;
            const float x = plot.getX() + plot.getWidth() * (float) i / (float) steps;
            const float y = plot.getBottom() - levelAt (t) * plot.getHeight();
            if (i == 0) path.startNewSubPath (x, y); else path.lineTo (x, y);
        }

        auto fillPath = path;
        fillPath.lineTo (plot.getRight(), plot.getBottom());
        fillPath.lineTo (plot.getX(), plot.getBottom());
        fillPath.closeSubPath();
        g.setGradientFill (juce::ColourGradient (ViolentColours::accent.withAlpha (0.35f), 0, plot.getY(),
                                                  ViolentColours::accent.withAlpha (0.0f), 0, plot.getBottom(), false));
        g.fillPath (fillPath);

        g.setColour (ViolentColours::accent);
        g.strokePath (path, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved));

        const auto activity = getActivity ? getActivity() : EnvelopeActivity {};
        if (! activity.active) return; // idle: no playhead, no animation

        const auto now = juce::Time::getMillisecondCounter();
        // Held (attack/decay/sustain) clamps into the sustain plateau rather
        // than running on into a fake release - real sustain holds for as
        // long as the note does, which this can't predict. Releasing plays
        // out the release ramp from when note-off actually happened, then
        // holds at the end rather than looping back to attack.
        const float t = activity.held
            ? juce::jmin ((float) (now - activity.startMs) * 0.001f, att + dec + holdS)
            : juce::jmin (att + dec + holdS + (float) (now - activity.releaseStartMs) * 0.001f, total);

        const float px = plot.getX() + plot.getWidth() * (t / total);
        const float py = plot.getBottom() - levelAt (t) * plot.getHeight();
        g.setColour (ViolentColours::text.withAlpha (0.5f));
        g.drawVerticalLine ((int) px, plot.getY(), plot.getBottom());
        g.setColour (ViolentColours::text);
        g.fillEllipse (px - 3.0f, py - 3.0f, 6.0f, 6.0f);
    }

private:
    void timerCallback() override { repaint(); }

    juce::Slider& attSlider, & decSlider, & susSlider, & relSlider;
    std::function<EnvelopeActivity()> getActivity;
};

//==============================================================================
/** LFO shape preview at its real rate, with a playhead that sweeps left to
    right in sync with actual phase and loops. Shape/rate are read fresh on
    every repaint so it tracks the shape ComboBox/rate knob immediately. */
class LfoGraphView : public juce::Component, private juce::Timer
{
public:
    LfoGraphView (juce::Slider& rate, juce::ComboBox& shape)
        : rateSlider (rate), shapeBox (shape)
    {
        startTimerHz (30);
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

        constexpr int numCycles = 2;
        const int shapeIdx = shapeBox.getSelectedItemIndex();

        auto shapeAt = [] (int idx, float phase) -> float
        {
            phase -= std::floor (phase);
            switch (idx)
            {
                case 0:  return std::sin (juce::MathConstants<float>::twoPi * phase);       // Sine
                case 1:  return 1.0f - 4.0f * std::abs (phase - 0.5f);                      // Triangle
                case 2:  return phase < 0.5f ? 1.0f : -1.0f;                                // Square
                default: return 2.0f * phase - 1.0f;                                        // Saw
            }
        };

        juce::Path path;
        constexpr int steps = 200;
        for (int i = 0; i <= steps; ++i)
        {
            const float phase = numCycles * (float) i / (float) steps;
            const float x = plot.getX() + plot.getWidth() * (float) i / (float) steps;
            const float y = plot.getCentreY() - shapeAt (shapeIdx, phase) * plot.getHeight() * 0.45f;
            if (i == 0) path.startNewSubPath (x, y); else path.lineTo (x, y);
        }
        g.setColour (ViolentColours::accent);
        g.strokePath (path, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved));

        const float rate = juce::jmax (0.02f, (float) rateSlider.getValue());
        const float sweepPeriod = numCycles / rate;
        const float sweepPhase = (float) std::fmod (juce::Time::getMillisecondCounterHiRes() * 0.001, (double) sweepPeriod) / sweepPeriod;
        const float px = plot.getX() + plot.getWidth() * sweepPhase;
        g.setColour (ViolentColours::text.withAlpha (0.5f));
        g.drawVerticalLine ((int) px, plot.getY(), plot.getBottom());
        g.setColour (ViolentColours::text);
        g.fillEllipse (px - 3.0f, plot.getCentreY() - shapeAt (shapeIdx, sweepPhase * numCycles) * plot.getHeight() * 0.45f - 3.0f, 6.0f, 6.0f);
    }

private:
    void timerCallback() override { repaint(); }

    juce::Slider& rateSlider;
    juce::ComboBox& shapeBox;
};

//==============================================================================
/** Master L+R level meter bar, polling the processor's level atomics via its
    own timer rather than needing an owner to push updates every frame. */
class LevelMeter : public juce::Component, private juce::Timer
{
public:
    explicit LevelMeter (ViolentAudioProcessor& p) : processor (p) { startTimerHz (30); }
    void paint (juce::Graphics& g) override;

private:
    ViolentAudioProcessor& processor;
    void timerCallback() override { repaint(); }
};

//==============================================================================
/** Sits on top of the whole rack, invisible until a Modulator's crosshair
    button arms processor.modTargetPick. While armed, it covers the full rack
    (so it can reach every knob regardless of scroll position) but only
    occupies each tagged modulatable knob's own bounds (see LabelledKnob::
    attachTo(), which stashes "modParamID" in the slider's properties) for
    hit-testing, painting, and the crosshair cursor - everywhere else in the
    rack is untouched "empty layout": clicks and hovers pass straight through
    to whatever's actually there, so the rest of the UI stays fully usable
    while a pick is armed. Clicking a tagged knob reports its parameter ID
    back via modTargetPick.onPick. Polls modTargetPick.active on a timer
    rather than needing every "arm" call site to reach back into this
    Component, so it can live once at the rack root. */
class ModTargetPickOverlay : public juce::Component, private juce::Timer
{
public:
    explicit ModTargetPickOverlay (ViolentAudioProcessor& p) : processor (p)
    {
        setInterceptsMouseClicks (true, false);
        startTimerHz (15);
    }

    void paint (juce::Graphics& g) override
    {
        for (auto& target : collectTargets())
        {
            g.setColour (ViolentColours::accent.withAlpha (0.14f));
            g.fillRoundedRectangle (target.bounds.toFloat(), 4.0f);
            g.setColour (ViolentColours::accent);
            g.drawRoundedRectangle (target.bounds.toFloat().reduced (0.5f), 4.0f, 1.5f);
        }
    }

    // Confines this component's interactive area to the knobs themselves -
    // everywhere else, JUCE routes the event straight through to whatever's
    // beneath, exactly as if this overlay weren't there.
    bool hitTest (int x, int y) override
    {
        for (auto& target : collectTargets())
            if (target.bounds.contains (x, y))
                return true;
        return false;
    }

    juce::MouseCursor getMouseCursor() override { return juce::MouseCursor::CrosshairCursor; }

    void mouseDown (const juce::MouseEvent& e) override
    {
        setVisible (false);
        processor.modTargetPick.active = false;

        auto onPick = std::move (processor.modTargetPick.onPick);
        processor.modTargetPick.onPick = nullptr;

        for (auto& target : collectTargets())
            if (target.bounds.contains (e.getPosition()) && onPick)
            {
                onPick (target.paramID);
                break;
            }
    }

private:
    ViolentAudioProcessor& processor;

    struct Target { juce::Rectangle<int> bounds; juce::String paramID; };

    // Walks the rack fresh every time (paint/hitTest/mouseDown), rather than
    // caching, so a knob that's moved (layout change, scroll) or been
    // removed since arming is never stale.
    std::vector<Target> collectTargets() const
    {
        std::vector<Target> out;
        auto* parent = getParentComponent();
        if (parent == nullptr) return out;

        std::function<void (juce::Component&)> visit = [&] (juce::Component& c)
        {
            if (c.getProperties().contains ("modParamID"))
                out.push_back ({ getLocalArea (&c, c.getLocalBounds()),
                                  c.getProperties()["modParamID"].toString() });
            for (int i = 0; i < c.getNumChildComponents(); ++i)
                visit (*c.getChildComponent (i));
        };
        visit (*parent);
        return out;
    }

    void timerCallback() override
    {
        const bool shouldShow = processor.modTargetPick.active;
        if (shouldShow != isVisible())
        {
            setVisible (shouldShow);
            if (shouldShow) { setBounds (getParentComponent()->getLocalBounds()); toFront (false); }
        }
        if (shouldShow) repaint(); // knob highlight rects don't move on their own, but the rack layout underneath can
    }
};

//==============================================================================
/** Small LED-style dot that lights up briefly whenever genuinely inbound
    MIDI arrives (see ViolentAudioProcessor::lastMidiActivityMs), polled via
    timer rather than needing the audio thread to push UI updates. */
class MidiActivityLight : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
{
public:
    explicit MidiActivityLight (ViolentAudioProcessor& p) : processor (p) { startTimerHz (30); }

    void paint (juce::Graphics& g) override
    {
        const auto now  = juce::Time::getMillisecondCounter();
        const auto last = processor.lastMidiActivityMs.load (std::memory_order_relaxed);
        const bool lit  = (now - last) < 150;

        auto b = getLocalBounds().toFloat().reduced (1.0f);
        g.setColour (lit ? ViolentColours::green : ViolentColours::overlay);
        g.fillEllipse (b);
        g.setColour (ViolentColours::subtext);
        g.drawEllipse (b, 1.0f);
    }

private:
    ViolentAudioProcessor& processor;
    void timerCallback() override { repaint(); }
};



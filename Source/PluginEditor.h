#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

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
class ViolentLookAndFeel : public juce::LookAndFeel_V4, private juce::Timer
{
public:
    ViolentLookAndFeel();
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
            juce::jmin (getWidth(), getHeight()) * 0.85f, juce::jmin (getWidth(), getHeight()) * 0.85f);
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
/** One FX card inside a generator's effect chain. */
class GeneratorFxCard : public juce::Component
{
public:
    static constexpr int CARD_H        = 100;
    static constexpr int FILTER_CARD_H = 150;

    GeneratorFxCard (ViolentAudioProcessor& p, int generatorIdx, int fxSlot);
    ~GeneratorFxCard() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForType (FxType t);
    int preferredHeight() const noexcept;

private:
    ViolentAudioProcessor& processor;
    int generator, slot;

    juce::TextButton removeBtn { "X" };
    juce::Label      titleLabel;

    LabelledKnob driveKnob  { "Drive",  ViolentColours::red    };
    LabelledKnob toneKnob   { "Tone",   ViolentColours::yellow };
    LabelledKnob levelKnob  { "Level",  ViolentColours::green  };
    juce::ComboBox distTypeBox;

    LabelledKnob threshKnob  { "Thresh",  ViolentColours::red    };
    LabelledKnob ratioKnob   { "Ratio",   ViolentColours::yellow };
    LabelledKnob attackKnob  { "Attack",  ViolentColours::green  };
    LabelledKnob releaseKnob { "Release", ViolentColours::blue   };
    LabelledKnob makeupKnob  { "Makeup",  ViolentColours::accent };

    LabelledKnob roomKnob    { "Room",    ViolentColours::blue   };
    LabelledKnob dampingKnob { "Damping", ViolentColours::teal   };
    LabelledKnob wetKnob     { "Wet",     ViolentColours::accent };
    LabelledKnob widthKnob   { "Width",   ViolentColours::green  };

    FilterTypeSelector filterTypeSelector;
    LabelledKnob     filterCutoffKnob { "Cutoff",    ViolentColours::blue   };
    LabelledKnob     filterResKnob    { "Resonance", ViolentColours::accent };
    FilterResponseView filterResponseView;

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CA> distTypeAtt;

    void setAllInvisible();
    void layoutKnobs (std::initializer_list<LabelledKnob*>, juce::Rectangle<int>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorFxCard)
};

//==============================================================================
/** MIDI modifier — sits before the generator: transpose, octave shift,
    quantize-to-key, and a simple held-note arpeggiator. */
class GeneratorMidiRow : public juce::Component
{
public:
    static constexpr int ROW_H = 58;

    GeneratorMidiRow (ViolentAudioProcessor& p, int generatorIdx);

    void resized() override;
    void paint (juce::Graphics&) override;

    // Existing (not being added/removed like filters and effects, since a
    // generator has at most one) — fired when the trash button is clicked.
    std::function<void()> onRemove;

private:
    ViolentAudioProcessor& processor;
    int generator;

    TrashButton      removeBtn;
    juce::Label      sectionLabel;
    LabelledKnob     transposeKnob { "Transpose", ViolentColours::teal   };
    LabelledKnob     octaveKnob    { "Octave",    ViolentColours::blue   };
    juce::TextButton keyBtn        { "Key" };
    juce::ComboBox   keyRootBox;
    juce::ComboBox   keyScaleBox;
    juce::TextButton arpBtn        { "Arp" };
    LabelledKnob     arpRateKnob  { "Arp Rate", ViolentColours::yellow };

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<CA> keyRootAtt, keyScaleAtt;
    std::unique_ptr<BA> keyEnAtt, arpEnAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorMidiRow)
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

//==============================================================================
/** One complete generator card: source + FX chain (filters are just another
    selectable effect type) + level/pan. */
class GeneratorCard : public juce::Component
{
public:
    static constexpr int SOURCE_H  = 150;  // source section height
    static constexpr int HEADER_H  = 36;
    static constexpr int FOOTER_H  = 44;
    static constexpr int ARROW_H   = 20;   // gap reserved for a routing arrow

    GeneratorCard (ViolentAudioProcessor& p, int generatorIdx);
    ~GeneratorCard() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // For the nav panel: current source-type label ("Synth"/"Sampler"), and
    // the FX-card component at a given slot (nullptr if empty).
    juce::String getSourceTypeLabel() const { return nameLabel.getText(); }
    juce::Component* getFxCard (int i) const { return fxCards[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    int generator;

    // Header
    TrashButton      removeBtn;
    juce::Label      nameLabel;
    juce::ToggleButton enableBtn;
    WaveformView     waveformView;

    // Source section
    juce::TextButton synthModeBtn   { "Synth" };
    juce::TextButton samplerModeBtn { "Sampler" };
    juce::ComboBox   srcTypeBox;
    juce::Label      srcTypeLabel;
    juce::TextButton loadSampleBtn { "Load..." };
    juce::Label      sampleFileLabel;
    int lastWaveformIndex = 0;

    LabelledKnob gainKnob    { "Gain",    ViolentColours::accent  };
    LabelledKnob octKnob     { "Oct",     ViolentColours::text    };
    LabelledKnob semiKnob    { "Semi",    ViolentColours::subtext };
    LabelledKnob detKnob     { "Detune",  ViolentColours::yellow  };
    LabelledKnob phaseKnob   { "Phase",   ViolentColours::teal    };
    LabelledKnob pwKnob      { "PW",      ViolentColours::blue    };
    LabelledKnob panKnob     { "Pan",     ViolentColours::accent  };
    LabelledKnob velKnob     { "Vel",     ViolentColours::green   };
    LabelledKnob uniKnob     { "Unison",  ViolentColours::red     };
    LabelledKnob uniSpreadKnob { "Spread",ViolentColours::yellow  };

    // ADSR box
    LabelledKnob attKnob { "Attack",  ViolentColours::green };
    LabelledKnob decKnob { "Decay",   ViolentColours::teal  };
    LabelledKnob susKnob { "Sustain", ViolentColours::blue  };
    LabelledKnob relKnob { "Release", ViolentColours::red   };

    // Footer: level + pan
    LabelledKnob levelKnob { "Level", ViolentColours::accent };
    LabelledKnob generatorPan { "Pan",   ViolentColours::yellow };

    // Sends to shared FX buses — one knob per bus slot; only the first
    // processor.numFxBuses of these are made visible/laid out.
    std::array<LabelledKnob, MAX_FX_BUSES> sendKnobs {
        LabelledKnob ("Send 1", ViolentColours::teal), LabelledKnob ("Send 2", ViolentColours::teal),
        LabelledKnob ("Send 3", ViolentColours::teal), LabelledKnob ("Send 4", ViolentColours::teal),
        LabelledKnob ("Send 5", ViolentColours::teal), LabelledKnob ("Send 6", ViolentColours::teal),
        LabelledKnob ("Send 7", ViolentColours::teal), LabelledKnob ("Send 8", ViolentColours::teal)
    };

    // FX chain
    std::array<std::unique_ptr<GeneratorFxCard>, MAX_GENERATOR_FX> fxCards;
    juce::TextButton addFxBtn { "+ Effect" };

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<BA> enableAtt;
    std::unique_ptr<CA> srcTypeAtt;

    std::unique_ptr<juce::FileChooser> fileChooser;
    void openFilePicker();

    // Y-centre (local coords) of the routing arrow drawn between the ADSR
    // section and the FX chain, computed in resized() and used by paint().
    int effectArrowY = 0;
    void drawRoutingArrow (juce::Graphics&, int y) const;

    // Bounding box of the ADSR envelope, boxed and drawn in paint().
    juce::Rectangle<int> adsrBoxBounds;

    // Construct/wire an FX card in one place, so every call site (initial
    // load, "+" button, shift-after-remove) stays consistent — in
    // particular, so removal actually gets wired up every time.
    void addFxCard (int arrayIndex, FxType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorCard)
};

//==============================================================================
/** One generator's full vertical slot in the rack: an optional MIDI modifier
    stage — added/removed just like a filter or effect, since a generator
    doesn't have to have one — plus the generator card itself, joined by a
    routing arrow whenever the modifier is present. */
class GeneratorUnit : public juce::Component
{
public:
    static constexpr int ARROW_H = 20;
    static constexpr int ADD_MIDI_BTN_H = 28;

    GeneratorUnit (ViolentAudioProcessor& p, int generatorIdx);

    void resized() override;
    void paint (juce::Graphics&) override;

    int preferredHeight() const noexcept;

    std::function<void()> onRemove;
    std::function<void()> onLayoutChanged;

    // For the nav panel; midi row is nullptr when not added.
    juce::Component* getMidiRowComponent() { return midiRow.get(); }
    GeneratorCard& getCard() { return card; }

private:
    ViolentAudioProcessor& processor;
    int generator;

    std::unique_ptr<GeneratorMidiRow> midiRow;
    juce::TextButton addMidiBtn { "+ MIDI Modifier" };
    GeneratorCard card;
    int arrowY = 0;

    void setMidiRowPresent (bool present);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorUnit)
};

//==============================================================================
/** Main panel: vertical list of GeneratorUnits. */
class GeneratorPanel : public juce::Component
{
public:
    explicit GeneratorPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every card from scratch, e.g. after a preset load
    // changes generator counts/modes/filters/fx behind the UI's back.
    void refreshFromState();

    // For the nav panel.
    int getNumUnits() const noexcept { return processor.numActiveGenerators; }
    GeneratorUnit* getUnit (int i) const { return units[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    std::array<std::unique_ptr<GeneratorUnit>, MAX_GENERATORS> units;
    juce::TextButton addBtn { "+ Add Generator" };

    void rebuild (bool forceRecreate = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorPanel)
};

//==============================================================================
/** One master filter: type/cutoff/resonance plus which generators route into it.
    Master filters are applied last, after every generator's own filters/FX. */
class MasterFilterRow : public juce::Component
{
public:
    static constexpr int ROW_H = 136;

    MasterFilterRow (ViolentAudioProcessor& p, int filterSlot);

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;

private:
    ViolentAudioProcessor& processor;
    int slot;

    FilterControlsBlock block;

    juce::Label routingLabel;
    std::array<juce::TextButton, MAX_GENERATORS> routingBtns;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterFilterRow)
};

//==============================================================================
/** Master filter chain — added last, after all generators. */
class MasterFilterPanel : public juce::Component
{
public:
    explicit MasterFilterPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every row from scratch, e.g. after a preset load.
    void refreshFromState();

    // For the nav panel.
    int getNumRows() const noexcept { return processor.numMasterFilters; }
    MasterFilterRow* getRow (int i) const { return rows[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<MasterFilterRow>, MAX_MASTER_FILTERS> rows;
    juce::TextButton addBtn { "+ Add Filter" };

    void rebuild (bool forceRecreate = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterFilterPanel)
};

//==============================================================================
/** One shared FX bus: a single chosen effect that any generator can send a
    portion of its (post-FX) signal into via a per-generator send knob (see
    GeneratorCard's Sends row). An alternative routing path alongside each
    generator's own private FX chain, not a replacement for it. */
class FxBusRow : public juce::Component
{
public:
    static constexpr int ROW_H        = 100;
    static constexpr int FILTER_ROW_H = 150;

    FxBusRow (ViolentAudioProcessor& p, int busSlot);
    ~FxBusRow() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForType (FxType t);
    int preferredHeight() const noexcept;

private:
    ViolentAudioProcessor& processor;
    int bus;

    TrashButton      removeBtn;
    juce::Label      titleLabel;

    LabelledKnob driveKnob  { "Drive",  ViolentColours::red    };
    LabelledKnob toneKnob   { "Tone",   ViolentColours::yellow };
    LabelledKnob levelKnob  { "Level",  ViolentColours::green  };
    juce::ComboBox distTypeBox;

    LabelledKnob threshKnob  { "Thresh",  ViolentColours::red    };
    LabelledKnob ratioKnob   { "Ratio",   ViolentColours::yellow };
    LabelledKnob attackKnob  { "Attack",  ViolentColours::green  };
    LabelledKnob releaseKnob { "Release", ViolentColours::blue   };
    LabelledKnob makeupKnob  { "Makeup",  ViolentColours::accent };

    LabelledKnob roomKnob    { "Room",    ViolentColours::blue   };
    LabelledKnob dampingKnob { "Damping", ViolentColours::teal   };
    LabelledKnob wetKnob     { "Wet",     ViolentColours::accent };
    LabelledKnob widthKnob   { "Width",   ViolentColours::green  };

    FilterTypeSelector filterTypeSelector;
    LabelledKnob     filterCutoffKnob { "Cutoff",    ViolentColours::blue   };
    LabelledKnob     filterResKnob    { "Resonance", ViolentColours::accent };
    FilterResponseView filterResponseView;

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CA> distTypeAtt;

    void setAllInvisible();
    void layoutKnobs (std::initializer_list<LabelledKnob*>, juce::Rectangle<int>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxBusRow)
};

//==============================================================================
/** Shared FX bus rack — generators send into these; each bus applies a
    single chosen effect and mixes its result additively into the master. */
class FxBusPanel : public juce::Component
{
public:
    explicit FxBusPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every row from scratch, e.g. after a preset load.
    void refreshFromState();

    // For the nav panel.
    int getNumBuses() const noexcept { return processor.numFxBuses; }
    FxBusRow* getRow (int i) const { return rows[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<FxBusRow>, MAX_FX_BUSES> rows;
    juce::TextButton addBtn { "+ Add Bus" };

    void rebuild (bool forceRecreate = false);
    void addRow (int arrayIndex, FxType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxBusPanel)
};

//==============================================================================
/** One channel strip in the mixer: name, live level meter, and a fader
    bound to that generator's Level parameter. */
class MixerChannel : public juce::Component, private juce::Timer
{
public:
    MixerChannel (ViolentAudioProcessor& p, int generatorIdx);

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }

    ViolentAudioProcessor& processor;
    int generator;

    juce::Label  nameLabel;
    juce::Slider levelSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerChannel)
};

//==============================================================================
/** Mixer strip along the bottom — one channel per active generator. */
class MixerPanel : public juce::Component
{
public:
    static constexpr int PANEL_H = 132;

    explicit MixerPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override;

    void rebuild();

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<MixerChannel>, MAX_GENERATORS> channels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerPanel)
};

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

//==============================================================================
class ViolentAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit ViolentAudioProcessorEditor (ViolentAudioProcessor&);
    ~ViolentAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    static constexpr int HEADER_H = 52;
    // Content taller than this scrolls instead of growing the window further.
    static constexpr int MAX_WINDOW_H = 800;

    ViolentAudioProcessor& processor;
    ViolentLookAndFeel     laf;

    ScalableRackComponent rack;
    RackScaler rackScaler { rack };
    juce::Viewport rackViewport;
    NavPanel navPanel;
    juce::Viewport navViewport;

    LevelMeter  meter;
    juce::TextButton previewBtn;
    juce::ComboBox   previewPatternBox;

    juce::ComboBox   presetBox;
    juce::TextButton savePresetBtn { "Save" };

    juce::TextButton zoomOutBtn { "-" };
    juce::TextButton zoomInBtn  { "+" };
    juce::Label      zoomLabel;

    float uiScale = 1.0f;
    void setUiScale (float newScale);

    void timerCallback() override;

    void refreshPresetList();
    void loadSelectedPreset();
    void promptAndSavePreset();

    void updateHeight();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViolentAudioProcessorEditor)
};

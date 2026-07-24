#include "SharedComponents.h"

//==============================================================================
// ViolentUI
//==============================================================================
ViolentUI::ViolentUI()
{
    setColour (juce::ResizableWindow::backgroundColourId,       ViolentColours::background);
    setColour (juce::Label::textColourId,                       ViolentColours::text);
    setColour (juce::Slider::thumbColourId,                     ViolentColours::accent);
    setColour (juce::Slider::trackColourId,                     ViolentColours::surface);
    setColour (juce::Slider::backgroundColourId,                ViolentColours::surface);
    setColour (juce::ToggleButton::textColourId,                ViolentColours::text);
    setColour (juce::TextButton::buttonColourId,                ViolentColours::surface);
    setColour (juce::TextButton::buttonOnColourId,              ViolentColours::accent);
    setColour (juce::TextButton::textColourOffId,               ViolentColours::subtext);
    setColour (juce::TextButton::textColourOnId,                ViolentColours::background);
    setColour (juce::ComboBox::backgroundColourId,              ViolentColours::surface);
    setColour (juce::ComboBox::textColourId,                    ViolentColours::text);
    setColour (juce::ComboBox::outlineColourId,                 ViolentColours::overlay);
    setColour (juce::PopupMenu::backgroundColourId,             ViolentColours::surface);
    setColour (juce::PopupMenu::textColourId,                   ViolentColours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId,  ViolentColours::accent);
    setColour (juce::PopupMenu::highlightedTextColourId,        ViolentColours::background);
}

void ViolentUI::drawRotarySlider (juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    // Margin leaves just enough room around the knob body for the hover glow
    // below to render without being clipped by the slider's own bounds.
    const float radius  = static_cast<float> (juce::jmin (width, height)) * 0.5f - 3.0f;
    const float centreX = static_cast<float> (x) + static_cast<float> (width)  * 0.5f;
    const float centreY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const float rx = centreX - radius, ry = centreY - radius, rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // The knob's own assigned colour (set via LabelledKnob's constructor or
    // setAccentColour()) is what it reads "hot" as — falls back to the
    // global accent for any slider that never had one assigned.
    juce::Colour hot = slider.findColour (juce::Slider::rotarySliderFillColourId, false);
    if (! hot.isOpaque()) hot = ViolentColours::accent;

    const bool  wantsGlow = slider.isEnabled() && (slider.isMouseOver (true) || slider.isMouseButtonDown());
    const float glow = animatedHoverAmount (slider, wantsGlow);
    if (glow > 0.001f)
    {
        // Subtle glow: a few soft, low-alpha rings just outside the knob body,
        // drawn before the body itself so only the outer ring shows through.
        for (int i = 3; i >= 1; --i)
        {
            const float glowR = radius + (float) i;
            g.setColour (hot.withAlpha (0.045f * (float) (4 - i) * glow));
            g.fillEllipse (centreX - glowR, centreY - glowR, glowR * 2.0f, glowR * 2.0f);
        }
    }

    g.setColour (ViolentColours::overlay);
    g.fillEllipse (rx, ry, rw, rw);

    juce::Path trackArc;
    trackArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                             0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (ViolentColours::surface);
    g.strokePath (trackArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    // Knobs read hot (their assigned colour) the further they're turned up,
    // and cool (muted grey) near the bottom of their range.
    const juce::Colour arc = ViolentColours::subtext.interpolatedWith (hot, sliderPos);

    juce::Path valArc;
    valArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                          0.0f, rotaryStartAngle, angle, true);
    g.setColour (arc);
    g.strokePath (valArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

void ViolentUI::drawLinearSlider (juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, float, float,
    juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearVertical) return;
    const float trackW  = 6.0f;
    const float centreX = (float) x + (float) width * 0.5f;
    const float top     = (float) y + 8.0f;
    const float bot     = (float) (y + height) - 8.0f;
    const float mid     = top + (bot - top) * 0.5f;

    g.setColour (ViolentColours::overlay);
    g.fillRoundedRectangle (centreX - trackW * 0.5f, top, trackW, bot - top, 3.0f);
    g.setColour (ViolentColours::subtext);
    g.drawHorizontalLine ((int) mid, centreX - 8.0f, centreX + 8.0f);

    juce::Colour fill = slider.findColour (juce::Slider::rotarySliderFillColourId, false);
    if (!fill.isOpaque()) fill = ViolentColours::accent;
    if (sliderPos < mid)
        { g.setColour (fill); g.fillRoundedRectangle (centreX - trackW * 0.5f, sliderPos, trackW, mid - sliderPos, 3.0f); }
    else
        { g.setColour (fill.withAlpha (0.6f)); g.fillRoundedRectangle (centreX - trackW * 0.5f, mid, trackW, sliderPos - mid, 3.0f); }

    g.setColour (ViolentColours::text);
    g.fillRoundedRectangle (centreX - 11.0f, sliderPos - 6.0f, 22.0f, 12.0f, 3.0f);
}

void ViolentUI::paintControlShape (juce::Graphics& g, juce::Component& c, juce::Rectangle<float> b,
                                             bool isOn, bool isHovered)
{
    const float hover = animatedHoverAmount (c, isHovered);

    // A component's own buttonOnColourId (e.g. set by GeneratorCard to its
    // per-generator accent) wins over the global accent if it's been set.
    juce::Colour on = c.findColour (juce::TextButton::buttonOnColourId, false);
    if (! on.isOpaque()) on = ViolentColours::accent;

    g.setColour (isOn ? on
                       : ViolentColours::surface.interpolatedWith (ViolentColours::overlay, hover));
    g.fillRoundedRectangle (b, ViolentColours::cornerRadius);

    juce::Colour border = isOn ? on.brighter (0.2f) : ViolentColours::overlay;
    border = border.brighter (0.35f * hover);
    g.setColour (border);
    g.drawRoundedRectangle (b, ViolentColours::cornerRadius, 1.0f);
}

void ViolentUI::drawToggleButton (juce::Graphics& g, juce::ToggleButton& btn,
                                            bool highlighted, bool)
{
    const bool on = btn.getToggleState();
    const auto b  = btn.getLocalBounds().toFloat().reduced (1.0f);
    paintControlShape (g, btn, b, on, highlighted);
    g.setColour (on ? ViolentColours::background : ViolentColours::text);
    g.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    g.drawFittedText (btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred, 1);
}

void ViolentUI::drawButtonBackground (juce::Graphics& g, juce::Button& btn,
                                                const juce::Colour&, bool highlighted, bool)
{
    const auto b  = btn.getLocalBounds().toFloat().reduced (1.0f);
    paintControlShape (g, btn, b, btn.getToggleState(), highlighted);
}

void ViolentUI::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                        int, int, int, int, juce::ComboBox& box)
{
    const auto b = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);
    // ComboBox has an internal Label covering most of its area (the text
    // display), so a plain isMouseOver() — which excludes children by
    // default — only ever sees the small uncovered strip near the arrow.
    // Include children so hover covers the whole control. Disabled boxes
    // shouldn't show a hover state at all, same as a disabled text field.
    const bool hovered = box.isEnabled() && box.isMouseOver (true);
    paintControlShape (g, box, b, box.isPopupActive(), hovered);

    const float arrowCX = (float) width - 15.0f, arrowCY = (float) height * 0.5f;
    juce::Path arrow;
    arrow.addTriangle (arrowCX - 4.0f, arrowCY - 2.5f, arrowCX + 4.0f, arrowCY - 2.5f, arrowCX, arrowCY + 3.0f);
    g.setColour (ViolentColours::subtext);
    g.fillPath (arrow);
}

juce::Font ViolentUI::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

juce::Font ViolentUI::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (juce::jmin (12.0f, (float) buttonHeight * 0.6f)).withStyle ("Bold"));
}

juce::Font ViolentUI::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

juce::Font ViolentUI::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

float ViolentUI::animatedHoverAmount (juce::Component& c, bool isHovered)
{
    constexpr double durationMs = 100.0;
    const double now = juce::Time::getMillisecondCounterHiRes();
    const float targetNow = isHovered ? 1.0f : 0.0f;

    auto& props = c.getProperties();
    const bool hasState = props.contains ("vHoverStart");

    double start  = hasState ? (double) props["vHoverStart"]  : now;
    float  from   = hasState ? (float) (double) props["vHoverFrom"]   : targetNow;
    float  target = hasState ? (float) (double) props["vHoverTarget"] : targetNow;

    const double elapsed = juce::jlimit (0.0, durationMs, now - start);
    const float  t = (float) (elapsed / durationMs);
    float progress = from + (target - from) * t;

    if (! hasState || ! juce::approximatelyEqual (target, targetNow))
    {
        // First time seeing this component, or the hover target just
        // flipped — (re)start the transition from wherever we are now, so
        // reversing direction mid-fade stays smooth instead of jumping.
        start = now;
        from = progress;
        target = targetNow;
        props.set ("vHoverStart", start);
        props.set ("vHoverFrom", (double) from);
        props.set ("vHoverTarget", (double) target);
    }

    if (! juce::approximatelyEqual (progress, target))
        registerForHoverAnimation (c);

    return progress;
}

void ViolentUI::registerForHoverAnimation (juce::Component& c)
{
    for (auto& sp : animatingComponents)
        if (sp.getComponent() == &c)
            return;

    animatingComponents.add (juce::Component::SafePointer<juce::Component> (&c));
    if (! isTimerRunning())
        startTimerHz (60);
}

void ViolentUI::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();

    for (int i = animatingComponents.size(); --i >= 0;)
    {
        auto* comp = animatingComponents.getReference (i).getComponent();
        if (comp == nullptr)
        {
            animatingComponents.remove (i);
            continue;
        }

        comp->repaint();

        auto& props = comp->getProperties();
        const double start = (double) props.getWithDefault ("vHoverStart", 0.0);
        if (now - start >= 100.0)
            animatingComponents.remove (i);
    }

    if (animatingComponents.isEmpty())
        stopTimer();
}

//==============================================================================
// ColourSwatchButton
//==============================================================================
namespace
{
    class ColourPickerContent : public juce::Component, private juce::ChangeListener
    {
    public:
        ColourPickerContent (juce::Colour initial, std::function<void (juce::Colour)> cb)
            : onChange (std::move (cb))
        {
            selector.setCurrentColour (initial, juce::dontSendNotification);
            selector.addChangeListener (this);
            addAndMakeVisible (selector);
            setSize (240, 300);
        }

        void resized() override { selector.setBounds (getLocalBounds()); }

    private:
        void changeListenerCallback (juce::ChangeBroadcaster*) override
        {
            if (onChange) onChange (selector.getCurrentColour());
        }

        juce::ColourSelector selector { juce::ColourSelector::showColourAtTop
                                       | juce::ColourSelector::showSliders
                                       | juce::ColourSelector::showColourspace };
        std::function<void (juce::Colour)> onChange;
    };
}

void ColourSwatchButton::mouseDown (const juce::MouseEvent&)
{
    auto content = std::make_unique<ColourPickerContent> (swatch, [this] (juce::Colour c)
    {
        setColour (c);
        if (onColourChanged) onColourChanged (c);
    });
    juce::CallOutBox::launchAsynchronously (std::move (content), getScreenBounds(), nullptr);
}

//==============================================================================
// LabelledKnob
//==============================================================================
LabelledKnob::LabelledKnob (const juce::String& name, juce::Colour colour)
    : knobColour (colour)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
    slider.setRepaintsOnMouseActivity (true);
    slider.setScrollWheelEnabled (false);
    addAndMakeVisible (slider);

    nameLabel.setText (name, juce::dontSendNotification);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (nameLabel);

    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (valueLabel);

    slider.onValueChange = [this]
    {
        valueLabel.setText (slider.getTextFromValue (slider.getValue()),
                            juce::dontSendNotification);
    };
}

void LabelledKnob::resized()
{
    auto a = getLocalBounds();
    nameLabel .setBounds (a.removeFromTop    (14));
    valueLabel.setBounds (a.removeFromBottom (13));
    slider    .setBounds (a);
}

void LabelledKnob::attachTo (juce::AudioProcessorValueTreeState& apvts, const juce::String& parameterID)
{
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, parameterID, slider);
    valueLabel.setText (slider.getTextFromValue (slider.getValue()), juce::dontSendNotification);
}

//==============================================================================
// LevelMeter
//==============================================================================
void LevelMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float w = b.getWidth() * 0.45f, gap = b.getWidth() - 2.0f * w;

    auto drawBar = [&] (float x, float level)
    {
        const float db   = juce::Decibels::gainToDecibels (juce::jmax (0.0001f, level));
        const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        g.setColour (ViolentColours::surface);
        g.fillRect (x, b.getY(), w, b.getHeight());
        g.setColour (norm > 0.85f ? ViolentColours::red
                   : norm > 0.6f  ? ViolentColours::yellow : ViolentColours::green);
        g.fillRect (x, b.getBottom() - b.getHeight() * norm, w, b.getHeight() * norm);
    };
    drawBar (b.getX(), peakL);
    drawBar (b.getX() + w + gap, peakR);
}


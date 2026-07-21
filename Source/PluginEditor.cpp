#include "PluginEditor.h"

//==============================================================================
// ViolentLookAndFeel
//==============================================================================
ViolentLookAndFeel::ViolentLookAndFeel()
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

void ViolentLookAndFeel::drawRotarySlider (juce::Graphics& g,
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

    const bool  wantsGlow = slider.isEnabled() && (slider.isMouseOver (true) || slider.isMouseButtonDown());
    const float glow = animatedHoverAmount (slider, wantsGlow);
    if (glow > 0.001f)
    {
        // Subtle glow: a few soft, low-alpha rings just outside the knob body,
        // drawn before the body itself so only the outer ring shows through.
        for (int i = 3; i >= 1; --i)
        {
            const float glowR = radius + (float) i;
            g.setColour (ViolentColours::accent.withAlpha (0.045f * (float) (4 - i) * glow));
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

    // Knobs read hot (accent orange) the further they're turned up, and cool
    // (muted grey) near the bottom of their range, regardless of parameter.
    const juce::Colour arc = ViolentColours::subtext.interpolatedWith (ViolentColours::accent, sliderPos);

    juce::Path valArc;
    valArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                          0.0f, rotaryStartAngle, angle, true);
    g.setColour (arc);
    g.strokePath (valArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

void ViolentLookAndFeel::drawLinearSlider (juce::Graphics& g,
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

void ViolentLookAndFeel::paintControlShape (juce::Graphics& g, juce::Component& c, juce::Rectangle<float> b,
                                             bool isOn, bool isHovered)
{
    const float hover = animatedHoverAmount (c, isHovered);

    g.setColour (isOn ? ViolentColours::accent
                       : ViolentColours::surface.interpolatedWith (ViolentColours::overlay, hover));
    g.fillRoundedRectangle (b, ViolentColours::cornerRadius);

    juce::Colour border = isOn ? ViolentColours::accent.brighter (0.2f) : ViolentColours::overlay;
    border = border.brighter (0.35f * hover);
    g.setColour (border);
    g.drawRoundedRectangle (b, ViolentColours::cornerRadius, 1.0f);
}

void ViolentLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& btn,
                                            bool highlighted, bool)
{
    const bool on = btn.getToggleState();
    const auto b  = btn.getLocalBounds().toFloat().reduced (1.0f);
    paintControlShape (g, btn, b, on, highlighted);
    g.setColour (on ? ViolentColours::background : ViolentColours::text);
    g.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    g.drawFittedText (btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred, 1);
}

void ViolentLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& btn,
                                                const juce::Colour&, bool highlighted, bool)
{
    const auto b  = btn.getLocalBounds().toFloat().reduced (1.0f);
    paintControlShape (g, btn, b, btn.getToggleState(), highlighted);
}

void ViolentLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
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

juce::Font ViolentLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

juce::Font ViolentLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (juce::jmin (12.0f, (float) buttonHeight * 0.6f)).withStyle ("Bold"));
}

juce::Font ViolentLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

juce::Font ViolentLookAndFeel::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

float ViolentLookAndFeel::animatedHoverAmount (juce::Component& c, bool isHovered)
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

void ViolentLookAndFeel::registerForHoverAnimation (juce::Component& c)
{
    for (auto& sp : animatingComponents)
        if (sp.getComponent() == &c)
            return;

    animatingComponents.add (juce::Component::SafePointer<juce::Component> (&c));
    if (! isTimerRunning())
        startTimerHz (60);
}

void ViolentLookAndFeel::timerCallback()
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
// LabelledKnob
//==============================================================================
LabelledKnob::LabelledKnob (const juce::String& name, juce::Colour colour)
    : knobColour (colour)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
    slider.setRepaintsOnMouseActivity (true);
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

//==============================================================================
// GeneratorFxCard
//==============================================================================
GeneratorFxCard::GeneratorFxCard (ViolentAudioProcessor& p, int generatorIdx, int fxSlot)
    : processor (p), generator (generatorIdx), slot (fxSlot),
      filterTypeSelector (p.apvts, ParamIDs::genFxFilterType (generatorIdx, fxSlot)),
      filterResponseView (p.apvts, ParamIDs::genFxFilterType (generatorIdx, fxSlot),
                           ParamIDs::genFxFilterCut  (generatorIdx, fxSlot),
                           ParamIDs::genFxFilterRes  (generatorIdx, fxSlot))
{
    titleLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    titleLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (titleLabel);

    removeBtn.setColour (juce::TextButton::textColourOffId, ViolentColours::red);
    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    for (const auto& t : { "Soft Clip", "Hard Clip", "Fuzz" })
        distTypeBox.addItem (t, distTypeBox.getNumItems() + 1);
    distTypeBox.setRepaintsOnMouseActivity (true);
    addChildComponent (distTypeBox);

    for (auto* k : { &driveKnob, &toneKnob, &levelKnob,
                     &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob,
                     &roomKnob, &dampingKnob, &wetKnob, &widthKnob,
                     &filterCutoffKnob, &filterResKnob })
        addChildComponent (*k);
    addChildComponent (filterTypeSelector);
    addChildComponent (filterResponseView);

    driveKnob    .attachTo (processor.apvts, ParamIDs::genFxDrive    (generator, slot));
    toneKnob     .attachTo (processor.apvts, ParamIDs::genFxTone     (generator, slot));
    levelKnob    .attachTo (processor.apvts, ParamIDs::genFxLevel    (generator, slot));
    distTypeAtt  = std::make_unique<CA> (processor.apvts, ParamIDs::genFxDistType (generator, slot), distTypeBox);
    threshKnob   .attachTo (processor.apvts, ParamIDs::genFxThresh   (generator, slot));
    ratioKnob    .attachTo (processor.apvts, ParamIDs::genFxRatio    (generator, slot));
    attackKnob   .attachTo (processor.apvts, ParamIDs::genFxAttack   (generator, slot));
    releaseKnob  .attachTo (processor.apvts, ParamIDs::genFxRelease  (generator, slot));
    makeupKnob   .attachTo (processor.apvts, ParamIDs::genFxMakeup   (generator, slot));
    roomKnob     .attachTo (processor.apvts, ParamIDs::genFxRoom     (generator, slot));
    dampingKnob  .attachTo (processor.apvts, ParamIDs::genFxDamping  (generator, slot));
    wetKnob      .attachTo (processor.apvts, ParamIDs::genFxWet      (generator, slot));
    widthKnob    .attachTo (processor.apvts, ParamIDs::genFxWidth    (generator, slot));
    filterCutoffKnob.attachTo (processor.apvts, ParamIDs::genFxFilterCut (generator, slot));
    filterResKnob   .attachTo (processor.apvts, ParamIDs::genFxFilterRes (generator, slot));

    showForType (processor.generators[(size_t) generator].fxTypes[(size_t) slot]);
}

GeneratorFxCard::~GeneratorFxCard() {}

void GeneratorFxCard::setAllInvisible()
{
    for (auto* k : { &driveKnob, &toneKnob, &levelKnob,
                     &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob,
                     &roomKnob, &dampingKnob, &wetKnob, &widthKnob,
                     &filterCutoffKnob, &filterResKnob })
        k->setVisible (false);
    distTypeBox.setVisible (false);
    filterTypeSelector.setVisible (false);
    filterResponseView.setVisible (false);
}

void GeneratorFxCard::showForType (FxType t)
{
    setAllInvisible();
    titleLabel.setText (fxTypeName (t), juce::dontSendNotification);
    switch (t)
    {
        case FxType::Distortion:
            driveKnob.setVisible(true); toneKnob.setVisible(true);
            levelKnob.setVisible(true); distTypeBox.setVisible(true); break;
        case FxType::Compressor:
            threshKnob.setVisible(true); ratioKnob.setVisible(true);
            attackKnob.setVisible(true); releaseKnob.setVisible(true);
            makeupKnob.setVisible(true); break;
        case FxType::Gate:
            threshKnob.setVisible(true); ratioKnob.setVisible(true);
            attackKnob.setVisible(true); releaseKnob.setVisible(true); break;
        case FxType::Reverb:
            roomKnob.setVisible(true); dampingKnob.setVisible(true);
            wetKnob.setVisible(true);  widthKnob.setVisible(true); break;
        case FxType::Filter:
            filterTypeSelector.setVisible(true);
            filterCutoffKnob.setVisible(true); filterResKnob.setVisible(true);
            filterResponseView.setVisible(true); break;
        default: break;
    }
}

int GeneratorFxCard::preferredHeight() const noexcept
{
    return processor.generators[(size_t) generator].fxTypes[(size_t) slot] == FxType::Filter
        ? FILTER_CARD_H : CARD_H;
}

void GeneratorFxCard::layoutKnobs (std::initializer_list<LabelledKnob*> ks, juce::Rectangle<int> a)
{
    const int w = a.getWidth() / (int) ks.size();
    for (auto* k : ks) k->setBounds (a.removeFromLeft (w).reduced (3, 2));
}

void GeneratorFxCard::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f, 1.0f);
}

void GeneratorFxCard::resized()
{
    auto a = getLocalBounds().reduced (4, 3);
    auto hdr = a.removeFromTop (22);
    removeBtn .setBounds (hdr.removeFromLeft (24));
    titleLabel.setBounds (hdr);
    a.removeFromTop (2);

    const FxType t = processor.generators[(size_t) generator].fxTypes[(size_t) slot];
    switch (t)
    {
        case FxType::Distortion:
            distTypeBox.setBounds (a.removeFromRight (80).reduced (2, 4));
            layoutKnobs ({ &driveKnob, &toneKnob, &levelKnob }, a); break;
        case FxType::Compressor:
            layoutKnobs ({ &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob }, a); break;
        case FxType::Gate:
            layoutKnobs ({ &threshKnob, &ratioKnob, &attackKnob, &releaseKnob }, a); break;
        case FxType::Reverb:
            layoutKnobs ({ &roomKnob, &dampingKnob, &wetKnob, &widthKnob }, a); break;
        case FxType::Filter:
        {
            filterTypeSelector.setBounds (a.removeFromTop (24).reduced (2, 2));
            a.removeFromTop (2);
            layoutKnobs ({ &filterCutoffKnob, &filterResKnob }, a.removeFromTop (54));
            a.removeFromTop (2);
            filterResponseView.setBounds (a);
            break;
        }
        default: break;
    }
}

//==============================================================================
// GeneratorMidiRow
//==============================================================================
GeneratorMidiRow::GeneratorMidiRow (ViolentAudioProcessor& p, int generatorIdx)
    : processor (p), generator (generatorIdx)
{
    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    sectionLabel.setText ("MIDI", juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    for (auto* k : { &transposeKnob, &octaveKnob, &arpRateKnob }) addAndMakeVisible (*k);

    keyBtn.setClickingTogglesState (true);
    addAndMakeVisible (keyBtn);
    arpBtn.setClickingTogglesState (true);
    addAndMakeVisible (arpBtn);

    for (const auto& t : { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" })
        keyRootBox.addItem (t, keyRootBox.getNumItems() + 1);
    keyRootBox.setRepaintsOnMouseActivity (true);
    addAndMakeVisible (keyRootBox);

    for (const auto& t : { "Major", "Minor" })
        keyScaleBox.addItem (t, keyScaleBox.getNumItems() + 1);
    keyScaleBox.setRepaintsOnMouseActivity (true);
    addAndMakeVisible (keyScaleBox);

    transposeKnob.attachTo (processor.apvts, ParamIDs::genMidiTranspose (generator));
    octaveKnob   .attachTo (processor.apvts, ParamIDs::genMidiOctave    (generator));
    arpRateKnob  .attachTo (processor.apvts, ParamIDs::genMidiArpRate   (generator));
    keyRootAtt   = std::make_unique<CA> (processor.apvts, ParamIDs::genMidiKeyRoot   (generator), keyRootBox);
    keyScaleAtt  = std::make_unique<CA> (processor.apvts, ParamIDs::genMidiKeyScale  (generator), keyScaleBox);
    keyEnAtt     = std::make_unique<BA> (processor.apvts, ParamIDs::genMidiKeyEnabled (generator), keyBtn);
    arpEnAtt     = std::make_unique<BA> (processor.apvts, ParamIDs::genMidiArpEnabled (generator), arpBtn);
}

void GeneratorMidiRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
}

void GeneratorMidiRow::resized()
{
    auto a = getLocalBounds().reduced (6, 3);
    removeBtn.setBounds (a.removeFromLeft (28).withSizeKeepingCentre (20, 20));
    sectionLabel.setBounds (a.removeFromLeft (40));

    a.removeFromLeft (4);
    transposeKnob.setBounds (a.removeFromLeft (72).reduced (2, 1));
    octaveKnob   .setBounds (a.removeFromLeft (72).reduced (2, 1));

    a.removeFromLeft (6);
    keyBtn     .setBounds (a.removeFromLeft (44).reduced (2, 14));
    keyRootBox .setBounds (a.removeFromLeft (64).reduced (2, 14));
    keyScaleBox.setBounds (a.removeFromLeft (74).reduced (2, 14));

    a.removeFromLeft (6);
    arpBtn      .setBounds (a.removeFromLeft (44).reduced (2, 14));
    arpRateKnob .setBounds (a.removeFromLeft (72).reduced (2, 1));
}

//==============================================================================
// FilterControlsBlock
//==============================================================================
FilterControlsBlock::FilterControlsBlock (juce::AudioProcessorValueTreeState& apvts, const juce::String& name,
                                           const juce::String& typeParamID,
                                           const juce::String& cutoffParamID,
                                           const juce::String& resParamID)
    : typeSelector (apvts, typeParamID),
      responseView (apvts, typeParamID, cutoffParamID, resParamID)
{
    nameLabel.setText (name, juce::dontSendNotification);
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (nameLabel);

    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    addAndMakeVisible (typeSelector);

    for (auto* k : { &cutoffKnob, &resKnob }) addAndMakeVisible (*k);
    addAndMakeVisible (responseView);

    cutoffKnob.attachTo (apvts, cutoffParamID);
    resKnob   .attachTo (apvts, resParamID);
}

void FilterControlsBlock::resized()
{
    auto a = getLocalBounds().reduced (4, 3);
    auto top = a.removeFromTop (40);
    removeBtn   .setBounds (top.removeFromLeft  (28).withSizeKeepingCentre (20, 20));
    nameLabel   .setBounds (top.removeFromLeft  (60));
    typeSelector.setBounds (top.removeFromLeft  (160).reduced (2, 4));
    const int kw = top.getWidth() / 2;
    cutoffKnob.setBounds (top.removeFromLeft (kw).reduced (3, 2));
    resKnob   .setBounds (top.reduced (3, 2));

    a.removeFromTop (2);
    responseView.setBounds (a);
}

//==============================================================================
// Shared routing-arrow drawing — used between every stage of the signal
// chain (MIDI modifier -> generator -> filters -> effects), whether the two
// stages are inside the same component or, like the MIDI modifier and its
// generator, separate sibling components.
//==============================================================================
static void drawGeneratorRoutingArrow (juce::Graphics& g, int width, int y)
{
    const float cx = (float) width * 0.5f;
    const float halfLineLen = 60.0f;

    g.setColour (ViolentColours::accent.withAlpha (0.35f));
    g.drawLine (cx - halfLineLen, (float) y, cx + halfLineLen, (float) y, 1.0f);

    juce::Path arrow;
    arrow.addTriangle (cx - 5.0f, (float) y - 4.0f, cx + 5.0f, (float) y - 4.0f, cx, (float) y + 4.0f);
    g.setColour (ViolentColours::accent);
    g.fillPath (arrow);
}

//==============================================================================
// GeneratorCard
//==============================================================================
GeneratorCard::GeneratorCard (ViolentAudioProcessor& p, int generatorIdx)
    : processor (p), generator (generatorIdx), waveformView (p, generatorIdx)
{
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (nameLabel);
    addAndMakeVisible (waveformView);

    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    enableBtn.setButtonText ("ON");
    enableBtn.setClickingTogglesState (true);
    addAndMakeVisible (enableBtn);
    enableAtt = std::make_unique<BA> (processor.apvts, ParamIDs::generatorEn (generator), enableBtn);

    // Source type
    for (const auto& t : { "Sine", "Saw", "Square", "Triangle", "Noise", "Sample" })
        srcTypeBox.addItem (t, srcTypeBox.getNumItems() + 1);
    srcTypeBox.setRepaintsOnMouseActivity (true);
    addAndMakeVisible (srcTypeBox);
    srcTypeAtt = std::make_unique<CA> (processor.apvts, ParamIDs::genSrcType (generator), srcTypeBox);

    addAndMakeVisible (synthModeBtn);
    addAndMakeVisible (samplerModeBtn);
    synthModeBtn.onClick = [this]
    {
        if (srcTypeBox.getSelectedItemIndex() == (int) SourceType::Sample)
            srcTypeBox.setSelectedItemIndex (lastWaveformIndex);
    };
    samplerModeBtn.onClick = [this]
    {
        if (srcTypeBox.getSelectedItemIndex() != (int) SourceType::Sample)
            lastWaveformIndex = srcTypeBox.getSelectedItemIndex();
        srcTypeBox.setSelectedItemIndex ((int) SourceType::Sample);
    };

    loadSampleBtn.onClick = [this] { openFilePicker(); };
    addChildComponent (loadSampleBtn);
    sampleFileLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addChildComponent (sampleFileLabel);

    // Source knobs
    for (auto* k : { &gainKnob, &octKnob, &semiKnob, &detKnob, &phaseKnob,
                     &pwKnob, &panKnob, &velKnob, &uniKnob, &uniSpreadKnob,
                     &attKnob, &decKnob, &susKnob, &relKnob,
                     &levelKnob, &generatorPan })
        addAndMakeVisible (*k);

    gainKnob      .attachTo (processor.apvts, ParamIDs::genSrcGain      (generator));
    octKnob       .attachTo (processor.apvts, ParamIDs::genSrcOct       (generator));
    semiKnob      .attachTo (processor.apvts, ParamIDs::genSrcSemi      (generator));
    detKnob       .attachTo (processor.apvts, ParamIDs::genSrcDet       (generator));
    phaseKnob     .attachTo (processor.apvts, ParamIDs::genSrcPhase     (generator));
    pwKnob        .attachTo (processor.apvts, ParamIDs::genSrcPW        (generator));
    panKnob       .attachTo (processor.apvts, ParamIDs::genSrcPan       (generator));
    velKnob       .attachTo (processor.apvts, ParamIDs::genSrcVel       (generator));
    uniKnob       .attachTo (processor.apvts, ParamIDs::genSrcUni       (generator));
    uniSpreadKnob .attachTo (processor.apvts, ParamIDs::genSrcUniSpread (generator));
    attKnob       .attachTo (processor.apvts, ParamIDs::genSrcAtt       (generator));
    decKnob       .attachTo (processor.apvts, ParamIDs::genSrcDec       (generator));
    susKnob       .attachTo (processor.apvts, ParamIDs::genSrcSus       (generator));
    relKnob       .attachTo (processor.apvts, ParamIDs::genSrcRel       (generator));
    levelKnob     .attachTo (processor.apvts, ParamIDs::generatorLevel  (generator));
    generatorPan  .attachTo (processor.apvts, ParamIDs::generatorPan    (generator));

    // FX chain (filters are just another selectable effect type)
    const auto& gen = processor.generators[(size_t) generator];
    for (int x = 0; x < gen.numFx; ++x)
        addFxCard (x, gen.fxTypes[(size_t) x]);

    addAndMakeVisible (addFxBtn);
    addFxBtn.onClick = [this]
    {
        auto& s = processor.generators[(size_t) generator];
        if (s.numFx >= MAX_GENERATOR_FX) return;

        juce::PopupMenu menu;
        menu.addItem (1, "Distortion");
        menu.addItem (2, "Compressor");
        menu.addItem (3, "Gate");
        menu.addItem (4, "Reverb");
        menu.addItem (5, "Filter");

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addFxBtn),
            [this] (int result)
            {
                if (result == 0) return;
                const FxType types[] = { FxType::Distortion, FxType::Compressor,
                                         FxType::Gate, FxType::Reverb, FxType::Filter };
                auto& s  = processor.generators[(size_t) generator];
                const int x = s.numFx++;
                s.fxTypes[(size_t) x] = types[result - 1];
                addFxCard (x, types[result - 1]);
                if (onLayoutChanged) onLayoutChanged();
            });
    };

    srcTypeBox.onChange = [this]
    {
        const bool isSample = (srcTypeBox.getSelectedItemIndex() == (int) SourceType::Sample);
        if (! isSample) lastWaveformIndex = srcTypeBox.getSelectedItemIndex();

        nameLabel.setText (isSample ? "Sampler" : "Synth", juce::dontSendNotification);

        srcTypeBox.setVisible      (! isSample);
        loadSampleBtn.setVisible   (isSample);
        sampleFileLabel.setVisible (isSample);

        synthModeBtn.setToggleState   (! isSample, juce::dontSendNotification);
        samplerModeBtn.setToggleState (  isSample, juce::dontSendNotification);

        resized();
        if (onLayoutChanged) onLayoutChanged();
    };
    srcTypeBox.onChange();
}

GeneratorCard::~GeneratorCard() {}

void GeneratorCard::addFxCard (int arrayIndex, FxType type)
{
    fxCards[(size_t) arrayIndex] = std::make_unique<GeneratorFxCard> (processor, generator, arrayIndex);
    addAndMakeVisible (*fxCards[(size_t) arrayIndex]);
    fxCards[(size_t) arrayIndex]->showForType (type);
    fxCards[(size_t) arrayIndex]->onRemove = [this, arrayIndex]
    {
        auto& s = processor.generators[(size_t) generator];
        for (int j = arrayIndex; j < s.numFx - 1; ++j)
        {
            s.fxTypes[(size_t) j] = s.fxTypes[(size_t) (j + 1)];
            addFxCard (j, s.fxTypes[(size_t) j]);
        }
        s.fxTypes[(size_t) (--s.numFx)] = FxType::None;
        fxCards[(size_t) s.numFx] = nullptr;
        if (onLayoutChanged) onLayoutChanged();
    };
}

void GeneratorCard::openFilePicker()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Load Sample",
        juce::File::getSpecialLocation (juce::File::userMusicDirectory),
        "*.wav;*.aif;*.aiff;*.mp3;*.flac");

    fileChooser->launchAsync (juce::FileBrowserComponent::openMode
                            | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            const auto f = fc.getResult();
            if (f.existsAsFile())
            {
                processor.loadSample (generator, f);
                sampleFileLabel.setText (f.getFileName(), juce::dontSendNotification);
            }
        });
}

int GeneratorCard::preferredHeight() const noexcept
{
    const auto& gen = processor.generators[(size_t) generator];
    int fxHeight = 0;
    for (int x = 0; x < gen.numFx; ++x)
        fxHeight += (fxCards[(size_t) x] ? fxCards[(size_t) x]->preferredHeight() : GeneratorFxCard::CARD_H) + 4;

    return HEADER_H + SOURCE_H
         + ARROW_H
         + fxHeight
         + (gen.numFx < MAX_GENERATOR_FX ? 32 : 0)
         + FOOTER_H + 8;
}

void GeneratorCard::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::overlay);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 8.0f);
    g.setColour (ViolentColours::accent.withAlpha (0.6f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 8.0f, 1.5f);

    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (adsrBoxBounds.toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (adsrBoxBounds.toFloat().reduced (2.0f), 5.0f, 1.0f);

    drawRoutingArrow (g, effectArrowY);
}

void GeneratorCard::drawRoutingArrow (juce::Graphics& g, int y) const
{
    drawGeneratorRoutingArrow (g, getWidth(), y);
}

void GeneratorCard::resized()
{
    auto a = getLocalBounds().reduced (6, 4);

    // Header
    auto hdr = a.removeFromTop (HEADER_H);
    enableBtn.setBounds (hdr.removeFromRight (40).reduced (2, 4));
    removeBtn.setBounds (hdr.removeFromLeft (28).withSizeKeepingCentre (20, 20));
    nameLabel.setBounds (hdr.removeFromLeft (70));
    synthModeBtn.setBounds   (hdr.removeFromLeft (55).reduced (2, 4));
    samplerModeBtn.setBounds (hdr.removeFromLeft (65).reduced (2, 4));
    if (srcTypeBox.isVisible())
        srcTypeBox.setBounds (hdr.removeFromLeft (90).reduced (2, 4));
    if (loadSampleBtn.isVisible())
    {
        loadSampleBtn.setBounds (hdr.removeFromLeft (70).reduced (2, 4));
        sampleFileLabel.setBounds (hdr.removeFromLeft (110).reduced (2, 4));
    }
    waveformView.setBounds (hdr.reduced (6, 4));

    a.removeFromTop (4);

    // Source knobs row 1
    auto row1 = a.removeFromTop (58);
    const int r1w = row1.getWidth() / 10;
    gainKnob    .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    octKnob     .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    semiKnob    .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    detKnob     .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    phaseKnob   .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    pwKnob      .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    panKnob     .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    velKnob     .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    uniKnob     .setBounds (row1.removeFromLeft (r1w).reduced (2, 1));
    uniSpreadKnob.setBounds (row1.reduced (2, 1));

    a.removeFromTop (4);

    // ADSR box — aligned to the same column grid as the source knobs above
    auto adsr = a.removeFromTop (58);
    adsrBoxBounds = adsr;
    attKnob.setBounds (adsr.removeFromLeft (r1w).reduced (2, 1));
    decKnob.setBounds (adsr.removeFromLeft (r1w).reduced (2, 1));
    susKnob.setBounds (adsr.removeFromLeft (r1w).reduced (2, 1));
    relKnob.setBounds (adsr.removeFromLeft (r1w).reduced (2, 1));

    effectArrowY = a.removeFromTop (ARROW_H).getCentreY();

    // FX (filters are just another selectable effect type in this chain)
    const auto& gen = processor.generators[(size_t) generator];
    for (int x = 0; x < MAX_GENERATOR_FX; ++x)
    {
        if (fxCards[(size_t) x])
        {
            fxCards[(size_t) x]->setBounds (a.removeFromTop (fxCards[(size_t) x]->preferredHeight()));
            a.removeFromTop (4);
        }
    }
    if (gen.numFx < MAX_GENERATOR_FX)
    {
        addFxBtn.setBounds (a.removeFromTop (28).reduced (4, 2));
        a.removeFromTop (4);
    }
    addFxBtn.setVisible (gen.numFx < MAX_GENERATOR_FX);

    // Footer: level + pan — aligned to the same column grid as everything above
    a.removeFromTop (4);
    levelKnob.setBounds (a.removeFromLeft (r1w).reduced (2, 1));
    a.removeFromLeft (r1w * 4);
    generatorPan.setBounds (a.removeFromLeft (r1w).reduced (2, 1));
}

//==============================================================================
// GeneratorUnit
//==============================================================================
GeneratorUnit::GeneratorUnit (ViolentAudioProcessor& p, int generatorIdx)
    : processor (p), generator (generatorIdx), card (p, generatorIdx)
{
    addAndMakeVisible (card);
    card.onRemove = [this] { if (onRemove) onRemove(); };
    card.onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };

    addAndMakeVisible (addMidiBtn);
    addMidiBtn.onClick = [this]
    {
        if (auto* param = dynamic_cast<juce::AudioParameterBool*> (
                processor.apvts.getParameter (ParamIDs::genMidiModEnabled (generator))))
            *param = true;
        setMidiRowPresent (true);
        if (onLayoutChanged) onLayoutChanged();
    };

    const bool present = processor.apvts.getRawParameterValue (ParamIDs::genMidiModEnabled (generator))->load() > 0.5f;
    setMidiRowPresent (present);
}

void GeneratorUnit::setMidiRowPresent (bool present)
{
    if (present)
    {
        if (! midiRow)
        {
            midiRow = std::make_unique<GeneratorMidiRow> (processor, generator);
            addAndMakeVisible (*midiRow);
            midiRow->onRemove = [this]
            {
                if (auto* param = dynamic_cast<juce::AudioParameterBool*> (
                        processor.apvts.getParameter (ParamIDs::genMidiModEnabled (generator))))
                    *param = false;
                setMidiRowPresent (false);
                if (onLayoutChanged) onLayoutChanged();
            };
        }
    }
    else
    {
        midiRow = nullptr;
    }
    addMidiBtn.setVisible (! present);
}

int GeneratorUnit::preferredHeight() const noexcept
{
    if (midiRow)
        return GeneratorMidiRow::ROW_H + ARROW_H + card.preferredHeight();
    return ADD_MIDI_BTN_H + 4 + card.preferredHeight();
}

void GeneratorUnit::paint (juce::Graphics& g)
{
    if (midiRow)
        drawGeneratorRoutingArrow (g, getWidth(), arrowY);
}

void GeneratorUnit::resized()
{
    auto a = getLocalBounds();

    if (midiRow)
    {
        midiRow->setBounds (a.removeFromTop (GeneratorMidiRow::ROW_H));
        arrowY = a.removeFromTop (ARROW_H).getCentreY();
    }
    else
    {
        addMidiBtn.setBounds (a.removeFromTop (ADD_MIDI_BTN_H).reduced (4, 2));
        a.removeFromTop (4);
    }

    card.setBounds (a);
}

//==============================================================================
// GeneratorPanel
//==============================================================================
GeneratorPanel::GeneratorPanel (ViolentAudioProcessor& p) : processor (p)
{
    addAndMakeVisible (addBtn);
    addBtn.onClick = [this]
    {
        if (processor.numActiveGenerators >= MAX_GENERATORS) return;
        ++processor.numActiveGenerators;
        rebuild();
        if (onLayoutChanged) onLayoutChanged();
    };
    rebuild();
}

void GeneratorPanel::refreshFromState()
{
    rebuild (true);
}

void GeneratorPanel::rebuild (bool forceRecreate)
{
    const int n = processor.numActiveGenerators;
    for (int s = 0; s < MAX_GENERATORS; ++s)
    {
        if (s < n)
        {
            if (!units[(size_t) s] || forceRecreate)
            {
                units[(size_t) s] = std::make_unique<GeneratorUnit> (processor, s);
                addAndMakeVisible (*units[(size_t) s]);
                units[(size_t) s]->onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
                units[(size_t) s]->onRemove = [this, s]
                {
                    if (processor.numActiveGenerators <= 1) return;
                    // Shift generator states down
                    for (int j = s; j < processor.numActiveGenerators - 1; ++j)
                        processor.generators[(size_t) j] = processor.generators[(size_t) (j + 1)];
                    processor.generators[(size_t) (--processor.numActiveGenerators)] = {};
                    rebuild();
                    if (onLayoutChanged) onLayoutChanged();
                };
            }
        }
        else
        {
            units[(size_t) s] = nullptr;
        }
    }
    resized();
}

int GeneratorPanel::preferredHeight() const noexcept
{
    int h = 8;
    for (int s = 0; s < processor.numActiveGenerators; ++s)
        if (units[(size_t) s]) h += units[(size_t) s]->preferredHeight() + 8;
    h += 40; // add button
    return h;
}

void GeneratorPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    for (int s = 0; s < processor.numActiveGenerators; ++s)
    {
        if (!units[(size_t) s]) continue;
        const int h = units[(size_t) s]->preferredHeight();
        units[(size_t) s]->setBounds (a.removeFromTop (h));
        a.removeFromTop (8);
    }
    if (processor.numActiveGenerators < MAX_GENERATORS)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numActiveGenerators < MAX_GENERATORS);
}

//==============================================================================
// MasterFilterRow
//==============================================================================
MasterFilterRow::MasterFilterRow (ViolentAudioProcessor& p, int filterSlot)
    : processor (p), slot (filterSlot),
      block (p.apvts, "Filter " + juce::String (filterSlot + 1),
             ParamIDs::masterFltType (filterSlot),
             ParamIDs::masterFltCut  (filterSlot),
             ParamIDs::masterFltRes  (filterSlot))
{
    addAndMakeVisible (block);
    block.onRemove = [this] { if (onRemove) onRemove(); };

    routingLabel.setText ("Applies to:", juce::dontSendNotification);
    routingLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    routingLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f)));
    addAndMakeVisible (routingLabel);

    auto& mf = processor.masterFilters[(size_t) slot];
    for (int g = 0; g < MAX_GENERATORS; ++g)
    {
        auto& btn = routingBtns[(size_t) g];
        btn.setButtonText (juce::String (g + 1));
        btn.setClickingTogglesState (true);
        btn.setToggleState (mf.routing[(size_t) g], juce::dontSendNotification);
        btn.onClick = [this, g]
        {
            processor.masterFilters[(size_t) slot].routing[(size_t) g] = routingBtns[(size_t) g].getToggleState();
        };
        addAndMakeVisible (btn);
    }
}

void MasterFilterRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
}

void MasterFilterRow::resized()
{
    auto a = getLocalBounds();

    auto routingRow = a.removeFromBottom (30).reduced (4, 2);
    block.setBounds (a);

    routingLabel.setBounds (routingRow.removeFromLeft (70));
    for (auto& btn : routingBtns)
        btn.setBounds (routingRow.removeFromLeft (26).reduced (2, 2));
}

//==============================================================================
// MasterFilterPanel
//==============================================================================
MasterFilterPanel::MasterFilterPanel (ViolentAudioProcessor& p) : processor (p)
{
    sectionLabel.setText ("MASTER FILTERS - applied last, after every generator",
                           juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    addAndMakeVisible (addBtn);
    addBtn.onClick = [this]
    {
        if (processor.numMasterFilters >= MAX_MASTER_FILTERS) return;
        ++processor.numMasterFilters;
        rebuild();
        if (onLayoutChanged) onLayoutChanged();
    };
    rebuild();
}

void MasterFilterPanel::refreshFromState()
{
    rebuild (true);
}

void MasterFilterPanel::rebuild (bool forceRecreate)
{
    const int n = processor.numMasterFilters;
    for (int f = 0; f < MAX_MASTER_FILTERS; ++f)
    {
        if (f < n)
        {
            if (!rows[(size_t) f] || forceRecreate)
            {
                rows[(size_t) f] = std::make_unique<MasterFilterRow> (processor, f);
                addAndMakeVisible (*rows[(size_t) f]);
                rows[(size_t) f]->onRemove = [this, f]
                {
                    for (int j = f; j < processor.numMasterFilters - 1; ++j)
                        processor.masterFilters[(size_t) j] = processor.masterFilters[(size_t) (j + 1)];
                    processor.masterFilters[(size_t) (--processor.numMasterFilters)] = {};
                    rebuild();
                    if (onLayoutChanged) onLayoutChanged();
                };
            }
        }
        else
        {
            rows[(size_t) f] = nullptr;
        }
    }
    resized();
}

int MasterFilterPanel::preferredHeight() const noexcept
{
    int h = 8 + 24; // section label
    for (int f = 0; f < processor.numMasterFilters; ++f)
        if (rows[(size_t) f]) h += MasterFilterRow::ROW_H + 8;
    h += 40; // add button
    return h;
}

void MasterFilterPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    sectionLabel.setBounds (a.removeFromTop (24));
    for (int f = 0; f < processor.numMasterFilters; ++f)
    {
        if (!rows[(size_t) f]) continue;
        rows[(size_t) f]->setBounds (a.removeFromTop (MasterFilterRow::ROW_H));
        a.removeFromTop (8);
    }
    if (processor.numMasterFilters < MAX_MASTER_FILTERS)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numMasterFilters < MAX_MASTER_FILTERS);
}

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

//==============================================================================
// ScalableRackComponent
//==============================================================================
ScalableRackComponent::ScalableRackComponent (ViolentAudioProcessor& p)
    : processor (p), generatorPanel (p), masterFilterPanel (p), mixerPanel (p)
{
    addAndMakeVisible (generatorPanel);
    addAndMakeVisible (masterFilterPanel);
    addAndMakeVisible (mixerPanel);

    generatorPanel.onLayoutChanged = [this] { mixerPanel.rebuild(); if (onLayoutChanged) onLayoutChanged(); };
    masterFilterPanel.onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
}

int ScalableRackComponent::preferredHeight() const noexcept
{
    return generatorPanel.preferredHeight() + masterFilterPanel.preferredHeight() + MixerPanel::PANEL_H;
}

void ScalableRackComponent::refreshFromState()
{
    generatorPanel.refreshFromState();
    masterFilterPanel.refreshFromState();
    mixerPanel.rebuild();
    resized();
}

void ScalableRackComponent::resized()
{
    const int genH = generatorPanel.preferredHeight();
    generatorPanel.setBounds (0, 0, BASE_WIDTH, genH);
    generatorPanel.resized();

    const int mfH = masterFilterPanel.preferredHeight();
    masterFilterPanel.setBounds (0, genH, BASE_WIDTH, mfH);
    masterFilterPanel.resized();

    mixerPanel.setBounds (0, genH + mfH, BASE_WIDTH, MixerPanel::PANEL_H);
    mixerPanel.resized();
}

//==============================================================================
// NavPanel
//==============================================================================
NavPanel::NavPanel (ViolentAudioProcessor& p, ScalableRackComponent& r,
                     juce::Component& scrollSpaceIn, juce::Viewport& viewportIn)
    : processor (p), rack (r), scrollSpace (scrollSpaceIn), viewport (viewportIn)
{
    refreshFromState();
}

void NavPanel::addEntry (const juce::String& label, juce::Component* target)
{
    auto btn = std::make_unique<juce::TextButton> (label);
    btn->setRepaintsOnMouseActivity (true);
    btn->onClick = [this, target]
    {
        if (target == nullptr) return;
        const auto pos = scrollSpace.getLocalPoint (target, juce::Point<int> (0, 0));
        viewport.setViewPosition (0, juce::jmax (0, pos.y - 8));
    };
    addAndMakeVisible (*btn);
    entries.push_back ({ std::move (btn), juce::Component::SafePointer<juce::Component> (target) });
}

void NavPanel::refreshFromState()
{
    entries.clear();

    auto& generatorPanel = rack.getGeneratorPanel();
    for (int g = 0; g < generatorPanel.getNumUnits(); ++g)
    {
        auto* unit = generatorPanel.getUnit (g);
        if (unit == nullptr) continue;

        if (auto* midiRow = unit->getMidiRowComponent())
            addEntry ("MIDI", midiRow);

        auto& card = unit->getCard();
        addEntry (card.getSourceTypeLabel(), &card);

        const auto& gen = processor.generators[(size_t) g];
        for (int x = 0; x < gen.numFx; ++x)
            addEntry (gen.fxTypes[(size_t) x] == FxType::Filter ? "Filter" : "Effect", card.getFxCard (x));
    }

    auto& masterFilterPanel = rack.getMasterFilterPanel();
    for (int m = 0; m < masterFilterPanel.getNumRows(); ++m)
        addEntry ("Master Filter", masterFilterPanel.getRow (m));

    setSize (WIDTH, preferredHeight());
    resized();
}

int NavPanel::preferredHeight() const noexcept
{
    return 4 + (int) entries.size() * 28;
}

void NavPanel::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);
}

void NavPanel::resized()
{
    auto a = getLocalBounds().reduced (4, 0);
    for (auto& e : entries)
        e.button->setBounds (a.removeFromTop (24).reduced (0, 1));
}

//==============================================================================
// ViolentAudioProcessorEditor
//==============================================================================
ViolentAudioProcessorEditor::ViolentAudioProcessorEditor (ViolentAudioProcessor& p)
    : AudioProcessorEditor (p), processor (p), rack (p), navPanel (p, rack, rackScaler, rackViewport)
{
    setLookAndFeel (&laf);
    setResizable (false, true);
    setResizeLimits (NavPanel::WIDTH + juce::roundToInt (ScalableRackComponent::BASE_WIDTH * 0.6f), 200,
                      NavPanel::WIDTH + juce::roundToInt (ScalableRackComponent::BASE_WIDTH * 1.5f), MAX_WINDOW_H);

    rackViewport.setViewedComponent (&rackScaler, false);
    rackViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (rackViewport);

    navViewport.setViewedComponent (&navPanel, false);
    navViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (navViewport);

    addAndMakeVisible (meter);

    previewBtn.setButtonText (juce::String (juce::CharPointer_UTF8 ("\xE2\x96\xB6")));
    previewBtn.setClickingTogglesState (true);
    previewBtn.setToggleState (false, juce::dontSendNotification);
    previewBtn.onClick = [this]
    {
        processor.previewActive.store (previewBtn.getToggleState(), std::memory_order_relaxed);
    };
    addAndMakeVisible (previewBtn);

    for (const auto& t : { "Arpeggios", "Low Notes", "Long Single Notes", "Chords" })
        previewPatternBox.addItem (t, previewPatternBox.getNumItems() + 1);
    previewPatternBox.setRepaintsOnMouseActivity (true);
    previewPatternBox.setSelectedItemIndex (0, juce::dontSendNotification);
    previewPatternBox.onChange = [this]
    {
        processor.setPreviewPattern (previewPatternBox.getSelectedItemIndex());
    };
    addAndMakeVisible (previewPatternBox);

    presetBox.setRepaintsOnMouseActivity (true);
    presetBox.setTextWhenNothingSelected ("Select preset...");
    presetBox.setTextWhenNoChoicesAvailable ("No presets");
    presetBox.onChange = [this] { loadSelectedPreset(); };
    addAndMakeVisible (presetBox);
    refreshPresetList();

    savePresetBtn.onClick = [this] { promptAndSavePreset(); };
    addAndMakeVisible (savePresetBtn);

    zoomOutBtn.onClick = [this] { setUiScale (uiScale - 0.1f); };
    zoomInBtn.onClick  = [this] { setUiScale (uiScale + 0.1f); };
    addAndMakeVisible (zoomOutBtn);
    addAndMakeVisible (zoomInBtn);

    zoomLabel.setJustificationType (juce::Justification::centred);
    zoomLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    zoomLabel.setColour (juce::Label::backgroundColourId, ViolentColours::overlay);
    zoomLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (14.0f).withStyle ("Bold")));
    addAndMakeVisible (zoomLabel);
    zoomLabel.setVisible (false);

    rack.onLayoutChanged = [this] { navPanel.refreshFromState(); updateHeight(); };

    updateHeight();
}

ViolentAudioProcessorEditor::~ViolentAudioProcessorEditor()
{
    processor.previewActive.store (false, std::memory_order_relaxed);
    setLookAndFeel (nullptr);
}

void ViolentAudioProcessorEditor::updateHeight()
{
    rackScaler.updateLayout (uiScale);

    const int newW = NavPanel::WIDTH + juce::roundToInt (ScalableRackComponent::BASE_WIDTH * uiScale);
    const int newH = juce::jmin (MAX_WINDOW_H, HEADER_H + rackScaler.getHeight());

    if (newW == getWidth() && newH == getHeight())
        resized(); // size unchanged, but layout/scale may still need re-applying
    else
        setSize (newW, newH);
}

void ViolentAudioProcessorEditor::setUiScale (float newScale)
{
    newScale = juce::jlimit (0.6f, 1.5f, newScale);
    if (juce::approximatelyEqual (newScale, uiScale)) return;

    uiScale = newScale;
    updateHeight();

    zoomLabel.setText (juce::String (juce::roundToInt (uiScale * 100.0f)) + "%", juce::dontSendNotification);
    zoomLabel.setVisible (true);
    startTimer (5000);
}

void ViolentAudioProcessorEditor::timerCallback()
{
    zoomLabel.setVisible (false);
    stopTimer();
}

void ViolentAudioProcessorEditor::refreshPresetList()
{
    presetBox.clear (juce::dontSendNotification);
    const auto names = processor.getPresetNames();
    int idToSelect = 0;
    for (int i = 0; i < names.size(); ++i)
    {
        presetBox.addItem (names[i], i + 1);
        if (names[i] == processor.currentPresetName)
            idToSelect = i + 1;
    }
    presetBox.setSelectedId (idToSelect, juce::dontSendNotification);
}

void ViolentAudioProcessorEditor::loadSelectedPreset()
{
    const auto name = presetBox.getText();
    if (name.isEmpty()) return;

    if (processor.loadPreset (name))
    {
        rack.refreshFromState();
        navPanel.refreshFromState();
        updateHeight();
    }
}

void ViolentAudioProcessorEditor::promptAndSavePreset()
{
    auto* aw = new juce::AlertWindow ("Save Preset", "Enter a name for this preset:", juce::AlertWindow::NoIcon);
    aw->addTextEditor ("name", processor.currentPresetName.isNotEmpty() ? processor.currentPresetName : "My Preset");
    aw->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (false, juce::ModalCallbackFunction::create ([this, aw] (int result)
    {
        if (result == 1)
        {
            const auto name = aw->getTextEditorContents ("name").trim();
            if (name.isNotEmpty() && processor.savePreset (name))
                refreshPresetList();
        }
    }), true);

    if (auto* nameEditor = aw->getTextEditor ("name"))
    {
        nameEditor->selectAll();
        nameEditor->grabKeyboardFocus();
    }
}

void ViolentAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);
    g.setColour (ViolentColours::surface);
    g.fillRect (0, 0, getWidth(), 50);
    g.setColour (ViolentColours::accent);
    g.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (22.0f).withStyle ("Bold")));
    g.drawText ("VIOLENT", 16, 0, 120, 50, juce::Justification::centredLeft);
    g.setColour (ViolentColours::overlay);
    g.fillRect (0, 50, getWidth(), 2);
}

void ViolentAudioProcessorEditor::resized()
{
    meter.setBounds (getWidth() - 44, 8, 36, 34);
    meter.setLevels (processor.levelL.load (std::memory_order_relaxed),
                     processor.levelR.load (std::memory_order_relaxed));

    previewBtn.setBounds (getWidth() - 84, 12, 30, 26);
    previewPatternBox.setBounds (getWidth() - 234, 12, 140, 26);

    presetBox.setBounds (150, 12, 180, 26);
    savePresetBtn.setBounds (336, 12, 56, 26);

    zoomOutBtn.setBounds (savePresetBtn.getRight() + 16, 12, 26, 26);
    zoomInBtn.setBounds (zoomOutBtn.getRight() + 4, 12, 26, 26);
    zoomLabel.setBounds (zoomInBtn.getRight() + 8, 12, 60, 26);

    navViewport.setBounds (0, HEADER_H, NavPanel::WIDTH, getHeight() - HEADER_H);
    rackViewport.setBounds (NavPanel::WIDTH, HEADER_H, getWidth() - NavPanel::WIDTH, getHeight() - HEADER_H);
}

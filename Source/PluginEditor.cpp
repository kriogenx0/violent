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
    const float radius  = static_cast<float> (juce::jmin (width, height)) * 0.5f - 4.0f;
    const float centreX = static_cast<float> (x) + static_cast<float> (width)  * 0.5f;
    const float centreY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const float rx = centreX - radius, ry = centreY - radius, rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (ViolentColours::overlay);
    g.fillEllipse (rx, ry, rw, rw);

    juce::Path trackArc;
    trackArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                             0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (ViolentColours::surface);
    g.strokePath (trackArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    juce::Colour arc = slider.findColour (juce::Slider::rotarySliderFillColourId, false);
    if (!arc.isOpaque()) arc = ViolentColours::accent;

    juce::Path valArc;
    valArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                          0.0f, rotaryStartAngle, angle, true);
    g.setColour (arc);
    g.strokePath (valArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    const float thumbR = 4.0f;
    const float thumbX = centreX + (radius - 8.0f) * std::cos (angle - juce::MathConstants<float>::halfPi);
    const float thumbY = centreY + (radius - 8.0f) * std::sin (angle - juce::MathConstants<float>::halfPi);
    g.setColour (ViolentColours::text);
    g.fillEllipse (thumbX - thumbR, thumbY - thumbR, thumbR * 2.0f, thumbR * 2.0f);
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

void ViolentLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& btn,
                                            bool, bool)
{
    const bool on = btn.getToggleState();
    const auto b  = btn.getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (on ? ViolentColours::accent : ViolentColours::overlay);
    g.fillRoundedRectangle (b, b.getHeight() * 0.5f);
    g.setColour (on ? ViolentColours::background : ViolentColours::text);
    g.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    g.drawFittedText (btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred, 1);
}

void ViolentLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& btn,
                                                const juce::Colour&, bool highlighted, bool)
{
    const auto b  = btn.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = btn.getToggleState();
    g.setColour (on ? ViolentColours::accent : (highlighted ? ViolentColours::overlay : ViolentColours::surface));
    g.fillRoundedRectangle (b, 6.0f);
    g.setColour (on ? ViolentColours::accent.brighter (0.2f) : ViolentColours::overlay);
    g.drawRoundedRectangle (b, 6.0f, 1.0f);
}

juce::Font ViolentLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
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
// SourceModeButton
//==============================================================================
void SourceModeButton::paintButton (juce::Graphics& g, bool highlighted, bool)
{
    const bool on = getToggleState();
    const auto b  = getLocalBounds().toFloat().reduced (1.0f);

    g.setColour (on ? ViolentColours::accent : (highlighted ? ViolentColours::overlay : ViolentColours::surface));
    g.fillRoundedRectangle (b, 5.0f);
    g.setColour (on ? ViolentColours::accent.brighter (0.2f) : ViolentColours::overlay);
    g.drawRoundedRectangle (b, 5.0f, 1.0f);

    g.setColour (on ? ViolentColours::background : ViolentColours::text);
    const auto icoArea = b.reduced (7.0f);

    if (icon == Icon::Oscillator)
    {
        juce::Path p;
        constexpr int steps = 20;
        for (int i = 0; i <= steps; ++i)
        {
            const float t = (float) i / (float) steps;
            const float x = icoArea.getX() + t * icoArea.getWidth();
            const float y = icoArea.getCentreY()
                           - std::sin (t * juce::MathConstants<float>::twoPi) * icoArea.getHeight() * 0.5f;
            if (i == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
        }
        g.strokePath (p, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else
    {
        constexpr int bars = 5;
        const float gap = icoArea.getWidth() * 0.08f;
        const float bw  = (icoArea.getWidth() - gap * (bars - 1)) / (float) bars;
        for (int i = 0; i < bars; ++i)
        {
            const float h = icoArea.getHeight() * (0.35f + 0.65f * std::abs (std::sin ((float) (i + 1) * 1.1f)));
            const float x = icoArea.getX() + (float) i * (bw + gap);
            g.fillRoundedRectangle (x, icoArea.getCentreY() - h * 0.5f, bw, h, bw * 0.3f);
        }
    }
}

//==============================================================================
// FxBusCard
//==============================================================================
FxBusCard::FxBusCard (ViolentAudioProcessor& p, int busIdx)
    : processor (p), bus (busIdx)
{
    titleLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    titleLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (titleLabel);

    removeBtn.setColour (juce::TextButton::textColourOffId, ViolentColours::red);
    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    for (const auto& t : { "Soft Clip", "Hard Clip", "Fuzz" })
        distTypeBox.addItem (t, distTypeBox.getNumItems() + 1);
    addChildComponent (distTypeBox);

    for (auto* k : { &driveKnob, &toneKnob, &levelKnob,
                     &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob,
                     &roomKnob, &dampingKnob, &wetKnob, &widthKnob })
        addChildComponent (*k);

    driveAtt     = std::make_unique<SA> (processor.apvts, ParamIDs::busDrive    (bus), driveKnob.slider);
    toneAtt      = std::make_unique<SA> (processor.apvts, ParamIDs::busTone     (bus), toneKnob.slider);
    levelAtt     = std::make_unique<SA> (processor.apvts, ParamIDs::busLevel    (bus), levelKnob.slider);
    distTypeAtt  = std::make_unique<CA> (processor.apvts, ParamIDs::busDistType (bus), distTypeBox);
    threshAtt    = std::make_unique<SA> (processor.apvts, ParamIDs::busThresh   (bus), threshKnob.slider);
    ratioAtt     = std::make_unique<SA> (processor.apvts, ParamIDs::busRatio    (bus), ratioKnob.slider);
    attackKnobAtt= std::make_unique<SA> (processor.apvts, ParamIDs::busAttack   (bus), attackKnob.slider);
    releaseAtt   = std::make_unique<SA> (processor.apvts, ParamIDs::busRelease  (bus), releaseKnob.slider);
    makeupAtt    = std::make_unique<SA> (processor.apvts, ParamIDs::busMakeup   (bus), makeupKnob.slider);
    roomAtt      = std::make_unique<SA> (processor.apvts, ParamIDs::busRoom     (bus), roomKnob.slider);
    dampingAtt   = std::make_unique<SA> (processor.apvts, ParamIDs::busDamping  (bus), dampingKnob.slider);
    wetAtt       = std::make_unique<SA> (processor.apvts, ParamIDs::busWet      (bus), wetKnob.slider);
    widthAtt     = std::make_unique<SA> (processor.apvts, ParamIDs::busWidth    (bus), widthKnob.slider);

    showForType (processor.fxBusTypes[(size_t) bus]);
}

FxBusCard::~FxBusCard() {}

void FxBusCard::setAllInvisible()
{
    for (auto* k : { &driveKnob, &toneKnob, &levelKnob,
                     &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob,
                     &roomKnob, &dampingKnob, &wetKnob, &widthKnob })
        k->setVisible (false);
    distTypeBox.setVisible (false);
}

void FxBusCard::showForType (FxType t)
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
        default: break;
    }
}

void FxBusCard::layoutKnobs (std::initializer_list<LabelledKnob*> ks, juce::Rectangle<int> a)
{
    const int w = a.getWidth() / (int) ks.size();
    for (auto* k : ks) k->setBounds (a.removeFromLeft (w).reduced (3, 2));
}

void FxBusCard::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f, 1.0f);
}

void FxBusCard::resized()
{
    auto a = getLocalBounds().reduced (4, 3);
    auto hdr = a.removeFromTop (22);
    removeBtn .setBounds (hdr.removeFromLeft (24));
    titleLabel.setBounds (hdr);
    a.removeFromTop (2);

    const FxType t = processor.fxBusTypes[(size_t) bus];
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
        default: break;
    }
}

//==============================================================================
// GeneratorFilterRow
//==============================================================================
GeneratorFilterRow::GeneratorFilterRow (ViolentAudioProcessor& p, int genIdx, int filterSlot)
    : processor (p), gen (genIdx), slot (filterSlot)
{
    nameLabel.setText ("Filter " + juce::String (slot + 1), juce::dontSendNotification);
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (nameLabel);

    removeBtn.setColour (juce::TextButton::textColourOffId, ViolentColours::red);
    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    for (const auto& t : { "LP", "HP", "BP", "Notch" })
        typeBox.addItem (t, typeBox.getNumItems() + 1);
    addAndMakeVisible (typeBox);

    for (auto* k : { &cutoffKnob, &resKnob }) addAndMakeVisible (*k);

    typeAtt   = std::make_unique<CA> (processor.apvts, ParamIDs::genFltType (gen, slot), typeBox);
    cutoffAtt = std::make_unique<SA> (processor.apvts, ParamIDs::genFltCut  (gen, slot), cutoffKnob.slider);
    resAtt    = std::make_unique<SA> (processor.apvts, ParamIDs::genFltRes  (gen, slot), resKnob.slider);
}

GeneratorFilterRow::~GeneratorFilterRow() {}

void GeneratorFilterRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
}

void GeneratorFilterRow::resized()
{
    auto a = getLocalBounds().reduced (4, 3);
    removeBtn .setBounds (a.removeFromLeft  (24));
    nameLabel .setBounds (a.removeFromLeft  (60));
    typeBox   .setBounds (a.removeFromLeft  (70).reduced (2, 4));
    const int kw = a.getWidth() / 2;
    cutoffKnob.setBounds (a.removeFromLeft (kw).reduced (3, 2));
    resKnob   .setBounds (a.reduced (3, 2));
}

//==============================================================================
// GeneratorCard
//==============================================================================
GeneratorCard::GeneratorCard (ViolentAudioProcessor& p, int genIdx)
    : processor (p), gen (genIdx)
{
    nameLabel.setText ("Generator " + juce::String (gen + 1), juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (nameLabel);

    removeBtn.setColour (juce::TextButton::textColourOffId, ViolentColours::red);
    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    enableBtn.setButtonText ("ON");
    enableBtn.setClickingTogglesState (true);
    addAndMakeVisible (enableBtn);
    enableAtt = std::make_unique<BA> (processor.apvts, ParamIDs::genEn (gen), enableBtn);

    // Source mode: Oscillator or Sampler
    addAndMakeVisible (oscModeBtn);
    addAndMakeVisible (samplerModeBtn);
    oscModeBtn.onClick     = [this] { setSourceMode (false); };
    samplerModeBtn.onClick = [this] { setSourceMode (true); };

    // Source type (waveform when in Oscillator mode)
    for (const auto& t : { "Sine", "Saw", "Square", "Triangle", "Noise", "Sample" })
        srcTypeBox.addItem (t, srcTypeBox.getNumItems() + 1);
    addAndMakeVisible (srcTypeBox);
    srcTypeAtt = std::make_unique<CA> (processor.apvts, ParamIDs::genSrcType (gen), srcTypeBox);

    loadSampleBtn.onClick = [this] { openFilePicker(); };
    addChildComponent (loadSampleBtn);
    sampleFileLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addChildComponent (sampleFileLabel);

    // Source knobs
    for (auto* k : { &gainKnob, &octKnob, &semiKnob, &detKnob, &phaseKnob,
                     &pwKnob, &panKnob, &velKnob, &uniKnob, &uniSpreadKnob,
                     &attKnob, &decKnob, &susKnob, &relKnob,
                     &levelKnob, &genPanKnob })
        addAndMakeVisible (*k);

    gainAtt       = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcGain      (gen), gainKnob.slider);
    octAtt        = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcOct       (gen), octKnob.slider);
    semiAtt       = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcSemi      (gen), semiKnob.slider);
    detAtt        = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcDet       (gen), detKnob.slider);
    phaseAtt      = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcPhase     (gen), phaseKnob.slider);
    pwAtt         = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcPW        (gen), pwKnob.slider);
    panSrcAtt     = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcPan       (gen), panKnob.slider);
    velAtt        = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcVel       (gen), velKnob.slider);
    uniAtt        = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcUni       (gen), uniKnob.slider);
    uniSpreadAtt  = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcUniSpread (gen), uniSpreadKnob.slider);
    attAtt        = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcAtt       (gen), attKnob.slider);
    decAtt        = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcDec       (gen), decKnob.slider);
    susAtt        = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcSus       (gen), susKnob.slider);
    relAtt        = std::make_unique<SA> (processor.apvts, ParamIDs::genSrcRel       (gen), relKnob.slider);
    levelAtt      = std::make_unique<SA> (processor.apvts, ParamIDs::genLevel        (gen), levelKnob.slider);
    genPanAtt     = std::make_unique<SA> (processor.apvts, ParamIDs::genPan          (gen), genPanKnob.slider);

    // Filter chain
    const auto& gs = processor.generators[(size_t) gen];
    for (int f = 0; f < gs.numFilters; ++f)
    {
        filterRows[(size_t) f] = std::make_unique<GeneratorFilterRow> (processor, gen, f);
        addAndMakeVisible (*filterRows[(size_t) f]);
        filterRows[(size_t) f]->onRemove = [this, f]
        {
            auto& s = processor.generators[(size_t) gen];
            for (int j = f; j < s.numFilters - 1; ++j)
                filterRows[(size_t) j] = std::make_unique<GeneratorFilterRow> (processor, gen, j + 1);
            filterRows[(size_t) (--s.numFilters)] = nullptr;
            if (onLayoutChanged) onLayoutChanged();
        };
    }

    addAndMakeVisible (addFilterBtn);
    addFilterBtn.onClick = [this]
    {
        auto& s = processor.generators[(size_t) gen];
        if (s.numFilters >= MAX_GEN_FILTERS) return;
        const int f = s.numFilters++;
        filterRows[(size_t) f] = std::make_unique<GeneratorFilterRow> (processor, gen, f);
        addAndMakeVisible (*filterRows[(size_t) f]);
        if (onLayoutChanged) onLayoutChanged();
    };

    srcTypeBox.onChange = [this]
    {
        const bool isSample = (srcTypeBox.getSelectedItemIndex() == (int) SourceType::Sample);
        oscModeBtn.setToggleState (!isSample, juce::dontSendNotification);
        samplerModeBtn.setToggleState (isSample, juce::dontSendNotification);
        srcTypeBox.setVisible (!isSample);
        loadSampleBtn.setVisible (isSample);
        sampleFileLabel.setVisible (isSample);
    };
    srcTypeBox.onChange();

    refreshSends();
}

GeneratorCard::~GeneratorCard() {}

void GeneratorCard::setSourceMode (bool sampler)
{
    const bool alreadySample = (srcTypeBox.getSelectedItemIndex() == (int) SourceType::Sample);
    if (sampler == alreadySample) return;

    srcTypeBox.setSelectedItemIndex (sampler ? (int) SourceType::Sample : (int) SourceType::Sine,
                                      juce::sendNotificationSync);
}

void GeneratorCard::refreshSends()
{
    const int n = processor.numFxBuses;
    for (int b = 0; b < MAX_FX_BUSES; ++b)
    {
        if (b < n)
        {
            const juce::String label = fxTypeName (processor.fxBusTypes[(size_t) b]);
            if (!sendKnobs[(size_t) b])
            {
                sendKnobs[(size_t) b] = std::make_unique<LabelledKnob> (label, ViolentColours::teal);
                addAndMakeVisible (*sendKnobs[(size_t) b]);
                sendAtts[(size_t) b] = std::make_unique<SA> (processor.apvts,
                    ParamIDs::genSend (gen, b), sendKnobs[(size_t) b]->slider);
            }
            else
            {
                sendKnobs[(size_t) b]->nameLabel.setText (label, juce::dontSendNotification);
            }
        }
        else
        {
            sendAtts[(size_t) b]  = nullptr;
            sendKnobs[(size_t) b] = nullptr;
        }
    }
    resized();
    if (onLayoutChanged) onLayoutChanged();
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
                processor.loadSample (gen, f);
                sampleFileLabel.setText (f.getFileName(), juce::dontSendNotification);
            }
        });
}

int GeneratorCard::preferredHeight() const noexcept
{
    const auto& gs = processor.generators[(size_t) gen];
    return HEADER_H + SOURCE_H
         + gs.numFilters * (GeneratorFilterRow::ROW_H + 4)
         + (gs.numFilters < MAX_GEN_FILTERS ? 32 : 0)
         + (processor.numFxBuses > 0 ? SENDS_H + 4 : 0)
         + FOOTER_H + 8;
}

void GeneratorCard::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::overlay);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 8.0f);
    g.setColour (ViolentColours::accent.withAlpha (0.6f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 8.0f, 1.5f);
}

void GeneratorCard::resized()
{
    auto a = getLocalBounds().reduced (6, 4);

    // Header
    auto hdr = a.removeFromTop (HEADER_H);
    removeBtn.setBounds (hdr.removeFromLeft (28));
    nameLabel.setBounds (hdr.removeFromLeft (110));
    enableBtn.setBounds (hdr.removeFromLeft (40).reduced (2, 4));
    oscModeBtn.setBounds (hdr.removeFromLeft (34).reduced (2, 4));
    samplerModeBtn.setBounds (hdr.removeFromLeft (34).reduced (2, 4));
    if (srcTypeBox.isVisible())
        srcTypeBox.setBounds (hdr.removeFromLeft (90).reduced (2, 4));
    if (loadSampleBtn.isVisible())
    {
        loadSampleBtn.setBounds (hdr.removeFromLeft (70).reduced (2, 4));
        sampleFileLabel.setBounds (hdr.reduced (2, 4));
    }

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

    // ADSR row (boxed)
    auto adsr = a.removeFromTop (58);
    const int aw = adsr.getWidth() / 4;
    attKnob.setBounds (adsr.removeFromLeft (aw).reduced (2, 1));
    decKnob.setBounds (adsr.removeFromLeft (aw).reduced (2, 1));
    susKnob.setBounds (adsr.removeFromLeft (aw).reduced (2, 1));
    relKnob.setBounds (adsr.reduced (2, 1));

    a.removeFromTop (6);

    // Filters
    const auto& gs = processor.generators[(size_t) gen];
    for (int f = 0; f < MAX_GEN_FILTERS; ++f)
    {
        if (filterRows[(size_t) f])
        {
            filterRows[(size_t) f]->setBounds (a.removeFromTop (GeneratorFilterRow::ROW_H));
            a.removeFromTop (4);
        }
    }
    if (gs.numFilters < MAX_GEN_FILTERS)
    {
        addFilterBtn.setBounds (a.removeFromTop (28).reduced (4, 2));
        a.removeFromTop (4);
    }
    addFilterBtn.setVisible (gs.numFilters < MAX_GEN_FILTERS);

    // Sends to shared FX buses
    const int numBuses = processor.numFxBuses;
    if (numBuses > 0)
    {
        auto sendsArea = a.removeFromTop (SENDS_H);
        const int sw = sendsArea.getWidth() / numBuses;
        for (int b = 0; b < numBuses; ++b)
            if (sendKnobs[(size_t) b])
                sendKnobs[(size_t) b]->setBounds (sendsArea.removeFromLeft (sw).reduced (2, 1));
        a.removeFromTop (4);
    }

    // Footer: level + pan
    a.removeFromTop (4);
    const int fw = a.getWidth() / 2;
    levelKnob .setBounds (a.removeFromLeft (fw).reduced (4, 2));
    genPanKnob.setBounds (a.reduced (4, 2));
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

void GeneratorPanel::rebuild()
{
    const int n = processor.numActiveGenerators;
    for (int s = 0; s < MAX_GENERATORS; ++s)
    {
        if (s < n)
        {
            if (!cards[(size_t) s])
            {
                cards[(size_t) s] = std::make_unique<GeneratorCard> (processor, s);
                addAndMakeVisible (*cards[(size_t) s]);
                cards[(size_t) s]->onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
                cards[(size_t) s]->onRemove = [this, s]
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
            cards[(size_t) s] = nullptr;
        }
    }
    resized();
}

void GeneratorPanel::refreshAllSends()
{
    for (int s = 0; s < processor.numActiveGenerators; ++s)
        if (cards[(size_t) s]) cards[(size_t) s]->refreshSends();
}

int GeneratorPanel::preferredHeight() const noexcept
{
    int h = 8;
    for (int s = 0; s < processor.numActiveGenerators; ++s)
        if (cards[(size_t) s]) h += cards[(size_t) s]->preferredHeight() + 8;
    h += 40; // add button
    return h;
}

void GeneratorPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    for (int s = 0; s < processor.numActiveGenerators; ++s)
    {
        if (!cards[(size_t) s]) continue;
        const int h = cards[(size_t) s]->preferredHeight();
        cards[(size_t) s]->setBounds (a.removeFromTop (h));
        a.removeFromTop (8);
    }
    if (processor.numActiveGenerators < MAX_GENERATORS)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numActiveGenerators < MAX_GENERATORS);
}

//==============================================================================
// FxRackPanel
//==============================================================================
FxRackPanel::FxRackPanel (ViolentAudioProcessor& p) : processor (p)
{
    titleLabel.setText ("FX Rack", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    titleLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (titleLabel);

    addAndMakeVisible (addBtn);
    addBtn.onClick = [this]
    {
        if (processor.numFxBuses >= MAX_FX_BUSES) return;

        juce::PopupMenu menu;
        menu.addItem (1, "Distortion");
        menu.addItem (2, "Compressor");
        menu.addItem (3, "Gate");
        menu.addItem (4, "Reverb");

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addBtn),
            [this] (int result)
            {
                if (result == 0) return;
                const FxType types[] = { FxType::Distortion, FxType::Compressor,
                                         FxType::Gate, FxType::Reverb };
                const int b = processor.numFxBuses++;
                processor.fxBusTypes[(size_t) b] = types[result - 1];
                rebuild();
                if (onLayoutChanged) onLayoutChanged();
            });
    };
    rebuild();
}

void FxRackPanel::rebuild()
{
    const int n = processor.numFxBuses;
    for (int b = 0; b < MAX_FX_BUSES; ++b)
    {
        if (b < n)
        {
            if (!cards[(size_t) b])
            {
                cards[(size_t) b] = std::make_unique<FxBusCard> (processor, b);
                addAndMakeVisible (*cards[(size_t) b]);
                cards[(size_t) b]->onRemove = [this, b]
                {
                    for (int j = b; j < processor.numFxBuses - 1; ++j)
                        processor.fxBusTypes[(size_t) j] = processor.fxBusTypes[(size_t) (j + 1)];
                    processor.fxBusTypes[(size_t) (--processor.numFxBuses)] = FxType::None;
                    rebuild();
                    if (onLayoutChanged) onLayoutChanged();
                };
            }
        }
        else
        {
            cards[(size_t) b] = nullptr;
        }
    }
    resized();
}

int FxRackPanel::preferredHeight() const noexcept
{
    int h = 8 + 24; // title
    for (int b = 0; b < processor.numFxBuses; ++b)
        if (cards[(size_t) b]) h += FxBusCard::CARD_H + 8;
    h += 40; // add button
    return h;
}

void FxRackPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    titleLabel.setBounds (a.removeFromTop (24));
    a.removeFromTop (4);
    for (int b = 0; b < processor.numFxBuses; ++b)
    {
        if (!cards[(size_t) b]) continue;
        cards[(size_t) b]->setBounds (a.removeFromTop (FxBusCard::CARD_H));
        a.removeFromTop (8);
    }
    if (processor.numFxBuses < MAX_FX_BUSES)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numFxBuses < MAX_FX_BUSES);
}

//==============================================================================
// ViolentAudioProcessorEditor
//==============================================================================
ViolentAudioProcessorEditor::ViolentAudioProcessorEditor (ViolentAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), generatorPanel (p), fxRackPanel (p)
{
    setLookAndFeel (&laf);
    setResizable (false, true);
    setResizeLimits (960, 300, 960, 6000);

    addAndMakeVisible (generatorPanel);
    addAndMakeVisible (fxRackPanel);
    addAndMakeVisible (meter);

    generatorPanel.onLayoutChanged = [this] { updateHeight(); };
    fxRackPanel.onLayoutChanged    = [this] { generatorPanel.refreshAllSends(); updateHeight(); };

    updateHeight();
}

ViolentAudioProcessorEditor::~ViolentAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

int ViolentAudioProcessorEditor::editorHeight() const noexcept
{
    return 52 + generatorPanel.preferredHeight() + fxRackPanel.preferredHeight();
}

void ViolentAudioProcessorEditor::updateHeight()
{
    setSize (getWidth(), editorHeight());
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

    const int genH = generatorPanel.preferredHeight();
    generatorPanel.setBounds (0, 52, getWidth(), genH);
    generatorPanel.resized();

    fxRackPanel.setBounds (0, 52 + genH, getWidth(), fxRackPanel.preferredHeight());
    fxRackPanel.resized();
}

#include "PluginEditor.h"

//==============================================================================
// ViolentLookAndFeel
//==============================================================================
ViolentLookAndFeel::ViolentLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId,   ViolentColours::background);
    setColour (juce::Label::textColourId,                   ViolentColours::text);
    setColour (juce::Slider::thumbColourId,                 ViolentColours::accent);
    setColour (juce::Slider::trackColourId,                 ViolentColours::surface);
    setColour (juce::Slider::backgroundColourId,            ViolentColours::surface);
    setColour (juce::ToggleButton::textColourId,            ViolentColours::text);
    setColour (juce::TextButton::buttonColourId,            ViolentColours::surface);
    setColour (juce::TextButton::buttonOnColourId,          ViolentColours::accent);
    setColour (juce::TextButton::textColourOffId,           ViolentColours::subtext);
    setColour (juce::TextButton::textColourOnId,            ViolentColours::background);
    setColour (juce::ComboBox::backgroundColourId,          ViolentColours::surface);
    setColour (juce::ComboBox::textColourId,                ViolentColours::text);
    setColour (juce::ComboBox::outlineColourId,             ViolentColours::overlay);
    setColour (juce::PopupMenu::backgroundColourId,         ViolentColours::surface);
    setColour (juce::PopupMenu::textColourId,               ViolentColours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, ViolentColours::accent);
    setColour (juce::PopupMenu::highlightedTextColourId,    ViolentColours::background);
}

void ViolentLookAndFeel::drawRotarySlider (juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    const float radius  = static_cast<float> (juce::jmin (width, height)) * 0.5f - 4.0f;
    const float centreX = static_cast<float> (x) + static_cast<float> (width)  * 0.5f;
    const float centreY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const float rx  = centreX - radius;
    const float ry  = centreY - radius;
    const float rw  = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    g.setColour (ViolentColours::overlay);
    g.fillEllipse (rx, ry, rw, rw);

    juce::Path trackArc;
    trackArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                             0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (ViolentColours::surface);
    g.strokePath (trackArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    juce::Colour arcColour = slider.findColour (juce::Slider::rotarySliderFillColourId, false);
    if (!arcColour.isOpaque()) arcColour = ViolentColours::accent;

    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                             0.0f, rotaryStartAngle, angle, true);
    g.setColour (arcColour);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    const float thumbRadius = 4.0f;
    const float thumbX = centreX + (radius - 8.0f) * std::cos (angle - juce::MathConstants<float>::halfPi);
    const float thumbY = centreY + (radius - 8.0f) * std::sin (angle - juce::MathConstants<float>::halfPi);
    g.setColour (ViolentColours::text);
    g.fillEllipse (thumbX - thumbRadius, thumbY - thumbRadius,
                   thumbRadius * 2.0f, thumbRadius * 2.0f);
}

void ViolentLookAndFeel::drawLinearSlider (juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, float, float,
    juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical)
    {
        const float trackW  = 6.0f;
        const float centreX = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
        const float trackTop    = static_cast<float> (y) + 8.0f;
        const float trackBottom = static_cast<float> (y + height) - 8.0f;
        const float trackLength = trackBottom - trackTop;

        g.setColour (ViolentColours::overlay);
        g.fillRoundedRectangle (centreX - trackW * 0.5f, trackTop, trackW, trackLength, 3.0f);

        const float midY = trackTop + trackLength * 0.5f;
        g.setColour (ViolentColours::subtext);
        g.drawHorizontalLine (static_cast<int> (midY), centreX - 8.0f, centreX + 8.0f);

        const float thumbY = sliderPos;
        juce::Colour fillColour = slider.findColour (juce::Slider::rotarySliderFillColourId, false);
        if (!fillColour.isOpaque()) fillColour = ViolentColours::accent;

        if (thumbY < midY)
        {
            g.setColour (fillColour);
            g.fillRoundedRectangle (centreX - trackW * 0.5f, thumbY,
                                    trackW, midY - thumbY, 3.0f);
        }
        else
        {
            g.setColour (fillColour.withAlpha (0.6f));
            g.fillRoundedRectangle (centreX - trackW * 0.5f, midY,
                                    trackW, thumbY - midY, 3.0f);
        }

        g.setColour (ViolentColours::text);
        g.fillRoundedRectangle (centreX - 11.0f, thumbY - 6.0f, 22.0f, 12.0f, 3.0f);
    }
}

void ViolentLookAndFeel::drawToggleButton (juce::Graphics& g,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted, bool)
{
    const bool isOn = button.getToggleState();
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);

    g.setColour (isOn ? ViolentColours::accent : ViolentColours::overlay);
    g.fillRoundedRectangle (bounds, bounds.getHeight() * 0.5f);

    g.setColour (isOn ? ViolentColours::accent.brighter (0.2f) : ViolentColours::subtext);
    g.drawRoundedRectangle (bounds, bounds.getHeight() * 0.5f, 1.5f);

    g.setColour (isOn ? ViolentColours::background : ViolentColours::text);
    g.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds(),
                      juce::Justification::centred, 1);
}

void ViolentLookAndFeel::drawButtonBackground (juce::Graphics& g,
    juce::Button& button,
    const juce::Colour&,
    bool shouldDrawButtonAsHighlighted, bool)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool isOn   = button.getToggleState();

    g.setColour (isOn ? ViolentColours::accent : (shouldDrawButtonAsHighlighted
                                           ? ViolentColours::overlay : ViolentColours::surface));
    g.fillRoundedRectangle (bounds, 6.0f);

    g.setColour (isOn ? ViolentColours::accent.brighter (0.2f) : ViolentColours::overlay);
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
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
    auto area = getLocalBounds();
    nameLabel .setBounds (area.removeFromTop (16));
    valueLabel.setBounds (area.removeFromBottom (14));
    slider    .setBounds (area);
}

//==============================================================================
// Helpers: strip header paint
//==============================================================================
static void paintStripHeader (juce::Graphics& g, juce::Component& strip,
                               const juce::String& title, bool enabled)
{
    g.fillAll (ViolentColours::background);
    g.setColour (enabled ? ViolentColours::overlay : ViolentColours::surface);
    g.fillRoundedRectangle (strip.getLocalBounds().toFloat().reduced (2.0f), 6.0f);
    g.setColour (enabled ? ViolentColours::accent : ViolentColours::subtext);
    g.drawRoundedRectangle (strip.getLocalBounds().toFloat().reduced (2.0f), 6.0f, 1.0f);
}

//==============================================================================
// SynthStrip
//==============================================================================
SynthStrip::SynthStrip (ViolentAudioProcessor& p, int slotIndex)
    : processor (p), slot (slotIndex)
{
    nameLabel.setText ("OSC " + juce::String (slot + 1), juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (nameLabel);

    removeBtn.setColour (juce::TextButton::textColourOffId, ViolentColours::red);
    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    waveLabel.setText ("Wave", juce::dontSendNotification);
    waveLabel.setJustificationType (juce::Justification::centred);
    waveLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (waveLabel);

    for (const auto& w : { "Sine", "Saw", "Square", "Tri", "Noise" })
        waveBox.addItem (w, waveBox.getNumItems() + 1);
    addAndMakeVisible (waveBox);
    waveAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, ParamIDs::synthType (slot), waveBox);

    for (auto* k : { &gainKnob, &octKnob, &semiKnob, &detuneKnob, &phaseKnob,
                     &pwKnob, &panKnob, &velKnob, &uniKnob, &uniSpreadKnob,
                     &attackKnob, &decayKnob, &sustainKnob, &releaseKnob })
        addAndMakeVisible (*k);

    gainAtt      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthGain     (slot), gainKnob.slider);
    octAtt       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthOct      (slot), octKnob.slider);
    semiAtt      = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthSemi     (slot), semiKnob.slider);
    detuneAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthDet      (slot), detuneKnob.slider);
    phaseAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthPhase    (slot), phaseKnob.slider);
    pwAtt        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthPW       (slot), pwKnob.slider);
    panAtt       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthPan      (slot), panKnob.slider);
    velAtt       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthVelS     (slot), velKnob.slider);
    uniAtt       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthUni      (slot), uniKnob.slider);
    uniSpreadAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthUniSpread(slot), uniSpreadKnob.slider);
    attackAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthAtt      (slot), attackKnob.slider);
    decayAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthDec      (slot), decayKnob.slider);
    sustainAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthSus      (slot), sustainKnob.slider);
    releaseAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthRel      (slot), releaseKnob.slider);
}

SynthStrip::~SynthStrip() {}

void SynthStrip::paint (juce::Graphics& g)
{
    paintStripHeader (g, *this, "OSC " + juce::String (slot + 1), true);
}

void SynthStrip::resized()
{
    auto area = getLocalBounds().reduced (6, 4);

    // Header row
    auto header = area.removeFromTop (26);
    removeBtn.setBounds (header.removeFromLeft  (28));
    nameLabel.setBounds (header.removeFromLeft  (72));
    waveLabel.setBounds (header.removeFromLeft  (36));
    waveBox  .setBounds (header.removeFromLeft  (86));

    area.removeFromTop (4);

    // Row 1: Gain | Oct | Semi | Detune | Phase | PW | Pan | Vel | Unison | Spread
    auto row1 = area.removeFromTop (42);
    const int r1w = row1.getWidth() / 10;
    gainKnob     .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    octKnob      .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    semiKnob     .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    detuneKnob   .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    phaseKnob    .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    pwKnob       .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    panKnob      .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    velKnob      .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    uniKnob      .setBounds (row1.removeFromLeft (r1w).reduced (3, 1));
    uniSpreadKnob.setBounds (row1.reduced (3, 1));

    area.removeFromTop (4);

    // Row 2: ADSR
    auto row2 = area;
    const int r2w = row2.getWidth() / 4;
    attackKnob .setBounds (row2.removeFromLeft (r2w).reduced (3, 1));
    decayKnob  .setBounds (row2.removeFromLeft (r2w).reduced (3, 1));
    sustainKnob.setBounds (row2.removeFromLeft (r2w).reduced (3, 1));
    releaseKnob.setBounds (row2.reduced (3, 1));
}

//==============================================================================
// FilterStrip
//==============================================================================
FilterStrip::FilterStrip (ViolentAudioProcessor& p, int slotIndex)
    : processor (p), slot (slotIndex)
{
    nameLabel.setText ("FILTER " + juce::String (slot + 1), juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (nameLabel);

    enableBtn.setButtonText ("ON");
    enableBtn.setClickingTogglesState (true);
    addAndMakeVisible (enableBtn);
    enableAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, ParamIDs::fltEn (slot), enableBtn);

    typeLabel.setText ("Type", juce::dontSendNotification);
    typeLabel.setJustificationType (juce::Justification::centred);
    typeLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (typeLabel);

    for (const auto& t : { "Low Pass", "High Pass", "Band Pass", "Notch" })
        typeBox.addItem (t, typeBox.getNumItems() + 1);
    addAndMakeVisible (typeBox);
    typeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, ParamIDs::fltType (slot), typeBox);

    for (auto* k : { &cutoffKnob, &resonanceKnob })
        addAndMakeVisible (*k);

    cutoffAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::fltCut (slot), cutoffKnob.slider);
    resonanceAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::fltRes (slot), resonanceKnob.slider);
}

FilterStrip::~FilterStrip() {}

void FilterStrip::paint (juce::Graphics& g)
{
    const bool en = processor.apvts.getRawParameterValue (ParamIDs::fltEn (slot))->load() > 0.5f;
    paintStripHeader (g, *this, "FILTER " + juce::String (slot + 1), en);
}

void FilterStrip::resized()
{
    auto area = getLocalBounds().reduced (6, 4);

    auto header = area.removeFromTop (26);
    enableBtn.setBounds (header.removeFromLeft (44));
    nameLabel.setBounds (header.removeFromLeft (90));
    typeLabel.setBounds (header.removeFromLeft (36));
    typeBox  .setBounds (header.removeFromLeft (120));

    area.removeFromTop (4);
    const int knobW = area.getWidth() / 2;
    cutoffKnob   .setBounds (area.removeFromLeft (knobW).reduced (8, 2));
    resonanceKnob.setBounds (area.removeFromLeft (knobW).reduced (8, 2));
}

//==============================================================================
// LFOStrip
//==============================================================================
LFOStrip::LFOStrip (ViolentAudioProcessor& p, int slotIndex)
    : processor (p), slot (slotIndex)
{
    nameLabel.setText ("LFO " + juce::String (slot + 1), juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (nameLabel);

    enableBtn.setButtonText ("ON");
    enableBtn.setClickingTogglesState (true);
    addAndMakeVisible (enableBtn);
    enableAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, ParamIDs::lfoEn (slot), enableBtn);

    shapeLabel.setText ("Shape", juce::dontSendNotification);
    shapeLabel.setJustificationType (juce::Justification::centred);
    shapeLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (shapeLabel);

    for (const auto& s : { "Sine", "Triangle", "Saw", "Square" })
        shapeBox.addItem (s, shapeBox.getNumItems() + 1);
    addAndMakeVisible (shapeBox);
    shapeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, ParamIDs::lfoShape (slot), shapeBox);

    targetLabel.setText ("Target", juce::dontSendNotification);
    targetLabel.setJustificationType (juce::Justification::centred);
    targetLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (targetLabel);

    for (const auto& t : { "None", "Flt Cutoff", "Osc Volume", "Osc Detune" })
        targetBox.addItem (t, targetBox.getNumItems() + 1);
    addAndMakeVisible (targetBox);
    targetAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, ParamIDs::lfoTarget (slot), targetBox);

    tgtSlotLabel.setText ("Slot", juce::dontSendNotification);
    tgtSlotLabel.setJustificationType (juce::Justification::centred);
    tgtSlotLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (tgtSlotLabel);

    for (int i = 1; i <= 4; ++i)
        tgtSlotBox.addItem (juce::String (i), i);
    addAndMakeVisible (tgtSlotBox);
    tgtSlotAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, ParamIDs::lfoTgtSlt (slot), tgtSlotBox);

    for (auto* k : { &rateKnob, &depthKnob })
        addAndMakeVisible (*k);

    rateAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::lfoRate  (slot), rateKnob.slider);
    depthAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::lfoDepth (slot), depthKnob.slider);
}

LFOStrip::~LFOStrip() {}

void LFOStrip::paint (juce::Graphics& g)
{
    const bool en = processor.apvts.getRawParameterValue (ParamIDs::lfoEn (slot))->load() > 0.5f;
    paintStripHeader (g, *this, "LFO " + juce::String (slot + 1), en);
}

void LFOStrip::resized()
{
    auto area = getLocalBounds().reduced (6, 4);

    auto header = area.removeFromTop (26);
    enableBtn  .setBounds (header.removeFromLeft (44));
    nameLabel  .setBounds (header.removeFromLeft (66));
    shapeLabel .setBounds (header.removeFromLeft (40));
    shapeBox   .setBounds (header.removeFromLeft (90));
    targetLabel.setBounds (header.removeFromLeft (44));
    targetBox  .setBounds (header.removeFromLeft (100));
    tgtSlotLabel.setBounds (header.removeFromLeft (34));
    tgtSlotBox  .setBounds (header.removeFromLeft (60));

    area.removeFromTop (4);
    const int knobW = area.getWidth() / 2;
    rateKnob .setBounds (area.removeFromLeft (knobW).reduced (8, 2));
    depthKnob.setBounds (area.removeFromLeft (knobW).reduced (8, 2));
}

//==============================================================================
// SampleStrip
//==============================================================================
SampleStrip::SampleStrip (ViolentAudioProcessor& p, int slotIndex)
    : processor (p), slot (slotIndex)
{
    nameLabel.setText ("SAMPLE " + juce::String (slot + 1), juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (nameLabel);

    enableBtn.setButtonText ("ON");
    enableBtn.setClickingTogglesState (true);
    addAndMakeVisible (enableBtn);
    enableAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, ParamIDs::smpEn (slot), enableBtn);

    loadBtn.onClick = [this] { openFileChooser(); };
    addAndMakeVisible (loadBtn);

    fileLabel.setText ("(no file)", juce::dontSendNotification);
    fileLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f)));
    fileLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    fileLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (fileLabel);

    loopBtn.setButtonText ("Loop");
    loopBtn.setClickingTogglesState (true);
    addAndMakeVisible (loopBtn);
    loopAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, ParamIDs::smpLoop (slot), loopBtn);

    rootLabel.setText ("Root", juce::dontSendNotification);
    rootLabel.setJustificationType (juce::Justification::centredRight);
    rootLabel.setColour (juce::Label::textColourId, ViolentColours::yellow);
    addAndMakeVisible (rootLabel);

    rootSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    rootSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 36, 18);
    rootSlider.setColour (juce::Slider::trackColourId,            ViolentColours::yellow.withAlpha (0.4f));
    rootSlider.setColour (juce::Slider::thumbColourId,            ViolentColours::yellow);
    addAndMakeVisible (rootSlider);

    for (auto* k : { &gainKnob, &attackKnob, &decayKnob, &sustainKnob, &releaseKnob })
        addAndMakeVisible (*k);

    rootAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::smpRoot (slot), rootSlider);
    gainAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::smpGain (slot), gainKnob.slider);
    attackAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::smpAtt  (slot), attackKnob.slider);
    decayAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::smpDec  (slot), decayKnob.slider);
    sustainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::smpSus  (slot), sustainKnob.slider);
    releaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::smpRel  (slot), releaseKnob.slider);

    updateFileLabel();
}

SampleStrip::~SampleStrip() {}

void SampleStrip::updateFileLabel()
{
    const auto& sm = processor.sampleModules[slot];
    juce::SpinLock::ScopedTryLockType tryLock (
        const_cast<juce::SpinLock&> (sm.lock));
    if (tryLock.isLocked() && sm.hasData)
        fileLabel.setText (juce::File (sm.filePath).getFileName(), juce::dontSendNotification);
    else
        fileLabel.setText ("(no file)", juce::dontSendNotification);
}

void SampleStrip::openFileChooser()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Select a sample", juce::File{},
        processor.formatManager.getWildcardForAllFormats(),
        true);

    fileChooser->launchAsync (juce::FileBrowserComponent::openMode |
                               juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto result = fc.getResult();
            if (result.existsAsFile())
            {
                processor.loadSample (slot, result);
                // Enable the slot automatically when a sample is loaded
                if (auto* param = processor.apvts.getParameter (ParamIDs::smpEn (slot)))
                    param->setValueNotifyingHost (1.0f);
                updateFileLabel();
            }
        });
}

void SampleStrip::paint (juce::Graphics& g)
{
    const bool en = processor.apvts.getRawParameterValue (ParamIDs::smpEn (slot))->load() > 0.5f;
    paintStripHeader (g, *this, "SAMPLE " + juce::String (slot + 1), en);
}

void SampleStrip::resized()
{
    auto area = getLocalBounds().reduced (6, 4);

    // Header row
    auto header = area.removeFromTop (26);
    enableBtn.setBounds (header.removeFromLeft (44));
    nameLabel.setBounds (header.removeFromLeft (90));
    loadBtn  .setBounds (header.removeFromLeft (70));
    fileLabel.setBounds (header.removeFromLeft (200));
    loopBtn  .setBounds (header.removeFromRight (60));

    // Root note bar
    area.removeFromTop (2);
    auto rootRow = area.removeFromTop (24);
    rootLabel .setBounds (rootRow.removeFromLeft (40));
    rootSlider.setBounds (rootRow.removeFromLeft (220));

    // ADSR + gain row
    area.removeFromTop (2);
    const int knobW = area.getWidth() / 5;
    gainKnob   .setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    attackKnob .setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    decayKnob  .setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    sustainKnob.setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    releaseKnob.setBounds (area.removeFromLeft (knobW).reduced (4, 2));
}

//==============================================================================
// SynthTabPanel
//==============================================================================
SynthTabPanel::SynthTabPanel (ViolentAudioProcessor& p)
    : processor (p),
      strips { SynthStrip(p,0), SynthStrip(p,1), SynthStrip(p,2), SynthStrip(p,3),
               SynthStrip(p,4), SynthStrip(p,5), SynthStrip(p,6), SynthStrip(p,7),
               SynthStrip(p,8), SynthStrip(p,9), SynthStrip(p,10),SynthStrip(p,11),
               SynthStrip(p,12),SynthStrip(p,13),SynthStrip(p,14),SynthStrip(p,15) }
{
    for (auto& s : strips) addAndMakeVisible (s);

    addAndMakeVisible (addBtn);
    addBtn.onClick = [this]
    {
        if (processor.numActiveSynths < MAX_SYNTHS)
        {
            ++processor.numActiveSynths;
            if (onCountChanged) onCountChanged();
        }
    };
}

void SynthTabPanel::resized()
{
    const int n    = processor.numActiveSynths;
    auto      area = getLocalBounds().reduced (8, 4);

    for (int i = 0; i < MAX_SYNTHS; ++i)
    {
        strips[i].setVisible (i < n);
        if (i < n)
        {
            strips[i].setBounds (area.removeFromTop (STRIP_H));
            strips[i].onRemove = [this, i]
            {
                if (processor.numActiveSynths <= 1) return;
                --processor.numActiveSynths;
                if (onCountChanged) onCountChanged();
            };
        }
    }

    if (n < MAX_SYNTHS)
        addBtn.setBounds (area.removeFromTop (BUTTON_H).reduced (8, 4));

    addBtn.setVisible (n < MAX_SYNTHS);
}

//==============================================================================
// FxCard
//==============================================================================
FxCard::FxCard (ViolentAudioProcessor& p, int slotIndex)
    : processor (p), slot (slotIndex)
{
    titleLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
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

    driveAtt    = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxDrive   (slot), driveKnob.slider);
    toneAtt     = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxTone    (slot), toneKnob.slider);
    levelAtt    = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxLevel   (slot), levelKnob.slider);
    distTypeAtt = std::make_unique<ComboAtt>  (processor.apvts, ParamIDs::fxDistType(slot), distTypeBox);
    threshAtt   = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxThresh  (slot), threshKnob.slider);
    ratioAtt    = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxRatio   (slot), ratioKnob.slider);
    attackAtt   = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxAttack  (slot), attackKnob.slider);
    releaseAtt  = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxRelease (slot), releaseKnob.slider);
    makeupAtt   = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxMakeup  (slot), makeupKnob.slider);
    roomAtt     = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxRoom    (slot), roomKnob.slider);
    dampingAtt  = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxDamping (slot), dampingKnob.slider);
    wetAtt      = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxWet     (slot), wetKnob.slider);
    widthAtt    = std::make_unique<SliderAtt> (processor.apvts, ParamIDs::fxWidth   (slot), widthKnob.slider);

    showForType (processor.fxChain[(size_t) slot]);
}

FxCard::~FxCard() {}

void FxCard::setAllInvisible()
{
    for (auto* k : { &driveKnob, &toneKnob, &levelKnob,
                     &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob,
                     &roomKnob, &dampingKnob, &wetKnob, &widthKnob })
        k->setVisible (false);
    distTypeBox.setVisible (false);
}

void FxCard::showForType (FxType t)
{
    setAllInvisible();
    titleLabel.setText (fxTypeName (t), juce::dontSendNotification);

    switch (t)
    {
        case FxType::Distortion:
            driveKnob.setVisible (true); toneKnob.setVisible (true);
            levelKnob.setVisible (true); distTypeBox.setVisible (true);
            break;
        case FxType::Compressor:
            threshKnob.setVisible (true); ratioKnob.setVisible (true);
            attackKnob.setVisible (true); releaseKnob.setVisible (true);
            makeupKnob.setVisible (true);
            break;
        case FxType::Gate:
            threshKnob.setVisible (true); ratioKnob.setVisible (true);
            attackKnob.setVisible (true); releaseKnob.setVisible (true);
            break;
        case FxType::Reverb:
            roomKnob.setVisible (true); dampingKnob.setVisible (true);
            wetKnob.setVisible (true);  widthKnob.setVisible (true);
            break;
        default: break;
    }
}

void FxCard::layoutKnobs (std::initializer_list<LabelledKnob*> knobs, juce::Rectangle<int> area)
{
    const int n = (int) knobs.size();
    if (n == 0) return;
    const int w = area.getWidth() / n;
    for (auto* k : knobs)
    {
        k->setBounds (area.removeFromLeft (w).reduced (4, 2));
    }
}

void FxCard::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);
    g.setColour (ViolentColours::overlay);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 6.0f);
    g.setColour (ViolentColours::accent);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 6.0f, 1.0f);
}

void FxCard::resized()
{
    auto area = getLocalBounds().reduced (6, 4);
    auto header = area.removeFromTop (26);
    removeBtn .setBounds (header.removeFromLeft (28));
    titleLabel.setBounds (header);
    area.removeFromTop (4);

    const FxType t = processor.fxChain[(size_t) slot];
    switch (t)
    {
        case FxType::Distortion:
        {
            const int typeW = 90;
            auto typeArea = area.removeFromRight (typeW);
            distTypeBox.setBounds (typeArea.reduced (4, 4));
            layoutKnobs ({ &driveKnob, &toneKnob, &levelKnob }, area);
            break;
        }
        case FxType::Compressor:
            layoutKnobs ({ &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob }, area);
            break;
        case FxType::Gate:
            layoutKnobs ({ &threshKnob, &ratioKnob, &attackKnob, &releaseKnob }, area);
            break;
        case FxType::Reverb:
            layoutKnobs ({ &roomKnob, &dampingKnob, &wetKnob, &widthKnob }, area);
            break;
        default: break;
    }
}

//==============================================================================
// FxTabPanel
//==============================================================================
FxTabPanel::FxTabPanel (ViolentAudioProcessor& p)
    : processor (p),
      cards { FxCard(p,0), FxCard(p,1), FxCard(p,2), FxCard(p,3),
              FxCard(p,4), FxCard(p,5), FxCard(p,6), FxCard(p,7) }
{
    for (auto& c : cards) addChildComponent (c);

    addAndMakeVisible (addBtn);
    addBtn.onClick = [this]
    {
        if (processor.numActiveFx >= MAX_FX) return;

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
                                         FxType::Gate,       FxType::Reverb };
                const int idx = processor.numActiveFx;
                processor.fxChain[(size_t) idx] = types[result - 1];
                cards[(size_t) idx].showForType (types[result - 1]);
                ++processor.numActiveFx;
                if (onCountChanged) onCountChanged();
            });
    };
}

void FxTabPanel::resized()
{
    const int n    = processor.numActiveFx;
    auto      area = getLocalBounds().reduced (8, 4);

    for (int i = 0; i < MAX_FX; ++i)
    {
        cards[(size_t) i].setVisible (i < n);
        if (i < n)
        {
            cards[(size_t) i].setBounds (area.removeFromTop (FxCard::CARD_H));
            area.removeFromTop (4);
            cards[(size_t) i].onRemove = [this, i]
            {
                if (processor.numActiveFx <= 0) return;
                // Shift slots down
                for (int j = i; j < processor.numActiveFx - 1; ++j)
                {
                    processor.fxChain[(size_t) j] = processor.fxChain[(size_t) (j + 1)];
                    cards[(size_t) j].showForType (processor.fxChain[(size_t) j]);
                }
                processor.fxChain[(size_t) (processor.numActiveFx - 1)] = FxType::None;
                --processor.numActiveFx;
                if (onCountChanged) onCountChanged();
            };
        }
    }

    if (n < MAX_FX)
        addBtn.setBounds (area.removeFromTop (36).reduced (8, 4));
    addBtn.setVisible (n < MAX_FX);
}

//==============================================================================
// ViolentAudioProcessorEditor
//==============================================================================
ViolentAudioProcessorEditor::ViolentAudioProcessorEditor (ViolentAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      synthPanel   (p),
      samplePanel  (p),
      filterPanel  (p),
      modPanel     (p),
      fxPanel      (p)
{
    setLookAndFeel (&laf);
    setResizable (false, true);
    setResizeLimits (920, 200, 920, 16 + 52 + MAX_SYNTHS * SynthTabPanel::STRIP_H + SynthTabPanel::BUTTON_H);
    setSize (920, editorHeight());

    synthPanel.onCountChanged = [this] { setSize (getWidth(), editorHeight()); };
    fxPanel.onCountChanged    = [this] { setSize (getWidth(), editorHeight()); };

    for (auto* btn : { &tabSynth, &tabSamples, &tabFilters, &tabMod, &tabFx })
    {
        btn->setClickingTogglesState (false);
        btn->setRadioGroupId (1);
        addAndMakeVisible (*btn);
    }

    tabSynth  .onClick = [this] { showTab (0); };
    tabSamples.onClick = [this] { showTab (1); };
    tabFilters.onClick = [this] { showTab (2); };
    tabMod    .onClick = [this] { showTab (3); };
    tabFx     .onClick = [this] { showTab (4); };

    addAndMakeVisible (synthPanel);
    addAndMakeVisible (samplePanel);
    addAndMakeVisible (filterPanel);
    addAndMakeVisible (modPanel);
    addAndMakeVisible (fxPanel);
    addAndMakeVisible (meter);

    showTab (0);
}

ViolentAudioProcessorEditor::~ViolentAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void ViolentAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);

    // Header bar
    g.setColour (ViolentColours::surface);
    g.fillRect (0, 0, getWidth(), 50);

    // Plugin name
    g.setColour (ViolentColours::accent);
    g.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (22.0f).withStyle ("Bold")));
    g.drawText ("VIOLENT", 16, 0, 120, 50, juce::Justification::centredLeft);

    // Separator
    g.setColour (ViolentColours::overlay);
    g.fillRect (0, 50, getWidth(), 2);
}

int ViolentAudioProcessorEditor::editorHeight() const noexcept
{
    if (currentTab == 0)
    {
        const int n = processor.numActiveSynths;
        const bool hasAdd = (n < MAX_SYNTHS);
        return 52 + n * SynthTabPanel::STRIP_H + (hasAdd ? SynthTabPanel::BUTTON_H : 0) + 16;
    }
    if (currentTab == 4)
    {
        const int n = processor.numActiveFx;
        const bool hasAdd = (n < MAX_FX);
        return 52 + n * (FxCard::CARD_H + 4) + (hasAdd ? 44 : 0) + 16;
    }
    return 560;
}

void ViolentAudioProcessorEditor::resized()
{
    const int tabW   = 90;
    const int startX = 150;

    tabSynth  .setBounds (startX + 0 * tabW, 10, tabW - 4, 32);
    tabSamples.setBounds (startX + 1 * tabW, 10, tabW - 4, 32);
    tabFilters.setBounds (startX + 2 * tabW, 10, tabW - 4, 32);
    tabMod    .setBounds (startX + 3 * tabW, 10, tabW - 4, 32);
    tabFx     .setBounds (startX + 4 * tabW, 10, tabW - 4, 32);

    meter.setBounds (getWidth() - 44, 8, 36, 34);
    meter.setLevels (processor.levelL.load (std::memory_order_relaxed),
                     processor.levelR.load (std::memory_order_relaxed));

    const auto content = juce::Rectangle<int> (0, 52, getWidth(), getHeight() - 52);
    synthPanel .setBounds (content);
    samplePanel.setBounds (content);
    filterPanel.setBounds (content);
    modPanel   .setBounds (content);
    fxPanel    .setBounds (content);
}

void ViolentAudioProcessorEditor::showTab (int tabIndex)
{
    currentTab = tabIndex;
    synthPanel .setVisible (tabIndex == 0);
    samplePanel.setVisible (tabIndex == 1);
    filterPanel.setVisible (tabIndex == 2);
    modPanel   .setVisible (tabIndex == 3);
    fxPanel    .setVisible (tabIndex == 4);

    juce::TextButton* tabs[] = { &tabSynth, &tabSamples, &tabFilters, &tabMod, &tabFx };
    for (int i = 0; i < 5; ++i)
        tabs[i]->setToggleState (i == tabIndex, juce::dontSendNotification);

    setSize (getWidth(), editorHeight());
}

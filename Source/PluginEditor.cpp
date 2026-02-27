#include "PluginEditor.h"

//==============================================================================
ViolentLookAndFeel::ViolentLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId,   Colours::background);
    setColour (juce::Label::textColourId,                   Colours::text);
    setColour (juce::Slider::thumbColourId,                 Colours::accent);
    setColour (juce::Slider::trackColourId,                 Colours::surface);
    setColour (juce::Slider::backgroundColourId,            Colours::surface);
    setColour (juce::ToggleButton::textColourId,            Colours::text);
    setColour (juce::TextButton::buttonColourId,            Colours::surface);
    setColour (juce::TextButton::buttonOnColourId,          Colours::accent);
    setColour (juce::TextButton::textColourOffId,           Colours::subtext);
    setColour (juce::TextButton::textColourOnId,            Colours::background);
    setColour (juce::ComboBox::backgroundColourId,          Colours::surface);
    setColour (juce::ComboBox::textColourId,                Colours::text);
    setColour (juce::ComboBox::outlineColourId,             Colours::overlay);
    setColour (juce::PopupMenu::backgroundColourId,         Colours::surface);
    setColour (juce::PopupMenu::textColourId,               Colours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::accent);
    setColour (juce::PopupMenu::highlightedTextColourId,    Colours::background);
}

void ViolentLookAndFeel::drawRotarySlider (juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    const float radius = static_cast<float> (juce::jmin (width, height)) * 0.5f - 4.0f;
    const float centreX = static_cast<float> (x) + static_cast<float> (width)  * 0.5f;
    const float centreY = static_cast<float> (y) + static_cast<float> (height) * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background circle
    g.setColour (Colours::overlay);
    g.fillEllipse (rx, ry, rw, rw);

    // Arc track
    juce::Path trackArc;
    trackArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                             0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (Colours::surface);
    g.strokePath (trackArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

    // Value arc - get accent colour from slider's custom property or use default
    juce::Colour arcColour = slider.findColour (juce::Slider::rotarySliderFillColourId,
                                                 false);
    if (\!arcColour.isOpaque())
        arcColour = Colours::accent;

    juce::Path valueArc;
    valueArc.addCentredArc (centreX, centreY, radius - 3.0f, radius - 3.0f,
                             0.0f, rotaryStartAngle, angle, true);
    g.setColour (arcColour);
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

    // Thumb dot
    const float thumbRadius = 4.0f;
    const float thumbX = centreX + (radius - 8.0f) * std::cos (angle - juce::MathConstants<float>::halfPi);
    const float thumbY = centreY + (radius - 8.0f) * std::sin (angle - juce::MathConstants<float>::halfPi);
    g.setColour (Colours::text);
    g.fillEllipse (thumbX - thumbRadius, thumbY - thumbRadius,
                   thumbRadius * 2.0f, thumbRadius * 2.0f);
}

void ViolentLookAndFeel::drawLinearSlider (juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
    juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical)
    {
        const float trackW = 6.0f;
        const float centreX = static_cast<float> (x) + static_cast<float> (width) * 0.5f;
        const float trackTop    = static_cast<float> (y) + 8.0f;
        const float trackBottom = static_cast<float> (y + height) - 8.0f;
        const float trackLength = trackBottom - trackTop;

        // Track background
        g.setColour (Colours::overlay);
        g.fillRoundedRectangle (centreX - trackW * 0.5f, trackTop, trackW, trackLength, 3.0f);

        // Centre line (0 dB reference)
        const float midY = trackTop + trackLength * 0.5f;
        g.setColour (Colours::subtext);
        g.drawHorizontalLine (static_cast<int> (midY), centreX - 8.0f, centreX + 8.0f);

        // Value fill
        const float thumbY = sliderPos;
        juce::Colour fillColour = slider.findColour (juce::Slider::rotarySliderFillColourId, false);
        if (\!fillColour.isOpaque()) fillColour = Colours::accent;

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

        // Thumb
        const float thumbH = 12.0f;
        const float thumbW = 22.0f;
        g.setColour (Colours::text);
        g.fillRoundedRectangle (centreX - thumbW * 0.5f, thumbY - thumbH * 0.5f,
                                thumbW, thumbH, 3.0f);
    }
}

void ViolentLookAndFeel::drawToggleButton (juce::Graphics& g,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool /*shouldDrawButtonAsDown*/)
{
    const bool isOn = button.getToggleState();
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);

    // Pill background
    g.setColour (isOn ? Colours::accent : Colours::overlay);
    g.fillRoundedRectangle (bounds, bounds.getHeight() * 0.5f);

    // Border
    g.setColour (isOn ? Colours::accent.brighter (0.2f) : Colours::subtext);
    g.drawRoundedRectangle (bounds, bounds.getHeight() * 0.5f, 1.5f);

    // Text
    g.setColour (isOn ? Colours::background : Colours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (13.0f).withStyle ("Bold")));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds(),
                      juce::Justification::centred, 1);
}

void ViolentLookAndFeel::drawButtonBackground (juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& /*backgroundColour*/,
    bool shouldDrawButtonAsHighlighted,
    bool /*shouldDrawButtonAsDown*/)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool isOn   = button.getToggleState();

    g.setColour (isOn ? Colours::accent : (shouldDrawButtonAsHighlighted
                                           ? Colours::overlay : Colours::surface));
    g.fillRoundedRectangle (bounds, 6.0f);

    g.setColour (isOn ? Colours::accent.brighter (0.2f) : Colours::overlay);
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);
}

juce::Font ViolentLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions().withHeight (12.0f));
}

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
    nameLabel.setColour (juce::Label::textColourId, Colours::subtext);
    addAndMakeVisible (nameLabel);

    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setColour (juce::Label::textColourId, Colours::text);
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
    nameLabel .setBounds (area.removeFromTop (18));
    valueLabel.setBounds (area.removeFromBottom (16));
    slider    .setBounds (area);
}

//==============================================================================
// EQPanel
//==============================================================================
EQPanel::EQPanel (ViolentAudioProcessor& p) : processor (p)
{
    const char* bandIDs[] = {
        "eq_band_0", "eq_band_1", "eq_band_2", "eq_band_3", "eq_band_4",
        "eq_band_5", "eq_band_6", "eq_band_7", "eq_band_8", "eq_band_9"
    };
    const char* freqNames[] = {
        "31", "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k"
    };

    for (int i = 0; i < NUM_BANDS; ++i)
    {
        auto& s = bandSliders[i];
        s.setSliderStyle (juce::Slider::LinearVertical);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setColour (juce::Slider::rotarySliderFillColourId, Colours::accent);
        addAndMakeVisible (s);

        auto& l = freqLabels[i];
        l.setText (freqNames[i], juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, Colours::subtext);
        addAndMakeVisible (l);

        sliderAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, bandIDs[i], s);
    }

    enableButton.setClickingTogglesState (true);
    addAndMakeVisible (enableButton);
    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, "eq_enabled", enableButton);
}

EQPanel::~EQPanel() {}

void EQPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);

    // dB guide lines
    g.setColour (Colours::overlay);
    const auto area   = getLocalBounds().reduced (20, 60).toFloat();
    const float midY  = area.getY() + area.getHeight() * 0.5f;
    g.drawHorizontalLine (static_cast<int> (midY),
                          area.getX(), area.getRight());
}

void EQPanel::resized()
{
    auto area = getLocalBounds().reduced (16);

    // Enable button top right
    enableButton.setBounds (area.getWidth() - 80, 4, 80, 28);

    // Band sliders
    const int labelH = 20;
    const int sliderArea = area.getHeight() - labelH - 20;
    const int bandW = area.getWidth() / NUM_BANDS;

    for (int i = 0; i < NUM_BANDS; ++i)
    {
        const int bx = area.getX() + i * bandW;
        bandSliders[i].setBounds (bx, area.getY() + 20,       bandW, sliderArea);
        freqLabels[i] .setBounds (bx, area.getBottom() - labelH, bandW, labelH);
    }
}

//==============================================================================
// ReverbPanel
//==============================================================================
ReverbPanel::ReverbPanel (ViolentAudioProcessor& p) : processor (p)
{
    enableButton.setClickingTogglesState (true);
    addAndMakeVisible (enableButton);
    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, "reverb_enabled", enableButton);

    for (auto* k : { &roomKnob, &dampingKnob, &wetKnob, &widthKnob })
        addAndMakeVisible (*k);

    roomAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "reverb_room",    roomKnob.slider);
    dampingAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "reverb_damping", dampingKnob.slider);
    wetAtt     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "reverb_wet",     wetKnob.slider);
    widthAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "reverb_width",   widthKnob.slider);
}

ReverbPanel::~ReverbPanel() {}

void ReverbPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);
    g.setColour (Colours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (32.0f).withStyle ("Bold")));
    g.drawText ("REVERB", getLocalBounds().removeFromTop (60), juce::Justification::centred);
}

void ReverbPanel::resized()
{
    auto area = getLocalBounds().reduced (20);
    enableButton.setBounds (area.getWidth() - 100, 4, 100, 28);

    auto knobArea = area.withTrimmedTop (60);
    const int knobW = knobArea.getWidth() / 4;
    roomKnob   .setBounds (knobArea.removeFromLeft (knobW).reduced (10));
    dampingKnob.setBounds (knobArea.removeFromLeft (knobW).reduced (10));
    wetKnob    .setBounds (knobArea.removeFromLeft (knobW).reduced (10));
    widthKnob  .setBounds (knobArea.removeFromLeft (knobW).reduced (10));
}

//==============================================================================
// DistortionPanel
//==============================================================================
DistortionPanel::DistortionPanel (ViolentAudioProcessor& p) : processor (p)
{
    enableButton.setClickingTogglesState (true);
    addAndMakeVisible (enableButton);
    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, "dist_enabled", enableButton);

    for (auto* k : { &driveKnob, &toneKnob, &levelKnob })
        addAndMakeVisible (*k);

    driveAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "dist_drive", driveKnob.slider);
    toneAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "dist_tone",  toneKnob.slider);
    levelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "dist_level", levelKnob.slider);

    typeBox.addItem ("Soft Clip", 1);
    typeBox.addItem ("Hard Clip", 2);
    typeBox.addItem ("Fuzz",      3);
    addAndMakeVisible (typeBox);

    typeLabel.setText ("Type", juce::dontSendNotification);
    typeLabel.setJustificationType (juce::Justification::centred);
    typeLabel.setColour (juce::Label::textColourId, Colours::subtext);
    addAndMakeVisible (typeLabel);

    typeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, "dist_type", typeBox);
}

DistortionPanel::~DistortionPanel() {}

void DistortionPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);
    g.setColour (Colours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (32.0f).withStyle ("Bold")));
    g.drawText ("DISTORTION", getLocalBounds().removeFromTop (60), juce::Justification::centred);
}

void DistortionPanel::resized()
{
    auto area = getLocalBounds().reduced (20);
    enableButton.setBounds (area.getWidth() - 120, 4, 120, 28);

    auto knobArea = area.withTrimmedTop (60);
    const int knobW    = knobArea.getWidth() / 4;
    const int typeBoxH = 30;

    driveKnob.setBounds (knobArea.removeFromLeft (knobW).reduced (10));
    toneKnob .setBounds (knobArea.removeFromLeft (knobW).reduced (10));
    levelKnob.setBounds (knobArea.removeFromLeft (knobW).reduced (10));

    auto typeArea = knobArea.removeFromLeft (knobW).reduced (10);
    typeLabel.setBounds (typeArea.removeFromTop (18));
    typeBox  .setBounds (typeArea.removeFromTop (typeBoxH));
}

//==============================================================================
// CompressorPanel
//==============================================================================
CompressorPanel::CompressorPanel (ViolentAudioProcessor& p) : processor (p)
{
    enableButton.setClickingTogglesState (true);
    addAndMakeVisible (enableButton);
    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, "comp_enabled", enableButton);

    for (auto* k : { &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob })
        addAndMakeVisible (*k);

    threshAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "comp_threshold", threshKnob.slider);
    ratioAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "comp_ratio",     ratioKnob.slider);
    attackAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "comp_attack",    attackKnob.slider);
    releaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "comp_release",   releaseKnob.slider);
    makeupAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "comp_makeup",    makeupKnob.slider);
}

CompressorPanel::~CompressorPanel() {}

void CompressorPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);
    g.setColour (Colours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (32.0f).withStyle ("Bold")));
    g.drawText ("COMPRESSOR", getLocalBounds().removeFromTop (60), juce::Justification::centred);
}

void CompressorPanel::resized()
{
    auto area = getLocalBounds().reduced (20);
    enableButton.setBounds (area.getWidth() - 130, 4, 130, 28);

    auto knobArea = area.withTrimmedTop (60);
    const int knobW = knobArea.getWidth() / 5;
    threshKnob .setBounds (knobArea.removeFromLeft (knobW).reduced (8));
    ratioKnob  .setBounds (knobArea.removeFromLeft (knobW).reduced (8));
    attackKnob .setBounds (knobArea.removeFromLeft (knobW).reduced (8));
    releaseKnob.setBounds (knobArea.removeFromLeft (knobW).reduced (8));
    makeupKnob .setBounds (knobArea.removeFromLeft (knobW).reduced (8));
}

//==============================================================================
// GatePanel
//==============================================================================
GatePanel::GatePanel (ViolentAudioProcessor& p) : processor (p)
{
    enableButton.setClickingTogglesState (true);
    addAndMakeVisible (enableButton);
    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, "gate_enabled", enableButton);

    for (auto* k : { &threshKnob, &attackKnob, &releaseKnob, &ratioKnob })
        addAndMakeVisible (*k);

    threshAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "gate_threshold", threshKnob.slider);
    attackAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "gate_attack",    attackKnob.slider);
    releaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "gate_release",   releaseKnob.slider);
    ratioAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, "gate_ratio",     ratioKnob.slider);
}

GatePanel::~GatePanel() {}

void GatePanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);
    g.setColour (Colours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (32.0f).withStyle ("Bold")));
    g.drawText ("GATE", getLocalBounds().removeFromTop (60), juce::Justification::centred);
}

void GatePanel::resized()
{
    auto area = getLocalBounds().reduced (20);
    enableButton.setBounds (area.getWidth() - 90, 4, 90, 28);

    auto knobArea = area.withTrimmedTop (60);
    const int knobW = knobArea.getWidth() / 4;
    threshKnob .setBounds (knobArea.removeFromLeft (knobW).reduced (10));
    attackKnob .setBounds (knobArea.removeFromLeft (knobW).reduced (10));
    releaseKnob.setBounds (knobArea.removeFromLeft (knobW).reduced (10));
    ratioKnob  .setBounds (knobArea.removeFromLeft (knobW).reduced (10));
}

//==============================================================================
// ViolentAudioProcessorEditor
//==============================================================================
ViolentAudioProcessorEditor::ViolentAudioProcessorEditor (ViolentAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      eqPanel   (p),
      reverbPanel (p),
      distPanel (p),
      compPanel (p),
      gatePanel (p)
{
    setLookAndFeel (&laf);
    setSize (800, 500);

    // Setup tab buttons
    for (auto* btn : { &tabEQ, &tabReverb, &tabDistortion, &tabCompressor, &tabGate })
    {
        btn->setClickingTogglesState (false);
        btn->setRadioGroupId (1);
        addAndMakeVisible (*btn);
    }

    tabEQ        .onClick = [this] { showTab (0); };
    tabReverb    .onClick = [this] { showTab (1); };
    tabDistortion.onClick = [this] { showTab (2); };
    tabCompressor.onClick = [this] { showTab (3); };
    tabGate      .onClick = [this] { showTab (4); };

    addAndMakeVisible (eqPanel);
    addAndMakeVisible (reverbPanel);
    addAndMakeVisible (distPanel);
    addAndMakeVisible (compPanel);
    addAndMakeVisible (gatePanel);

    showTab (0);
}

ViolentAudioProcessorEditor::~ViolentAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void ViolentAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);

    // Header bar
    g.setColour (Colours::surface);
    g.fillRect (0, 0, getWidth(), 50);

    // Plugin name
    g.setColour (Colours::accent);
    g.setFont (juce::Font (juce::FontOptions().withHeight (24.0f).withStyle ("Bold")));
    g.drawText ("VIOLENT", 16, 0, 120, 50, juce::Justification::centredLeft);

    // Tab separator
    g.setColour (Colours::overlay);
    g.fillRect (0, 50, getWidth(), 2);
}

void ViolentAudioProcessorEditor::resized()
{
    const int headerH  = 50;
    const int tabBarH  = 40;
    const int numTabs  = 5;
    const int tabW     = 100;
    const int tabStartX = 140;

    tabEQ        .setBounds (tabStartX + 0 * tabW, 5,        tabW - 4, tabBarH - 8);
    tabReverb    .setBounds (tabStartX + 1 * tabW, 5,        tabW - 4, tabBarH - 8);
    tabDistortion.setBounds (tabStartX + 2 * tabW, 5,        tabW - 4, tabBarH - 8);
    tabCompressor.setBounds (tabStartX + 3 * tabW, 5,        tabW - 4, tabBarH - 8);
    tabGate      .setBounds (tabStartX + 4 * tabW, 5,        tabW - 4, tabBarH - 8);

    const auto contentBounds = juce::Rectangle<int> (0, headerH + 2, getWidth(), getHeight() - headerH - 2);
    eqPanel   .setBounds (contentBounds);
    reverbPanel.setBounds (contentBounds);
    distPanel .setBounds (contentBounds);
    compPanel .setBounds (contentBounds);
    gatePanel .setBounds (contentBounds);
}

void ViolentAudioProcessorEditor::showTab (int tabIndex)
{
    currentTab = tabIndex;

    eqPanel   .setVisible (tabIndex == 0);
    reverbPanel.setVisible (tabIndex == 1);
    distPanel .setVisible (tabIndex == 2);
    compPanel .setVisible (tabIndex == 3);
    gatePanel .setVisible (tabIndex == 4);

    // Highlight active tab button
    juce::TextButton* tabs[] = { &tabEQ, &tabReverb, &tabDistortion, &tabCompressor, &tabGate };
    for (int i = 0; i < 5; ++i)
        tabs[i]->setToggleState (i == tabIndex, juce::dontSendNotification);
}

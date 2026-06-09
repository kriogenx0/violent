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
    g.setFont (juce::Font (juce::FontOptions().withHeight (12.0f).withStyle ("Bold")));
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
    return juce::Font (juce::FontOptions().withHeight (12.0f));
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
    nameLabel.setText ("SYNTH " + juce::String (slot + 1), juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (nameLabel);

    enableBtn.setButtonText ("ON");
    enableBtn.setClickingTogglesState (true);
    addAndMakeVisible (enableBtn);
    enableAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, ParamIDs::synthEn (slot), enableBtn);

    waveLabel.setText ("Wave", juce::dontSendNotification);
    waveLabel.setJustificationType (juce::Justification::centred);
    waveLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (waveLabel);

    for (const auto& w : { "Sine", "Saw", "Square", "Tri" })
        waveBox.addItem (w, waveBox.getNumItems() + 1);
    addAndMakeVisible (waveBox);
    waveAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, ParamIDs::synthType (slot), waveBox);

    for (auto* k : { &gainKnob, &detuneKnob, &attackKnob, &decayKnob, &sustainKnob, &releaseKnob })
        addAndMakeVisible (*k);

    gainAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthGain (slot), gainKnob.slider);
    detuneAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthDet  (slot), detuneKnob.slider);
    attackAtt  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthAtt  (slot), attackKnob.slider);
    decayAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthDec  (slot), decayKnob.slider);
    sustainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthSus  (slot), sustainKnob.slider);
    releaseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, ParamIDs::synthRel  (slot), releaseKnob.slider);
}

SynthStrip::~SynthStrip() {}

void SynthStrip::paint (juce::Graphics& g)
{
    const bool en = processor.apvts.getRawParameterValue (ParamIDs::synthEn (slot))->load() > 0.5f;
    paintStripHeader (g, *this, "SYNTH " + juce::String (slot + 1), en);
}

void SynthStrip::resized()
{
    auto area = getLocalBounds().reduced (6, 4);

    // Header row
    auto header = area.removeFromTop (26);
    enableBtn .setBounds (header.removeFromLeft  (44));
    nameLabel .setBounds (header.removeFromLeft  (80));
    waveLabel .setBounds (header.removeFromLeft  (36));
    waveBox   .setBounds (header.removeFromLeft  (90));

    // Knobs row
    area.removeFromTop (4);
    const int knobW = area.getWidth() / 6;
    gainKnob   .setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    detuneKnob .setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    attackKnob .setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    decayKnob  .setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    sustainKnob.setBounds (area.removeFromLeft (knobW).reduced (4, 2));
    releaseKnob.setBounds (area.removeFromLeft (knobW).reduced (4, 2));
}

//==============================================================================
// FilterStrip
//==============================================================================
FilterStrip::FilterStrip (ViolentAudioProcessor& p, int slotIndex)
    : processor (p), slot (slotIndex)
{
    nameLabel.setText ("FILTER " + juce::String (slot + 1), juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (13.0f).withStyle ("Bold")));
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
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (13.0f).withStyle ("Bold")));
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
    nameLabel.setFont (juce::Font (juce::FontOptions().withHeight (13.0f).withStyle ("Bold")));
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
    fileLabel.setFont (juce::Font (juce::FontOptions().withHeight (11.0f)));
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
// SampleTabPanel
//==============================================================================
SampleTabPanel::SampleTabPanel (ViolentAudioProcessor& p)
    : strips { SampleStrip (p, 0), SampleStrip (p, 1), SampleStrip (p, 2), SampleStrip (p, 3) }
{
    for (auto& s : strips) addAndMakeVisible (s);
}

void SampleTabPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    const int h = area.getHeight() / MAX_SAMPLES;
    for (auto& s : strips)
        s.setBounds (area.removeFromTop (h));
}

//==============================================================================
// Tab panels
//==============================================================================
SynthTabPanel::SynthTabPanel (ViolentAudioProcessor& p)
    : strips { SynthStrip (p, 0), SynthStrip (p, 1), SynthStrip (p, 2), SynthStrip (p, 3) }
{
    for (auto& s : strips) addAndMakeVisible (s);
}

void SynthTabPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    const int h = area.getHeight() / MAX_SYNTHS;
    for (auto& s : strips)
        s.setBounds (area.removeFromTop (h));
}

FilterTabPanel::FilterTabPanel (ViolentAudioProcessor& p)
    : strips { FilterStrip (p, 0), FilterStrip (p, 1), FilterStrip (p, 2), FilterStrip (p, 3) }
{
    for (auto& s : strips) addAndMakeVisible (s);
}

void FilterTabPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    const int h = area.getHeight() / MAX_FILTERS;
    for (auto& s : strips)
        s.setBounds (area.removeFromTop (h));
}

ModTabPanel::ModTabPanel (ViolentAudioProcessor& p)
    : strips { LFOStrip (p, 0), LFOStrip (p, 1), LFOStrip (p, 2), LFOStrip (p, 3) }
{
    for (auto& s : strips) addAndMakeVisible (s);
}

void ModTabPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    const int h = area.getHeight() / MAX_LFOS;
    for (auto& s : strips)
        s.setBounds (area.removeFromTop (h));
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
        s.setColour (juce::Slider::rotarySliderFillColourId, ViolentColours::accent);
        addAndMakeVisible (s);

        auto& l = freqLabels[i];
        l.setText (freqNames[i], juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, ViolentColours::subtext);
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
    g.fillAll (ViolentColours::background);
    g.setColour (ViolentColours::overlay);
    const auto area  = getLocalBounds().reduced (20, 60).toFloat();
    const float midY = area.getY() + area.getHeight() * 0.5f;
    g.drawHorizontalLine (static_cast<int> (midY), area.getX(), area.getRight());
}

void EQPanel::resized()
{
    auto area = getLocalBounds().reduced (16);
    enableButton.setBounds (area.getWidth() - 80, 4, 80, 28);
    const int labelH    = 20;
    const int sliderArea = area.getHeight() - labelH - 20;
    const int bandW     = area.getWidth() / NUM_BANDS;
    for (int i = 0; i < NUM_BANDS; ++i)
    {
        const int bx = area.getX() + i * bandW;
        bandSliders[i].setBounds (bx, area.getY() + 20,          bandW, sliderArea);
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
    g.fillAll (ViolentColours::background);
    g.setColour (ViolentColours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (28.0f).withStyle ("Bold")));
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
    typeLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (typeLabel);
    typeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processor.apvts, "dist_type", typeBox);
}

DistortionPanel::~DistortionPanel() {}

void DistortionPanel::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);
    g.setColour (ViolentColours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (28.0f).withStyle ("Bold")));
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
    g.fillAll (ViolentColours::background);
    g.setColour (ViolentColours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (28.0f).withStyle ("Bold")));
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
    g.fillAll (ViolentColours::background);
    g.setColour (ViolentColours::text);
    g.setFont (juce::Font (juce::FontOptions().withHeight (28.0f).withStyle ("Bold")));
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
// FxTabPanel
//==============================================================================
FxTabPanel::FxTabPanel (ViolentAudioProcessor& p)
    : processor (p),
      eqPanel (p), distPanel (p), compPanel (p), gatePanel (p), reverbPanel (p)
{
    for (auto* btn : { &tabEQ, &tabDist, &tabComp, &tabGate, &tabVerb })
    {
        btn->setClickingTogglesState (false);
        btn->setRadioGroupId (2);
        addAndMakeVisible (*btn);
    }
    tabEQ  .onClick = [this] { showFxTab (0); };
    tabDist.onClick = [this] { showFxTab (1); };
    tabComp.onClick = [this] { showFxTab (2); };
    tabGate.onClick = [this] { showFxTab (3); };
    tabVerb.onClick = [this] { showFxTab (4); };

    addAndMakeVisible (eqPanel);
    addAndMakeVisible (distPanel);
    addAndMakeVisible (compPanel);
    addAndMakeVisible (gatePanel);
    addAndMakeVisible (reverbPanel);

    showFxTab (0);
}

void FxTabPanel::showFxTab (int idx)
{
    currentFxTab = idx;
    eqPanel    .setVisible (idx == 0);
    distPanel  .setVisible (idx == 1);
    compPanel  .setVisible (idx == 2);
    gatePanel  .setVisible (idx == 3);
    reverbPanel.setVisible (idx == 4);

    juce::TextButton* tabs[] = { &tabEQ, &tabDist, &tabComp, &tabGate, &tabVerb };
    for (int i = 0; i < 5; ++i)
        tabs[i]->setToggleState (i == idx, juce::dontSendNotification);
}

void FxTabPanel::paint (juce::Graphics& g)
{
    g.fillAll (ViolentColours::background);
}

void FxTabPanel::resized()
{
    auto area = getLocalBounds();

    // Sub-tab bar
    auto tabBar = area.removeFromTop (36);
    const int tabW = tabBar.getWidth() / 5;
    tabEQ  .setBounds (tabBar.removeFromLeft (tabW).reduced (2, 4));
    tabDist.setBounds (tabBar.removeFromLeft (tabW).reduced (2, 4));
    tabComp.setBounds (tabBar.removeFromLeft (tabW).reduced (2, 4));
    tabGate.setBounds (tabBar.removeFromLeft (tabW).reduced (2, 4));
    tabVerb.setBounds (tabBar.removeFromLeft (tabW).reduced (2, 4));

    for (auto* panel : { (juce::Component*)&eqPanel, (juce::Component*)&distPanel, (juce::Component*)&compPanel,
                                            (juce::Component*)&gatePanel, (juce::Component*)&reverbPanel })
        panel->setBounds (area);
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
    setSize (920, 560);

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
    g.setFont (juce::Font (juce::FontOptions().withHeight (22.0f).withStyle ("Bold")));
    g.drawText ("VIOLENT", 16, 0, 120, 50, juce::Justification::centredLeft);

    // Separator
    g.setColour (ViolentColours::overlay);
    g.fillRect (0, 50, getWidth(), 2);
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
}

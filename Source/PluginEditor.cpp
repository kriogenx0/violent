#include "PluginEditor.h"

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

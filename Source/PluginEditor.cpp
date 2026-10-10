#include "PluginEditor.h"

//==============================================================================
// ViolentAudioProcessorEditor
//==============================================================================
ViolentAudioProcessorEditor::ViolentAudioProcessorEditor (ViolentAudioProcessor& p)
    : AudioProcessorEditor (p), processor (p), rack (p), minimap (p, rack, rackScaler, rackViewport)
{
    setLookAndFeel (&laf);
    // Width is fixed (only the zoom buttons change it, not dragging or the
    // native maximize button) — see updateWindowSize(), which re-pins the resize
    // limits' width whenever the zoom changes. Height is freely resizable,
    // including via the native macOS maximize button.
    setResizable (true, false);

    rackViewport.setViewedComponent (&rackScaler, false);
    rackViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (rackViewport);

    navViewport.setViewedComponent (&minimap, false);
    navViewport.setScrollBarsShown (true, false);
    addAndMakeVisible (navViewport);

    addAndMakeVisible (meter);

    addAndMakeVisible (midiLight);
    midiLight.setTooltip ("MIDI activity");

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
    previewPatternBox.setScrollWheelEnabled (false);
    previewPatternBox.setSelectedItemIndex (0, juce::dontSendNotification);
    previewPatternBox.onChange = [this]
    {
        processor.setPreviewPattern (previewPatternBox.getSelectedItemIndex());
    };
    addAndMakeVisible (previewPatternBox);

    presetBox.setRepaintsOnMouseActivity (true);
    presetBox.setScrollWheelEnabled (false);
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

    randomizeBtn.onClick = [this]
    {
        processor.randomizeAll();
        rack.refreshFromState();
        minimap.refreshFromState();
        updateLayout();
    };
    addAndMakeVisible (randomizeBtn);

    rack.onLayoutChanged = [this] { minimap.refreshFromState(); updateLayout(); };

    updateWindowSize();
}

ViolentAudioProcessorEditor::~ViolentAudioProcessorEditor()
{
    processor.previewActive.store (false, std::memory_order_relaxed);
    setLookAndFeel (nullptr);
}

void ViolentAudioProcessorEditor::updateLayout()
{
    // The rack scrolls inside rackViewport, so adding/removing/rearranging
    // components only changes how much there is to scroll - the window never
    // needs a different size for it, and must not be asked for one here.
    // Asking for a height the screen can't give a window (see
    // updateWindowSize()) is clamped by the OS without any notification, which
    // is what used to leave everything drawn out of step with where it responds
    // to clicks after adding a component, until the next real resize.
    rackScaler.updateLayout (uiScale);
}

void ViolentAudioProcessorEditor::updateWindowSize()
{
    rackScaler.updateLayout (uiScale);

    const int newW = Minimap::WIDTH + juce::roundToInt (ScalableRackComponent::BASE_WIDTH * uiScale);

    // The window opens filling the screen vertically, then keeps whatever
    // height it's given (free resize/maximize) - so the height is only chosen
    // here the first time through, and never again on later layout changes.
    // That first request is the display's whole usable height, which a window
    // with a title bar can't actually have (the title bar takes part of it), so
    // as the window is shown the OS trims it to fit and reports the trimmed
    // size to JUCE - the frame genuinely changes, so that notification does
    // arrive. Asking for it again once the window is already that tall is
    // different: the OS clamps the request straight back to the current frame
    // and, with no frame change, posts nothing.
    int newH = getHeight();

    if (newH <= 0)
    {
        const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds());
        newH = display != nullptr ? juce::roundToInt (display->userBounds.getHeight()) : HEADER_H + rackScaler.getHeight();
    }

    setResizeLimits (newW, 200, newW, 100000);

    if (newW == getWidth() && newH == getHeight())
        return;

    setSize (newW, newH);

    // A native window may decline a resize it can't fit (e.g. a zoomed width
    // wider than the screen). When the clamped result equals the window's
    // current size there is no frame-change notification, so JUCE would keep
    // the size it asked for while the native view stays smaller. ComponentPeer
    // then scales the painted image down to the native view but not mouse
    // input, so everything drawn drifts away from where it actually responds
    // (by an amount that grows down/across the window) until some other resize
    // happens to re-sync the two. Re-reading the native bounds here adopts
    // whatever size the window really ended up with. Standalone only: plugin
    // hosts own their view's size and may apply a resize asynchronously.
    if (juce::JUCEApplicationBase::isStandaloneApp())
        if (auto* peer = getPeer())
            peer->handleMovedOrResized();
}

void ViolentAudioProcessorEditor::setUiScale (float newScale)
{
    // 0.85 is the lowest scale at which the header toolbar's fixed-width
    // controls (which don't shrink with zoom) still fit the window without
    // the right-anchored preview controls overlapping the left-anchored
    // zoom/randomize buttons.
    newScale = juce::jlimit (0.85f, 1.5f, newScale);
    if (juce::approximatelyEqual (newScale, uiScale)) return;

    uiScale = newScale;
    updateWindowSize();

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
        minimap.refreshFromState();
        updateLayout();
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

    previewBtn.setBounds (getWidth() - 84, 12, 30, 26);
    midiLight.setBounds (getWidth() - 53, 21, 8, 8);
    previewPatternBox.setBounds (getWidth() - 234, 12, 140, 26);

    // Starts at 218 rather than 150 to leave clear room for the standalone
    // window's native-title-bar "Options" button (see Main.cpp), which lives
    // at x=150-210 in this same coordinate space — they used to overlap.
    presetBox.setBounds (218, 12, 180, 26);
    savePresetBtn.setBounds (404, 12, 56, 26);

    zoomOutBtn.setBounds (savePresetBtn.getRight() + 16, 12, 26, 26);
    zoomInBtn.setBounds (zoomOutBtn.getRight() + 4, 12, 26, 26);
    zoomLabel.setBounds (zoomInBtn.getRight() + 8, 12, 60, 26);

    randomizeBtn.setBounds (zoomLabel.getRight() + 8, 12, 90, 26);

    navViewport.setBounds (0, HEADER_H, Minimap::WIDTH, getHeight() - HEADER_H);
    rackViewport.setBounds (Minimap::WIDTH, HEADER_H, getWidth() - Minimap::WIDTH, getHeight() - HEADER_H);
}

#include "ModulatorView.h"

//==============================================================================
// ModulatorRow
//==============================================================================
ModulatorRow::ModulatorRow (ViolentAudioProcessor& p, int modSlot)
    : processor (p), slot (modSlot)
{
    auto& mod = processor.modulators[(size_t) slot];

    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    enableBtn.setClickingTogglesState (true);
    enableBtn.setToggleState (mod.enabled, juce::dontSendNotification);
    enableBtn.onClick = [this]
    {
        processor.modulators[(size_t) slot].enabled = enableBtn.getToggleState();
        updateEnabledLook();
    };
    addAndMakeVisible (enableBtn);
    updateEnabledLook();

    if (mod.name.isEmpty())
        mod.name = juce::String (modulatorSourceTypeName (mod.sourceType)) + " " + juce::String (slot + 1);
    nameLabel.setText (mod.name, juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    nameLabel.setEditable (true, true, false);
    nameLabel.onTextChange = [this] { processor.modulators[(size_t) slot].name = nameLabel.getText(); };
    addAndMakeVisible (nameLabel);

    addAndMakeVisible (amountKnob);
    amountKnob.attachTo (processor.apvts, ParamIDs::modAmount (slot));

    for (auto* k : { &lfoRateKnob, &envAttKnob, &envDecKnob, &envSusKnob, &envRelKnob })
        addChildComponent (*k);
    addChildComponent (lfoShapeBox);
    addChildComponent (lfoGraph);
    addChildComponent (envGraph);

    for (const auto& s : { "Sine", "Triangle", "Square", "Saw" })
        lfoShapeBox.addItem (s, lfoShapeBox.getNumItems() + 1);
    lfoShapeBox.setRepaintsOnMouseActivity (true);
    lfoShapeBox.setScrollWheelEnabled (false);

    lfoRateKnob.attachTo (processor.apvts, ParamIDs::modLfoRate (slot));
    lfoShapeAtt = std::make_unique<CA> (processor.apvts, ParamIDs::modLfoShape (slot), lfoShapeBox);
    envAttKnob.attachTo (processor.apvts, ParamIDs::modEnvAtt (slot));
    envDecKnob.attachTo (processor.apvts, ParamIDs::modEnvDec (slot));
    envSusKnob.attachTo (processor.apvts, ParamIDs::modEnvSus (slot));
    envRelKnob.attachTo (processor.apvts, ParamIDs::modEnvRel (slot));

    targetLabel.setText ("Target:", juce::dontSendNotification);
    targetLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    targetLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f)));
    addAndMakeVisible (targetLabel);

    crosshairBtn.setButtonText (juce::String (juce::CharPointer_UTF8 ("\xE2\x8C\x96")) + " Pick");
    crosshairBtn.onClick = [this]
    {
        processor.modTargetPick.active = true;
        processor.modTargetPick.onPick = [this] (const juce::String& paramID)
        {
            processor.modulators[(size_t) slot].targetParamID = paramID;
            updateTargetLabel();
        };
    };
    addAndMakeVisible (crosshairBtn);

    targetNameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    targetNameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f)));
    addAndMakeVisible (targetNameLabel);
    updateTargetLabel();

    showForSource (mod.sourceType);
}

void ModulatorRow::updateTargetLabel()
{
    const auto& target = processor.modulators[(size_t) slot].targetParamID;
    if (target.isEmpty())
    {
        targetNameLabel.setText ("(none)", juce::dontSendNotification);
        return;
    }

    for (const auto& param : processor.getModulatableParameters())
    {
        if (param.first == target)
        {
            targetNameLabel.setText (param.second, juce::dontSendNotification);
            return;
        }
    }
    targetNameLabel.setText ("(none)", juce::dontSendNotification);
}

void ModulatorRow::updateEnabledLook()
{
    enableBtn.setButtonText (enableBtn.getToggleState() ? "ON" : "OFF");
}

void ModulatorRow::showForSource (ModulatorSourceType t)
{
    const bool isLfo = (t == ModulatorSourceType::LFO);
    lfoRateKnob.setVisible (isLfo);
    lfoShapeBox.setVisible (isLfo);
    lfoGraph.setVisible (isLfo);
    for (auto* k : { &envAttKnob, &envDecKnob, &envSusKnob, &envRelKnob })
        k->setVisible (! isLfo);
    envGraph.setVisible (! isLfo);
}

void ModulatorRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), ViolentColours::cornerRadius);
    g.setColour (ViolentColours::accent.withAlpha (0.6f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), ViolentColours::cornerRadius, 1.5f);
}

void ModulatorRow::resized()
{
    auto a = getLocalBounds().reduced (6, 3);

    auto hdr = a.removeFromTop (28);
    removeBtn.setBounds (hdr.removeFromLeft (28).withSizeKeepingCentre (20, 20));
    enableBtn.setBounds (hdr.removeFromLeft (44).reduced (2, 3));
    nameLabel.setBounds (hdr.removeFromLeft (140).reduced (4, 3));

    a.removeFromTop (2);
    auto graphRow = a.removeFromTop (58);
    lfoGraph.setBounds (graphRow);
    envGraph.setBounds (graphRow);

    a.removeFromTop (4);
    auto controls = a.removeFromTop (58);
    amountKnob.setBounds (controls.removeFromLeft (72).reduced (2, 1));

    const auto srcType = processor.modulators[(size_t) slot].sourceType;
    if (srcType == ModulatorSourceType::LFO)
    {
        lfoRateKnob.setBounds (controls.removeFromLeft (72).reduced (2, 1));
        lfoShapeBox.setBounds (controls.removeFromLeft (100).reduced (2, 16));
    }
    else
    {
        const int w = controls.getWidth() / 4;
        for (auto* k : { &envAttKnob, &envDecKnob, &envSusKnob, &envRelKnob })
            k->setBounds (controls.removeFromLeft (w).reduced (2, 1));
    }

    a.removeFromTop (4);
    auto targetRow = a.removeFromTop (30);
    targetLabel.setBounds (targetRow.removeFromLeft (50));
    crosshairBtn.setBounds (targetRow.removeFromLeft (60).reduced (2, 3));
    targetNameLabel.setBounds (targetRow.reduced (6, 3));
}

//==============================================================================
// ModulatorPanel
//==============================================================================
namespace
{
    constexpr ModulatorSourceType kAddSourceTypes[NUM_MODULATOR_SOURCE_TYPES] =
        { ModulatorSourceType::LFO, ModulatorSourceType::Envelope };
}

ModulatorPanel::ModulatorPanel (ViolentAudioProcessor& p) : processor (p)
{
    sectionLabel.setText ("MODULATORS", juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    for (int i = 0; i < NUM_MODULATOR_SOURCE_TYPES; ++i)
    {
        const auto type = kAddSourceTypes[i];
        auto& btn = addTypeBtns[(size_t) i];
        btn.setButtonText (juce::String ("+ ") + modulatorSourceTypeName (type));
        addAndMakeVisible (btn);
        btn.onClick = [this, type]
        {
            if (processor.numModulators >= MAX_MODULATORS) return;
            const int m = processor.numModulators++;
            processor.modulators[(size_t) m].sourceType = type;
            addRow (m, type);
            if (onLayoutChanged) onLayoutChanged();
        };
    }
    rebuild();
}

void ModulatorPanel::addRow (int arrayIndex, ModulatorSourceType type)
{
    rows[(size_t) arrayIndex] = std::make_unique<ModulatorRow> (processor, arrayIndex);
    addAndMakeVisible (*rows[(size_t) arrayIndex]);
    rows[(size_t) arrayIndex]->showForSource (type);
    rows[(size_t) arrayIndex]->onRemove = [this, arrayIndex]
    {
        // Deferred: addRow()/rows[...] = nullptr below destroy this row, including
        // the DeleteButton whose click is invoking this very callback. Tearing it
        // down synchronously would free the component while it's still on the call
        // stack (inside Button::mouseUp), which leaves JUCE's mouse-hover/hit-test
        // tracking pointing at freed components until the next full re-hover —
        // surfacing as clicks landing in the wrong place. callAsync lets the click
        // finish unwinding first.
        juce::MessageManager::callAsync ([this, arrayIndex]
        {
            for (int j = arrayIndex; j < processor.numModulators - 1; ++j)
            {
                processor.modulators[(size_t) j] = processor.modulators[(size_t) (j + 1)];
                addRow (j, processor.modulators[(size_t) j].sourceType);
            }
            processor.modulators[(size_t) (--processor.numModulators)] = {};
            rows[(size_t) processor.numModulators] = nullptr;
            if (onLayoutChanged) onLayoutChanged();
        });
    };
}

void ModulatorPanel::refreshFromState()
{
    rebuild (true);
}

void ModulatorPanel::rebuild (bool forceRecreate)
{
    const int n = processor.numModulators;
    for (int m = 0; m < MAX_MODULATORS; ++m)
    {
        if (m < n)
        {
            if (!rows[(size_t) m] || forceRecreate)
                addRow (m, processor.modulators[(size_t) m].sourceType);
        }
        else
        {
            rows[(size_t) m] = nullptr;
        }
    }
    resized();
}

int ModulatorPanel::preferredHeight() const noexcept
{
    int h = 8 + 24; // section label
    for (int m = 0; m < processor.numModulators; ++m)
        if (rows[(size_t) m]) h += ModulatorRow::ROW_H + 8;
    h += 40; // add-type boxes
    return h;
}

void ModulatorPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    sectionLabel.setBounds (a.removeFromTop (24));
    for (int m = 0; m < processor.numModulators; ++m)
    {
        if (!rows[(size_t) m]) continue;
        rows[(size_t) m]->setBounds (a.removeFromTop (ModulatorRow::ROW_H));
        rows[(size_t) m]->resized();
        a.removeFromTop (8);
    }
    const bool canAdd = processor.numModulators < MAX_MODULATORS;
    if (canAdd)
    {
        auto row = a.removeFromTop (32).reduced (4, 2);
        const int w = row.getWidth() / NUM_MODULATOR_SOURCE_TYPES;
        for (int i = 0; i < NUM_MODULATOR_SOURCE_TYPES; ++i)
            addTypeBtns[(size_t) i].setBounds (
                (i < NUM_MODULATOR_SOURCE_TYPES - 1 ? row.removeFromLeft (w) : row).reduced (2, 0));
    }
    for (auto& btn : addTypeBtns)
        btn.setVisible (canAdd);
}

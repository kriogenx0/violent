#include "MidiModifierView.h"

//==============================================================================
// MidiModifierRow
//==============================================================================
MidiModifierRow::MidiModifierRow (ViolentAudioProcessor& p, int modSlot)
    : processor (p), slot (modSlot)
{
    auto& mm = processor.midiModifiers[(size_t) slot];

    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    enableBtn.setClickingTogglesState (true);
    enableBtn.setToggleState (mm.enabled, juce::dontSendNotification);
    enableBtn.onClick = [this]
    {
        processor.midiModifiers[(size_t) slot].enabled = enableBtn.getToggleState();
        updateEnabledLook();
    };
    addAndMakeVisible (enableBtn);
    updateEnabledLook();

    if (mm.name.isEmpty())
        mm.name = juce::String (midiModTypeName (mm.type)) + " " + juce::String (slot + 1);
    nameLabel.setText (mm.name, juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    nameLabel.setEditable (true, true, false);
    nameLabel.onTextChange = [this] { processor.midiModifiers[(size_t) slot].name = nameLabel.getText(); };
    addAndMakeVisible (nameLabel);

    for (auto* k : { &transposeKnob, &octaveKnob, &arpRateKnob }) addChildComponent (*k);

    for (const auto& t : { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" })
        keyRootBox.addItem (t, keyRootBox.getNumItems() + 1);
    keyRootBox.setRepaintsOnMouseActivity (true);
    keyRootBox.setScrollWheelEnabled (false);
    addChildComponent (keyRootBox);

    for (const auto& t : { "Major", "Minor" })
        keyScaleBox.addItem (t, keyScaleBox.getNumItems() + 1);
    keyScaleBox.setRepaintsOnMouseActivity (true);
    keyScaleBox.setScrollWheelEnabled (false);
    addChildComponent (keyScaleBox);

    transposeKnob.attachTo (processor.apvts, ParamIDs::midiModTranspose (slot));
    octaveKnob   .attachTo (processor.apvts, ParamIDs::midiModOctave    (slot));
    arpRateKnob  .attachTo (processor.apvts, ParamIDs::midiModArpRate   (slot));
    keyRootAtt   = std::make_unique<CA> (processor.apvts, ParamIDs::midiModKeyRoot  (slot), keyRootBox);
    keyScaleAtt  = std::make_unique<CA> (processor.apvts, ParamIDs::midiModKeyScale (slot), keyScaleBox);

    routingLabel.setText ("Applies to:", juce::dontSendNotification);
    routingLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    routingLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f)));
    addAndMakeVisible (routingLabel);

    for (int g = 0; g < MAX_GENERATORS; ++g)
    {
        auto& btn = routingBtns[(size_t) g];
        btn.setButtonText (juce::String (g + 1));
        btn.setClickingTogglesState (true);
        btn.setToggleState (mm.routing[(size_t) g], juce::dontSendNotification);
        btn.onClick = [this, g]
        {
            processor.midiModifiers[(size_t) slot].routing[(size_t) g] = routingBtns[(size_t) g].getToggleState();
        };
        addAndMakeVisible (btn);
    }

    showForType (mm.type);
}

void MidiModifierRow::updateEnabledLook()
{
    enableBtn.setButtonText (enableBtn.getToggleState() ? "ON" : "OFF");
}

void MidiModifierRow::setAllInvisible()
{
    transposeKnob.setVisible (false);
    octaveKnob.setVisible (false);
    arpRateKnob.setVisible (false);
    keyRootBox.setVisible (false);
    keyScaleBox.setVisible (false);
}

void MidiModifierRow::showForType (MidiModType t)
{
    setAllInvisible();
    switch (t)
    {
        case MidiModType::PitchShift:
            transposeKnob.setVisible (true); octaveKnob.setVisible (true); break;
        case MidiModType::KeyShift:
            keyRootBox.setVisible (true); keyScaleBox.setVisible (true); break;
        case MidiModType::Arp:
            arpRateKnob.setVisible (true); break;
    }
}

void MidiModifierRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f, 1.0f);
}

void MidiModifierRow::resized()
{
    auto a = getLocalBounds().reduced (6, 3);

    auto hdr = a.removeFromTop (28);
    removeBtn.setBounds (hdr.removeFromLeft (28).withSizeKeepingCentre (20, 20));
    enableBtn.setBounds (hdr.removeFromLeft (44).reduced (2, 3));
    nameLabel.setBounds (hdr.removeFromLeft (140).reduced (4, 3));

    switch (processor.midiModifiers[(size_t) slot].type)
    {
        case MidiModType::PitchShift:
            transposeKnob.setBounds (hdr.removeFromLeft (72).reduced (2, 1));
            octaveKnob   .setBounds (hdr.removeFromLeft (72).reduced (2, 1));
            break;
        case MidiModType::KeyShift:
            keyRootBox .setBounds (hdr.removeFromLeft (74).reduced (2, 3));
            keyScaleBox.setBounds (hdr.removeFromLeft (84).reduced (2, 3));
            break;
        case MidiModType::Arp:
            arpRateKnob.setBounds (hdr.removeFromLeft (72).reduced (2, 1));
            break;
    }

    a.removeFromTop (4);
    auto routingRow = a.removeFromTop (30);
    routingLabel.setBounds (routingRow.removeFromLeft (70));
    for (int g = 0; g < MAX_GENERATORS; ++g)
    {
        auto& btn = routingBtns[(size_t) g];
        btn.setVisible (g < processor.numActiveGenerators);
        if (btn.isVisible())
            btn.setBounds (routingRow.removeFromLeft (26).reduced (2, 2));
    }
}

//==============================================================================
// MidiModifierPanel
//==============================================================================
MidiModifierPanel::MidiModifierPanel (ViolentAudioProcessor& p) : processor (p)
{
    sectionLabel.setText ("MIDI MODIFIERS", juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    addAndMakeVisible (addBtn);
    addBtn.onClick = [this]
    {
        if (processor.numMidiModifiers >= MAX_MIDI_MODIFIERS) return;

        juce::PopupMenu menu;
        menu.addItem (1, "Pitch Shift");
        menu.addItem (2, "Key Shift");
        menu.addItem (3, "Arpeggiator");

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addBtn),
            [this] (int result)
            {
                if (result == 0) return;
                if (processor.numMidiModifiers >= MAX_MIDI_MODIFIERS) return;
                const MidiModType types[] = { MidiModType::PitchShift, MidiModType::KeyShift, MidiModType::Arp };
                const int m = processor.numMidiModifiers++;
                processor.midiModifiers[(size_t) m].type = types[result - 1];
                addRow (m, types[result - 1]);
                if (onLayoutChanged) onLayoutChanged();
            });
    };
    rebuild();
}

void MidiModifierPanel::addRow (int arrayIndex, MidiModType type)
{
    rows[(size_t) arrayIndex] = std::make_unique<MidiModifierRow> (processor, arrayIndex);
    addAndMakeVisible (*rows[(size_t) arrayIndex]);
    rows[(size_t) arrayIndex]->showForType (type);
    rows[(size_t) arrayIndex]->onRemove = [this, arrayIndex]
    {
        for (int j = arrayIndex; j < processor.numMidiModifiers - 1; ++j)
        {
            processor.midiModifiers[(size_t) j] = processor.midiModifiers[(size_t) (j + 1)];
            addRow (j, processor.midiModifiers[(size_t) j].type);
        }
        processor.midiModifiers[(size_t) (--processor.numMidiModifiers)] = {};
        rows[(size_t) processor.numMidiModifiers] = nullptr;
        if (onLayoutChanged) onLayoutChanged();
    };
}

void MidiModifierPanel::refreshFromState()
{
    rebuild (true);
}

void MidiModifierPanel::rebuild (bool forceRecreate)
{
    const int n = processor.numMidiModifiers;
    for (int m = 0; m < MAX_MIDI_MODIFIERS; ++m)
    {
        if (m < n)
        {
            if (!rows[(size_t) m] || forceRecreate)
                addRow (m, processor.midiModifiers[(size_t) m].type);
        }
        else
        {
            rows[(size_t) m] = nullptr;
        }
    }
    resized();
}

int MidiModifierPanel::preferredHeight() const noexcept
{
    int h = 8 + 24; // section label
    for (int m = 0; m < processor.numMidiModifiers; ++m)
        if (rows[(size_t) m]) h += MidiModifierRow::ROW_H + 8;
    h += 40; // add button
    return h;
}

void MidiModifierPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    sectionLabel.setBounds (a.removeFromTop (24));
    for (int m = 0; m < processor.numMidiModifiers; ++m)
    {
        if (!rows[(size_t) m]) continue;
        rows[(size_t) m]->setBounds (a.removeFromTop (MidiModifierRow::ROW_H));
        // setBounds() is a no-op (and skips resized()) when a row's bounds
        // happen not to have moved — which is the common case here, since
        // adding/removing a *generator* changes routing-button visibility
        // without changing any row's own position — so force it explicitly.
        rows[(size_t) m]->resized();
        a.removeFromTop (8);
    }
    if (processor.numMidiModifiers < MAX_MIDI_MODIFIERS)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numMidiModifiers < MAX_MIDI_MODIFIERS);
}

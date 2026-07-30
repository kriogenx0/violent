#include "EffectView.h"

//==============================================================================
// EffectRow
//==============================================================================
EffectRow::EffectRow (ViolentAudioProcessor& p, int fxSlot)
    : processor (p), slot (fxSlot),
      filterTypeSelector (p.apvts, ParamIDs::effectFilterType (fxSlot)),
      filterResponseView (p.apvts, ParamIDs::effectFilterType (fxSlot),
                           ParamIDs::effectFilterCut  (fxSlot),
                           ParamIDs::effectFilterRes  (fxSlot))
{
    auto& fx = processor.effects[(size_t) slot];

    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    enableBtn.setClickingTogglesState (true);
    enableBtn.setToggleState (fx.enabled, juce::dontSendNotification);
    enableBtn.onClick = [this]
    {
        processor.effects[(size_t) slot].enabled = enableBtn.getToggleState();
        updateEnabledLook();
    };
    addAndMakeVisible (enableBtn);
    updateEnabledLook();

    if (fx.name.isEmpty())
        fx.name = juce::String (fxTypeName (fx.type)) + " " + juce::String (slot + 1);
    nameLabel.setText (fx.name, juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    nameLabel.setEditable (true, true, false);
    nameLabel.onTextChange = [this] { processor.effects[(size_t) slot].name = nameLabel.getText(); };
    addAndMakeVisible (nameLabel);

    for (const auto& t : { "Soft Clip", "Hard Clip", "Fuzz" })
        distTypeBox.addItem (t, distTypeBox.getNumItems() + 1);
    distTypeBox.setRepaintsOnMouseActivity (true);
    distTypeBox.setScrollWheelEnabled (false);
    addChildComponent (distTypeBox);

    for (auto* k : { &driveKnob, &toneKnob, &levelKnob,
                     &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob,
                     &roomKnob, &dampingKnob, &wetKnob, &widthKnob,
                     &filterCutoffKnob, &filterResKnob })
        addChildComponent (*k);
    addChildComponent (filterTypeSelector);
    addChildComponent (filterResponseView);

    driveKnob    .attachTo (processor.apvts, ParamIDs::effectDrive    (slot));
    toneKnob     .attachTo (processor.apvts, ParamIDs::effectTone     (slot));
    levelKnob    .attachTo (processor.apvts, ParamIDs::effectLevel    (slot));
    distTypeAtt  = std::make_unique<CA> (processor.apvts, ParamIDs::effectDistType (slot), distTypeBox);
    threshKnob   .attachTo (processor.apvts, ParamIDs::effectThresh   (slot));
    ratioKnob    .attachTo (processor.apvts, ParamIDs::effectRatio    (slot));
    attackKnob   .attachTo (processor.apvts, ParamIDs::effectAttack   (slot));
    releaseKnob  .attachTo (processor.apvts, ParamIDs::effectRelease  (slot));
    makeupKnob   .attachTo (processor.apvts, ParamIDs::effectMakeup   (slot));
    roomKnob     .attachTo (processor.apvts, ParamIDs::effectRoom     (slot));
    dampingKnob  .attachTo (processor.apvts, ParamIDs::effectDamping  (slot));
    wetKnob      .attachTo (processor.apvts, ParamIDs::effectWet      (slot));
    widthKnob    .attachTo (processor.apvts, ParamIDs::effectWidth    (slot));
    filterCutoffKnob.attachTo (processor.apvts, ParamIDs::effectFilterCut (slot));
    filterResKnob   .attachTo (processor.apvts, ParamIDs::effectFilterRes (slot));

    routingLabel.setText ("Applies to:", juce::dontSendNotification);
    routingLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    routingLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f)));
    addAndMakeVisible (routingLabel);

    for (int g = 0; g < MAX_GENERATORS; ++g)
    {
        auto& btn = routingBtns[(size_t) g];
        btn.setButtonText (juce::String (g + 1));
        btn.setClickingTogglesState (true);
        btn.setToggleState (fx.routing[(size_t) g], juce::dontSendNotification);
        btn.onClick = [this, g]
        {
            processor.effects[(size_t) slot].routing[(size_t) g] = routingBtns[(size_t) g].getToggleState();
        };
        addAndMakeVisible (btn);
    }

    showForType (fx.type);
}

EffectRow::~EffectRow() {}

void EffectRow::updateEnabledLook()
{
    enableBtn.setButtonText (enableBtn.getToggleState() ? "ON" : "OFF");
}

void EffectRow::setAllInvisible()
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

void EffectRow::showForType (FxType t)
{
    setAllInvisible();
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

int EffectRow::preferredHeight() const noexcept
{
    const int controlsH = processor.effects[(size_t) slot].type == FxType::Filter
        ? FILTER_CONTROLS_H : CONTROLS_H;
    return 28 + controlsH + ROUTING_H + 8;
}

void EffectRow::layoutKnobs (std::initializer_list<LabelledKnob*> ks, juce::Rectangle<int> a)
{
    const int w = a.getWidth() / (int) ks.size();
    for (auto* k : ks) k->setBounds (a.removeFromLeft (w).reduced (3, 2));
}

void EffectRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f, 1.0f);
}

void EffectRow::resized()
{
    auto a = getLocalBounds().reduced (6, 3);

    auto hdr = a.removeFromTop (28);
    removeBtn.setBounds (hdr.removeFromLeft (28).withSizeKeepingCentre (20, 20));
    enableBtn.setBounds (hdr.removeFromLeft (44).reduced (2, 3));
    nameLabel.setBounds (hdr.removeFromLeft (140).reduced (4, 3));

    a.removeFromTop (2);

    const FxType t = processor.effects[(size_t) slot].type;
    const int controlsH = t == FxType::Filter ? FILTER_CONTROLS_H : CONTROLS_H;
    auto controls = a.removeFromTop (controlsH);
    switch (t)
    {
        case FxType::Distortion:
            distTypeBox.setBounds (controls.removeFromRight (80).reduced (2, 4));
            layoutKnobs ({ &driveKnob, &toneKnob, &levelKnob }, controls); break;
        case FxType::Compressor:
            layoutKnobs ({ &threshKnob, &ratioKnob, &attackKnob, &releaseKnob, &makeupKnob }, controls); break;
        case FxType::Gate:
            layoutKnobs ({ &threshKnob, &ratioKnob, &attackKnob, &releaseKnob }, controls); break;
        case FxType::Reverb:
            layoutKnobs ({ &roomKnob, &dampingKnob, &wetKnob, &widthKnob }, controls); break;
        case FxType::Filter:
        {
            filterTypeSelector.setBounds (controls.removeFromTop (24).reduced (2, 2));
            controls.removeFromTop (2);
            layoutKnobs ({ &filterCutoffKnob, &filterResKnob }, controls.removeFromTop (54));
            controls.removeFromTop (2);
            filterResponseView.setBounds (controls);
            break;
        }
        default: break;
    }

    a.removeFromTop (4);
    auto routingRow = a.removeFromTop (ROUTING_H);
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
// EffectPanel
//==============================================================================
namespace
{
    constexpr FxType kAddableFxTypes[EffectPanel::NUM_ADDABLE_FX_TYPES] =
        { FxType::Distortion, FxType::Compressor, FxType::Gate, FxType::Reverb, FxType::Filter };
}

EffectPanel::EffectPanel (ViolentAudioProcessor& p) : processor (p)
{
    sectionLabel.setText ("EFFECTS", juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    for (int i = 0; i < NUM_ADDABLE_FX_TYPES; ++i)
    {
        const auto type = kAddableFxTypes[i];
        auto& btn = addTypeBtns[(size_t) i];
        btn.setButtonText (juce::String ("+ ") + fxTypeName (type));
        addAndMakeVisible (btn);
        btn.onClick = [this, type]
        {
            if (processor.numEffects >= MAX_EFFECTS) return;
            const int x = processor.numEffects++;
            processor.effects[(size_t) x].type = type;
            addRow (x, type);
            if (onLayoutChanged) onLayoutChanged();
        };
    }
    rebuild();
}

void EffectPanel::addRow (int arrayIndex, FxType type)
{
    rows[(size_t) arrayIndex] = std::make_unique<EffectRow> (processor, arrayIndex);
    addAndMakeVisible (*rows[(size_t) arrayIndex]);
    rows[(size_t) arrayIndex]->showForType (type);
    rows[(size_t) arrayIndex]->onRemove = [this, arrayIndex]
    {
        for (int j = arrayIndex; j < processor.numEffects - 1; ++j)
        {
            processor.effects[(size_t) j] = processor.effects[(size_t) (j + 1)];
            addRow (j, processor.effects[(size_t) j].type);
        }
        processor.effects[(size_t) (--processor.numEffects)] = {};
        rows[(size_t) processor.numEffects] = nullptr;
        if (onLayoutChanged) onLayoutChanged();
    };
}

void EffectPanel::refreshFromState()
{
    rebuild (true);
}

void EffectPanel::rebuild (bool forceRecreate)
{
    const int n = processor.numEffects;
    for (int x = 0; x < MAX_EFFECTS; ++x)
    {
        if (x < n)
        {
            if (!rows[(size_t) x] || forceRecreate)
                addRow (x, processor.effects[(size_t) x].type);
        }
        else
        {
            rows[(size_t) x] = nullptr;
        }
    }
    resized();
}

int EffectPanel::preferredHeight() const noexcept
{
    int h = 8 + 24; // section label
    for (int x = 0; x < processor.numEffects; ++x)
        if (rows[(size_t) x]) h += rows[(size_t) x]->preferredHeight() + 8;
    h += 40; // add button
    return h;
}

void EffectPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    sectionLabel.setBounds (a.removeFromTop (24));
    for (int x = 0; x < processor.numEffects; ++x)
    {
        if (!rows[(size_t) x]) continue;
        rows[(size_t) x]->setBounds (a.removeFromTop (rows[(size_t) x]->preferredHeight()));
        // setBounds() is a no-op (and skips resized()) when a row's bounds
        // happen not to have moved — which is the common case here, since
        // adding/removing a *generator* changes routing-button visibility
        // without changing any row's own position — so force it explicitly.
        rows[(size_t) x]->resized();
        a.removeFromTop (8);
    }
    const bool canAdd = processor.numEffects < MAX_EFFECTS;
    if (canAdd)
    {
        auto row = a.removeFromTop (32).reduced (4, 2);
        const int w = row.getWidth() / NUM_ADDABLE_FX_TYPES;
        for (int i = 0; i < NUM_ADDABLE_FX_TYPES; ++i)
            addTypeBtns[(size_t) i].setBounds (
                (i < NUM_ADDABLE_FX_TYPES - 1 ? row.removeFromLeft (w) : row).reduced (2, 0));
    }
    for (auto& btn : addTypeBtns)
        btn.setVisible (canAdd);
}

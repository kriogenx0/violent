#include "FxBusView.h"

//==============================================================================
// FxBusRow
//==============================================================================
FxBusRow::FxBusRow (ViolentAudioProcessor& p, int busSlot)
    : processor (p), bus (busSlot),
      filterTypeSelector (p.apvts, ParamIDs::busFilterType (busSlot)),
      filterResponseView (p.apvts, ParamIDs::busFilterType (busSlot),
                           ParamIDs::busFilterCut  (busSlot),
                           ParamIDs::busFilterRes  (busSlot))
{
    titleLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    titleLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (titleLabel);

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

    driveKnob    .attachTo (processor.apvts, ParamIDs::busDrive    (bus));
    toneKnob     .attachTo (processor.apvts, ParamIDs::busTone     (bus));
    levelKnob    .attachTo (processor.apvts, ParamIDs::busLevel    (bus));
    distTypeAtt  = std::make_unique<CA> (processor.apvts, ParamIDs::busDistType (bus), distTypeBox);
    threshKnob   .attachTo (processor.apvts, ParamIDs::busThresh   (bus));
    ratioKnob    .attachTo (processor.apvts, ParamIDs::busRatio    (bus));
    attackKnob   .attachTo (processor.apvts, ParamIDs::busAttack   (bus));
    releaseKnob  .attachTo (processor.apvts, ParamIDs::busRelease  (bus));
    makeupKnob   .attachTo (processor.apvts, ParamIDs::busMakeup   (bus));
    roomKnob     .attachTo (processor.apvts, ParamIDs::busRoom     (bus));
    dampingKnob  .attachTo (processor.apvts, ParamIDs::busDamping  (bus));
    wetKnob      .attachTo (processor.apvts, ParamIDs::busWet      (bus));
    widthKnob    .attachTo (processor.apvts, ParamIDs::busWidth    (bus));
    filterCutoffKnob.attachTo (processor.apvts, ParamIDs::busFilterCut (bus));
    filterResKnob   .attachTo (processor.apvts, ParamIDs::busFilterRes (bus));

    showForType (processor.fxBusTypes[(size_t) bus]);
}

FxBusRow::~FxBusRow() {}

void FxBusRow::setAllInvisible()
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

void FxBusRow::showForType (FxType t)
{
    setAllInvisible();
    titleLabel.setText ("Bus " + juce::String (bus + 1) + ": " + fxTypeName (t), juce::dontSendNotification);
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

int FxBusRow::preferredHeight() const noexcept
{
    return processor.fxBusTypes[(size_t) bus] == FxType::Filter ? FILTER_ROW_H : ROW_H;
}

void FxBusRow::layoutKnobs (std::initializer_list<LabelledKnob*> ks, juce::Rectangle<int> a)
{
    const int w = a.getWidth() / (int) ks.size();
    for (auto* k : ks) k->setBounds (a.removeFromLeft (w).reduced (3, 2));
}

void FxBusRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f, 1.0f);
}

void FxBusRow::resized()
{
    auto a = getLocalBounds().reduced (4, 3);
    auto hdr = a.removeFromTop (22);
    removeBtn .setBounds (hdr.removeFromLeft (28).withSizeKeepingCentre (20, 20));
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
// FxBusPanel
//==============================================================================
FxBusPanel::FxBusPanel (ViolentAudioProcessor& p) : processor (p)
{
    sectionLabel.setText ("SHARED FX BUSES - generators send into these",
                           juce::dontSendNotification);
    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    addAndMakeVisible (addBtn);
    addBtn.onClick = [this]
    {
        if (processor.numFxBuses >= MAX_FX_BUSES) return;

        juce::PopupMenu menu;
        menu.addItem (1, "Distortion");
        menu.addItem (2, "Compressor");
        menu.addItem (3, "Gate");
        menu.addItem (4, "Reverb");
        menu.addItem (5, "Filter");

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addBtn),
            [this] (int result)
            {
                if (result == 0) return;
                if (processor.numFxBuses >= MAX_FX_BUSES) return;
                const FxType types[] = { FxType::Distortion, FxType::Compressor,
                                         FxType::Gate, FxType::Reverb, FxType::Filter };
                const int b = processor.numFxBuses++;
                processor.fxBusTypes[(size_t) b] = types[result - 1];
                addRow (b, types[result - 1]);
                if (onLayoutChanged) onLayoutChanged();
            });
    };
    rebuild();
}

void FxBusPanel::addRow (int arrayIndex, FxType type)
{
    rows[(size_t) arrayIndex] = std::make_unique<FxBusRow> (processor, arrayIndex);
    addAndMakeVisible (*rows[(size_t) arrayIndex]);
    rows[(size_t) arrayIndex]->showForType (type);
    rows[(size_t) arrayIndex]->onRemove = [this, arrayIndex]
    {
        for (int j = arrayIndex; j < processor.numFxBuses - 1; ++j)
        {
            processor.fxBusTypes[(size_t) j] = processor.fxBusTypes[(size_t) (j + 1)];
            addRow (j, processor.fxBusTypes[(size_t) j]);
        }
        processor.fxBusTypes[(size_t) (--processor.numFxBuses)] = FxType::None;
        rows[(size_t) processor.numFxBuses] = nullptr;
        if (onLayoutChanged) onLayoutChanged();
    };
}

void FxBusPanel::refreshFromState()
{
    rebuild (true);
}

void FxBusPanel::rebuild (bool forceRecreate)
{
    const int n = processor.numFxBuses;
    for (int b = 0; b < MAX_FX_BUSES; ++b)
    {
        if (b < n)
        {
            if (!rows[(size_t) b] || forceRecreate)
                addRow (b, processor.fxBusTypes[(size_t) b]);
        }
        else
        {
            rows[(size_t) b] = nullptr;
        }
    }
    resized();
}

int FxBusPanel::preferredHeight() const noexcept
{
    int h = 8 + 24; // section label
    for (int b = 0; b < processor.numFxBuses; ++b)
        if (rows[(size_t) b]) h += rows[(size_t) b]->preferredHeight() + 8;
    h += 40; // add button
    return h;
}

void FxBusPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    sectionLabel.setBounds (a.removeFromTop (24));
    for (int b = 0; b < processor.numFxBuses; ++b)
    {
        if (!rows[(size_t) b]) continue;
        rows[(size_t) b]->setBounds (a.removeFromTop (rows[(size_t) b]->preferredHeight()));
        a.removeFromTop (8);
    }
    if (processor.numFxBuses < MAX_FX_BUSES)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numFxBuses < MAX_FX_BUSES);
}


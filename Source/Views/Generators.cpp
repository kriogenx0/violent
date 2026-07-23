#include "Generators.h"

//==============================================================================
// GeneratorFxCard
//==============================================================================
GeneratorFxCard::GeneratorFxCard (ViolentAudioProcessor& p, int generatorIdx, int fxSlot)
    : processor (p), generator (generatorIdx), slot (fxSlot),
      filterTypeSelector (p.apvts, ParamIDs::genFxFilterType (generatorIdx, fxSlot)),
      filterResponseView (p.apvts, ParamIDs::genFxFilterType (generatorIdx, fxSlot),
                           ParamIDs::genFxFilterCut  (generatorIdx, fxSlot),
                           ParamIDs::genFxFilterRes  (generatorIdx, fxSlot))
{
    titleLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Bold")));
    titleLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (titleLabel);

    removeBtn.setColour (juce::TextButton::textColourOffId, ViolentColours::red);
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

    driveKnob    .attachTo (processor.apvts, ParamIDs::genFxDrive    (generator, slot));
    toneKnob     .attachTo (processor.apvts, ParamIDs::genFxTone     (generator, slot));
    levelKnob    .attachTo (processor.apvts, ParamIDs::genFxLevel    (generator, slot));
    distTypeAtt  = std::make_unique<CA> (processor.apvts, ParamIDs::genFxDistType (generator, slot), distTypeBox);
    threshKnob   .attachTo (processor.apvts, ParamIDs::genFxThresh   (generator, slot));
    ratioKnob    .attachTo (processor.apvts, ParamIDs::genFxRatio    (generator, slot));
    attackKnob   .attachTo (processor.apvts, ParamIDs::genFxAttack   (generator, slot));
    releaseKnob  .attachTo (processor.apvts, ParamIDs::genFxRelease  (generator, slot));
    makeupKnob   .attachTo (processor.apvts, ParamIDs::genFxMakeup   (generator, slot));
    roomKnob     .attachTo (processor.apvts, ParamIDs::genFxRoom     (generator, slot));
    dampingKnob  .attachTo (processor.apvts, ParamIDs::genFxDamping  (generator, slot));
    wetKnob      .attachTo (processor.apvts, ParamIDs::genFxWet      (generator, slot));
    widthKnob    .attachTo (processor.apvts, ParamIDs::genFxWidth    (generator, slot));
    filterCutoffKnob.attachTo (processor.apvts, ParamIDs::genFxFilterCut (generator, slot));
    filterResKnob   .attachTo (processor.apvts, ParamIDs::genFxFilterRes (generator, slot));

    showForType (processor.generators[(size_t) generator].fxTypes[(size_t) slot]);
}

GeneratorFxCard::~GeneratorFxCard() {}

void GeneratorFxCard::setAllInvisible()
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

void GeneratorFxCard::showForType (FxType t)
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
        case FxType::Filter:
            filterTypeSelector.setVisible(true);
            filterCutoffKnob.setVisible(true); filterResKnob.setVisible(true);
            filterResponseView.setVisible(true); break;
        default: break;
    }
}

int GeneratorFxCard::preferredHeight() const noexcept
{
    return processor.generators[(size_t) generator].fxTypes[(size_t) slot] == FxType::Filter
        ? FILTER_CARD_H : CARD_H;
}

void GeneratorFxCard::layoutKnobs (std::initializer_list<LabelledKnob*> ks, juce::Rectangle<int> a)
{
    const int w = a.getWidth() / (int) ks.size();
    for (auto* k : ks) k->setBounds (a.removeFromLeft (w).reduced (3, 2));
}

void GeneratorFxCard::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f, 1.0f);
}

void GeneratorFxCard::resized()
{
    auto a = getLocalBounds().reduced (4, 3);
    auto hdr = a.removeFromTop (22);
    removeBtn .setBounds (hdr.removeFromLeft (24));
    titleLabel.setBounds (hdr);
    a.removeFromTop (2);

    const FxType t = processor.generators[(size_t) generator].fxTypes[(size_t) slot];
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
// GeneratorMidiRow
//==============================================================================
GeneratorMidiRow::GeneratorMidiRow (ViolentAudioProcessor& p, int generatorIdx, int modSlot)
    : processor (p), generator (generatorIdx), slot (modSlot)
{
    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    sectionLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    sectionLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f).withStyle ("Bold")));
    addAndMakeVisible (sectionLabel);

    for (auto* k : { &transposeKnob, &octaveKnob, &arpRateKnob }) addChildComponent (*k);

    for (const auto& t : { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" })
        keyRootBox.addItem (t, keyRootBox.getNumItems() + 1);
    keyRootBox.setRepaintsOnMouseActivity (true);
    addChildComponent (keyRootBox);

    for (const auto& t : { "Major", "Minor" })
        keyScaleBox.addItem (t, keyScaleBox.getNumItems() + 1);
    keyScaleBox.setRepaintsOnMouseActivity (true);
    addChildComponent (keyScaleBox);

    transposeKnob.attachTo (processor.apvts, ParamIDs::genMidiTranspose (generator, slot));
    octaveKnob   .attachTo (processor.apvts, ParamIDs::genMidiOctave    (generator, slot));
    arpRateKnob  .attachTo (processor.apvts, ParamIDs::genMidiArpRate   (generator, slot));
    keyRootAtt   = std::make_unique<CA> (processor.apvts, ParamIDs::genMidiKeyRoot  (generator, slot), keyRootBox);
    keyScaleAtt  = std::make_unique<CA> (processor.apvts, ParamIDs::genMidiKeyScale (generator, slot), keyScaleBox);

    showForType (processor.generators[(size_t) generator].midiModTypes[(size_t) slot]);
}

void GeneratorMidiRow::setAllInvisible()
{
    transposeKnob.setVisible (false);
    octaveKnob.setVisible (false);
    arpRateKnob.setVisible (false);
    keyRootBox.setVisible (false);
    keyScaleBox.setVisible (false);
}

void GeneratorMidiRow::showForType (MidiModType t)
{
    setAllInvisible();
    sectionLabel.setText (midiModTypeName (t), juce::dontSendNotification);
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

void GeneratorMidiRow::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 5.0f);
}

void GeneratorMidiRow::resized()
{
    auto a = getLocalBounds().reduced (6, 3);
    removeBtn.setBounds (a.removeFromLeft (28).withSizeKeepingCentre (20, 20));
    sectionLabel.setBounds (a.removeFromLeft (96));
    a.removeFromLeft (4);

    const auto t = processor.generators[(size_t) generator].midiModTypes[(size_t) slot];
    switch (t)
    {
        case MidiModType::PitchShift:
            transposeKnob.setBounds (a.removeFromLeft (72).reduced (2, 1));
            octaveKnob   .setBounds (a.removeFromLeft (72).reduced (2, 1));
            break;
        case MidiModType::KeyShift:
            keyRootBox .setBounds (a.removeFromLeft (74).reduced (2, 14));
            keyScaleBox.setBounds (a.removeFromLeft (84).reduced (2, 14));
            break;
        case MidiModType::Arp:
            arpRateKnob.setBounds (a.removeFromLeft (72).reduced (2, 1));
            break;
    }
}

//==============================================================================
// Shared routing-arrow drawing — used between every stage of the signal
// chain (MIDI modifier -> generator -> filters -> effects), whether the two
// stages are inside the same component or, like the MIDI modifier and its
// generator, separate sibling components.
//==============================================================================
static void drawGeneratorRoutingArrow (juce::Graphics& g, int width, int y)
{
    const float cx = (float) width * 0.5f;
    const float halfLineLen = 60.0f;

    g.setColour (ViolentColours::accent.withAlpha (0.35f));
    g.drawLine (cx - halfLineLen, (float) y, cx + halfLineLen, (float) y, 1.0f);

    juce::Path arrow;
    arrow.addTriangle (cx - 5.0f, (float) y - 4.0f, cx + 5.0f, (float) y - 4.0f, cx, (float) y + 4.0f);
    g.setColour (ViolentColours::accent);
    g.fillPath (arrow);
}

//==============================================================================
// GeneratorCard
//==============================================================================
GeneratorCard::GeneratorCard (ViolentAudioProcessor& p, int generatorIdx)
    : processor (p), generator (generatorIdx), waveformView (p, generatorIdx)
{
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    addAndMakeVisible (nameLabel);
    addAndMakeVisible (waveformView);

    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    enableBtn.setButtonText ("ON");
    enableBtn.setClickingTogglesState (true);
    addAndMakeVisible (enableBtn);
    enableAtt = std::make_unique<BA> (processor.apvts, ParamIDs::generatorEn (generator), enableBtn);

    // Source type
    for (const auto& t : { "Sine", "Saw", "Square", "Triangle", "Noise", "Sample" })
        srcTypeBox.addItem (t, srcTypeBox.getNumItems() + 1);
    srcTypeBox.setRepaintsOnMouseActivity (true);
    addAndMakeVisible (srcTypeBox);
    srcTypeAtt = std::make_unique<CA> (processor.apvts, ParamIDs::genSrcType (generator), srcTypeBox);

    addAndMakeVisible (synthModeBtn);
    addAndMakeVisible (samplerModeBtn);
    synthModeBtn.onClick = [this]
    {
        if (srcTypeBox.getSelectedItemIndex() == (int) SourceType::Sample)
            srcTypeBox.setSelectedItemIndex (lastWaveformIndex);
    };
    samplerModeBtn.onClick = [this]
    {
        if (srcTypeBox.getSelectedItemIndex() != (int) SourceType::Sample)
            lastWaveformIndex = srcTypeBox.getSelectedItemIndex();
        srcTypeBox.setSelectedItemIndex ((int) SourceType::Sample);
    };

    loadSampleBtn.onClick = [this] { openFilePicker(); };
    addChildComponent (loadSampleBtn);
    sampleFileLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addChildComponent (sampleFileLabel);

    // Source knobs
    for (auto* k : { &gainKnob, &octKnob, &semiKnob, &detKnob, &phaseKnob,
                     &pwKnob, &panKnob, &velKnob, &uniKnob, &uniSpreadKnob,
                     &attKnob, &decKnob, &susKnob, &relKnob,
                     &levelKnob, &generatorPan })
        addAndMakeVisible (*k);

    gainKnob      .attachTo (processor.apvts, ParamIDs::genSrcGain      (generator));
    octKnob       .attachTo (processor.apvts, ParamIDs::genSrcOct       (generator));
    semiKnob      .attachTo (processor.apvts, ParamIDs::genSrcSemi      (generator));
    detKnob       .attachTo (processor.apvts, ParamIDs::genSrcDet       (generator));
    phaseKnob     .attachTo (processor.apvts, ParamIDs::genSrcPhase     (generator));
    pwKnob        .attachTo (processor.apvts, ParamIDs::genSrcPW        (generator));
    panKnob       .attachTo (processor.apvts, ParamIDs::genSrcPan       (generator));
    velKnob       .attachTo (processor.apvts, ParamIDs::genSrcVel       (generator));
    uniKnob       .attachTo (processor.apvts, ParamIDs::genSrcUni       (generator));
    uniSpreadKnob .attachTo (processor.apvts, ParamIDs::genSrcUniSpread (generator));
    attKnob       .attachTo (processor.apvts, ParamIDs::genSrcAtt       (generator));
    decKnob       .attachTo (processor.apvts, ParamIDs::genSrcDec       (generator));
    susKnob       .attachTo (processor.apvts, ParamIDs::genSrcSus       (generator));
    relKnob       .attachTo (processor.apvts, ParamIDs::genSrcRel       (generator));
    levelKnob     .attachTo (processor.apvts, ParamIDs::generatorLevel  (generator));
    generatorPan  .attachTo (processor.apvts, ParamIDs::generatorPan    (generator));

    for (int b = 0; b < MAX_FX_BUSES; ++b)
    {
        addChildComponent (sendKnobs[(size_t) b]);
        sendKnobs[(size_t) b].attachTo (processor.apvts, ParamIDs::genSend (generator, b));
    }

    // FX chain (filters are just another selectable effect type)
    const auto& gen = processor.generators[(size_t) generator];
    for (int x = 0; x < gen.numFx; ++x)
        addFxCard (x, gen.fxTypes[(size_t) x]);

    addAndMakeVisible (addFxBtn);
    addFxBtn.onClick = [this]
    {
        auto& s = processor.generators[(size_t) generator];
        if (s.numFx >= MAX_GENERATOR_FX) return;

        juce::PopupMenu menu;
        menu.addItem (1, "Distortion");
        menu.addItem (2, "Compressor");
        menu.addItem (3, "Gate");
        menu.addItem (4, "Reverb");
        menu.addItem (5, "Filter");

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addFxBtn),
            [this] (int result)
            {
                if (result == 0) return;
                const FxType types[] = { FxType::Distortion, FxType::Compressor,
                                         FxType::Gate, FxType::Reverb, FxType::Filter };
                auto& s  = processor.generators[(size_t) generator];
                const int x = s.numFx++;
                s.fxTypes[(size_t) x] = types[result - 1];
                addFxCard (x, types[result - 1]);
                if (onLayoutChanged) onLayoutChanged();
            });
    };

    srcTypeBox.onChange = [this]
    {
        const bool isSample = (srcTypeBox.getSelectedItemIndex() == (int) SourceType::Sample);
        if (! isSample) lastWaveformIndex = srcTypeBox.getSelectedItemIndex();

        nameLabel.setText (isSample ? "Sampler" : "Synth", juce::dontSendNotification);

        srcTypeBox.setVisible      (! isSample);
        loadSampleBtn.setVisible   (isSample);
        sampleFileLabel.setVisible (isSample);

        synthModeBtn.setToggleState   (! isSample, juce::dontSendNotification);
        samplerModeBtn.setToggleState (  isSample, juce::dontSendNotification);

        resized();
        if (onLayoutChanged) onLayoutChanged();
    };
    srcTypeBox.onChange();
}

GeneratorCard::~GeneratorCard() {}

void GeneratorCard::addFxCard (int arrayIndex, FxType type)
{
    fxCards[(size_t) arrayIndex] = std::make_unique<GeneratorFxCard> (processor, generator, arrayIndex);
    addAndMakeVisible (*fxCards[(size_t) arrayIndex]);
    fxCards[(size_t) arrayIndex]->showForType (type);
    fxCards[(size_t) arrayIndex]->onRemove = [this, arrayIndex]
    {
        auto& s = processor.generators[(size_t) generator];
        for (int j = arrayIndex; j < s.numFx - 1; ++j)
        {
            s.fxTypes[(size_t) j] = s.fxTypes[(size_t) (j + 1)];
            addFxCard (j, s.fxTypes[(size_t) j]);
        }
        s.fxTypes[(size_t) (--s.numFx)] = FxType::None;
        fxCards[(size_t) s.numFx] = nullptr;
        if (onLayoutChanged) onLayoutChanged();
    };
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
                processor.loadSample (generator, f);
                sampleFileLabel.setText (f.getFileName(), juce::dontSendNotification);
            }
        });
}

int GeneratorCard::preferredHeight() const noexcept
{
    const auto& gen = processor.generators[(size_t) generator];
    int fxHeight = 0;
    for (int x = 0; x < gen.numFx; ++x)
        fxHeight += (fxCards[(size_t) x] ? fxCards[(size_t) x]->preferredHeight() : GeneratorFxCard::CARD_H) + 4;

    return HEADER_H + SOURCE_H
         + ARROW_H
         + fxHeight
         + (gen.numFx < MAX_GENERATOR_FX ? 32 : 0)
         + (processor.numFxBuses > 0 ? 62 : 0)
         + FOOTER_H + 8;
}

void GeneratorCard::paint (juce::Graphics& g)
{
    g.setColour (ViolentColours::overlay);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 8.0f);
    g.setColour (ViolentColours::accent.withAlpha (0.6f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 8.0f, 1.5f);

    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (adsrBoxBounds.toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (adsrBoxBounds.toFloat().reduced (2.0f), 5.0f, 1.0f);

    drawRoutingArrow (g, effectArrowY);
}

void GeneratorCard::drawRoutingArrow (juce::Graphics& g, int y) const
{
    drawGeneratorRoutingArrow (g, getWidth(), y);
}

void GeneratorCard::resized()
{
    auto a = getLocalBounds().reduced (6, 4);

    // Header
    auto hdr = a.removeFromTop (HEADER_H);
    enableBtn.setBounds (hdr.removeFromRight (40).reduced (2, 4));
    removeBtn.setBounds (hdr.removeFromLeft (28).withSizeKeepingCentre (20, 20));
    nameLabel.setBounds (hdr.removeFromLeft (70));
    synthModeBtn.setBounds   (hdr.removeFromLeft (55).reduced (2, 4));
    samplerModeBtn.setBounds (hdr.removeFromLeft (65).reduced (2, 4));
    if (srcTypeBox.isVisible())
        srcTypeBox.setBounds (hdr.removeFromLeft (90).reduced (2, 4));
    if (loadSampleBtn.isVisible())
    {
        loadSampleBtn.setBounds (hdr.removeFromLeft (70).reduced (2, 4));
        sampleFileLabel.setBounds (hdr.removeFromLeft (110).reduced (2, 4));
    }
    waveformView.setBounds (hdr.reduced (6, 4));

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

    // ADSR box — aligned to the same column grid as the source knobs above
    auto adsr = a.removeFromTop (58);
    adsrBoxBounds = adsr;
    attKnob.setBounds (adsr.removeFromLeft (r1w).reduced (2, 1));
    decKnob.setBounds (adsr.removeFromLeft (r1w).reduced (2, 1));
    susKnob.setBounds (adsr.removeFromLeft (r1w).reduced (2, 1));
    relKnob.setBounds (adsr.removeFromLeft (r1w).reduced (2, 1));

    effectArrowY = a.removeFromTop (ARROW_H).getCentreY();

    // FX (filters are just another selectable effect type in this chain)
    const auto& gen = processor.generators[(size_t) generator];
    for (int x = 0; x < MAX_GENERATOR_FX; ++x)
    {
        if (fxCards[(size_t) x])
        {
            fxCards[(size_t) x]->setBounds (a.removeFromTop (fxCards[(size_t) x]->preferredHeight()));
            a.removeFromTop (4);
        }
    }
    if (gen.numFx < MAX_GENERATOR_FX)
    {
        addFxBtn.setBounds (a.removeFromTop (28).reduced (4, 2));
        a.removeFromTop (4);
    }
    addFxBtn.setVisible (gen.numFx < MAX_GENERATOR_FX);

    // Sends to shared FX buses, if any exist
    if (processor.numFxBuses > 0)
    {
        auto sendsRow = a.removeFromTop (58);
        const int sw = sendsRow.getWidth() / processor.numFxBuses;
        for (int b = 0; b < processor.numFxBuses; ++b)
            sendKnobs[(size_t) b].setBounds (sendsRow.removeFromLeft (sw).reduced (2, 1));
        a.removeFromTop (4);
    }
    for (int b = 0; b < MAX_FX_BUSES; ++b)
        sendKnobs[(size_t) b].setVisible (b < processor.numFxBuses);

    // Footer: level + pan — aligned to the same column grid as everything above
    a.removeFromTop (4);
    levelKnob.setBounds (a.removeFromLeft (r1w).reduced (2, 1));
    a.removeFromLeft (r1w * 4);
    generatorPan.setBounds (a.removeFromLeft (r1w).reduced (2, 1));
}

//==============================================================================
// GeneratorUnit
//==============================================================================
GeneratorUnit::GeneratorUnit (ViolentAudioProcessor& p, int generatorIdx)
    : processor (p), generator (generatorIdx), card (p, generatorIdx)
{
    addAndMakeVisible (card);
    card.onRemove = [this] { if (onRemove) onRemove(); };
    card.onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };

    const auto& gen = processor.generators[(size_t) generator];
    for (int m = 0; m < gen.numMidiMods; ++m)
        addMidiRow (m, gen.midiModTypes[(size_t) m]);

    addAndMakeVisible (addMidiBtn);
    addMidiBtn.onClick = [this]
    {
        auto& s = processor.generators[(size_t) generator];
        if (s.numMidiMods >= MAX_GENERATOR_MIDI_MODS) return;

        juce::PopupMenu menu;
        menu.addItem (1, "Pitch Shift");
        menu.addItem (2, "Key Shift");
        menu.addItem (3, "Arpeggiator");

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addMidiBtn),
            [this] (int result)
            {
                if (result == 0) return;
                const MidiModType types[] = { MidiModType::PitchShift, MidiModType::KeyShift, MidiModType::Arp };
                auto& s2 = processor.generators[(size_t) generator];
                const int m = s2.numMidiMods++;
                s2.midiModTypes[(size_t) m] = types[result - 1];
                addMidiRow (m, types[result - 1]);
                if (onLayoutChanged) onLayoutChanged();
            });
    };
}

void GeneratorUnit::addMidiRow (int arrayIndex, MidiModType type)
{
    midiRows[(size_t) arrayIndex] = std::make_unique<GeneratorMidiRow> (processor, generator, arrayIndex);
    addAndMakeVisible (*midiRows[(size_t) arrayIndex]);
    midiRows[(size_t) arrayIndex]->showForType (type);
    midiRows[(size_t) arrayIndex]->onRemove = [this, arrayIndex]
    {
        auto& s = processor.generators[(size_t) generator];
        for (int j = arrayIndex; j < s.numMidiMods - 1; ++j)
        {
            s.midiModTypes[(size_t) j] = s.midiModTypes[(size_t) (j + 1)];
            addMidiRow (j, s.midiModTypes[(size_t) j]);
        }
        --s.numMidiMods;
        midiRows[(size_t) s.numMidiMods] = nullptr;
        if (onLayoutChanged) onLayoutChanged();
    };
}

int GeneratorUnit::preferredHeight() const noexcept
{
    const auto& gen = processor.generators[(size_t) generator];
    int h = gen.numMidiMods * (GeneratorMidiRow::ROW_H + 4);
    if (gen.numMidiMods < MAX_GENERATOR_MIDI_MODS) h += ADD_MIDI_BTN_H + 4;
    if (gen.numMidiMods > 0) h += ARROW_H;
    return h + card.preferredHeight();
}

void GeneratorUnit::paint (juce::Graphics& g)
{
    if (processor.generators[(size_t) generator].numMidiMods > 0)
        drawGeneratorRoutingArrow (g, getWidth(), arrowY);
}

void GeneratorUnit::resized()
{
    auto a = getLocalBounds();
    const auto& gen = processor.generators[(size_t) generator];

    for (int m = 0; m < MAX_GENERATOR_MIDI_MODS; ++m)
    {
        if (midiRows[(size_t) m])
        {
            midiRows[(size_t) m]->setBounds (a.removeFromTop (GeneratorMidiRow::ROW_H));
            a.removeFromTop (4);
        }
    }
    if (gen.numMidiMods < MAX_GENERATOR_MIDI_MODS)
    {
        addMidiBtn.setBounds (a.removeFromTop (ADD_MIDI_BTN_H).reduced (4, 2));
        a.removeFromTop (4);
    }
    addMidiBtn.setVisible (gen.numMidiMods < MAX_GENERATOR_MIDI_MODS);

    if (gen.numMidiMods > 0)
        arrowY = a.removeFromTop (ARROW_H).getCentreY();

    card.setBounds (a);
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

void GeneratorPanel::refreshFromState()
{
    rebuild (true);
}

void GeneratorPanel::rebuild (bool forceRecreate)
{
    const int n = processor.numActiveGenerators;
    for (int s = 0; s < MAX_GENERATORS; ++s)
    {
        if (s < n)
        {
            if (!units[(size_t) s] || forceRecreate)
            {
                units[(size_t) s] = std::make_unique<GeneratorUnit> (processor, s);
                addAndMakeVisible (*units[(size_t) s]);
                units[(size_t) s]->onLayoutChanged = [this] { if (onLayoutChanged) onLayoutChanged(); };
                units[(size_t) s]->onRemove = [this, s]
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
            units[(size_t) s] = nullptr;
        }
    }
    resized();
}

int GeneratorPanel::preferredHeight() const noexcept
{
    int h = 8;
    for (int s = 0; s < processor.numActiveGenerators; ++s)
        if (units[(size_t) s]) h += units[(size_t) s]->preferredHeight() + 8;
    h += 40; // add button
    return h;
}

void GeneratorPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    for (int s = 0; s < processor.numActiveGenerators; ++s)
    {
        if (!units[(size_t) s]) continue;
        const int h = units[(size_t) s]->preferredHeight();
        units[(size_t) s]->setBounds (a.removeFromTop (h));
        a.removeFromTop (8);
    }
    if (processor.numActiveGenerators < MAX_GENERATORS)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numActiveGenerators < MAX_GENERATORS);
}


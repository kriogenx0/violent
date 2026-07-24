#include "Generators.h"

//==============================================================================
// GeneratorCard
//==============================================================================
GeneratorCard::GeneratorCard (ViolentAudioProcessor& p, int generatorIdx)
    : processor (p), generator (generatorIdx), waveformView (p, generatorIdx)
{
    auto& gen = processor.generators[(size_t) generator];

    addAndMakeVisible (removeBtn);
    removeBtn.onClick = [this] { if (onRemove) onRemove(); };

    enableBtn.setButtonText ("ON");
    enableBtn.setClickingTogglesState (true);
    addAndMakeVisible (enableBtn);
    enableAtt = std::make_unique<BA> (processor.apvts, ParamIDs::generatorEn (generator), enableBtn);

    if (gen.name.isEmpty())
    {
        const bool isSample = ((int) processor.apvts.getRawParameterValue (ParamIDs::genSrcType (generator))->load()
                                == (int) SourceType::Sample);
        gen.name = (isSample ? "Sampler " : "Synth ") + juce::String (generator + 1);
    }
    nameLabel.setText (gen.name, juce::dontSendNotification);
    nameLabel.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (13.0f).withStyle ("Bold")));
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    nameLabel.setEditable (true, true, false);
    nameLabel.onTextChange = [this] { processor.generators[(size_t) generator].name = nameLabel.getText(); };
    addAndMakeVisible (nameLabel);

    colourBtn.setColour (gen.colour);
    colourBtn.onColourChanged = [this] (juce::Colour c) { processor.generators[(size_t) generator].colour = c; repaint(); };
    addAndMakeVisible (colourBtn);

    addAndMakeVisible (waveformView);

    // Source type
    for (const auto& t : { "Sine", "Saw", "Square", "Triangle", "Noise", "Sample" })
        srcTypeBox.addItem (t, srcTypeBox.getNumItems() + 1);
    srcTypeBox.setRepaintsOnMouseActivity (true);
    srcTypeBox.setScrollWheelEnabled (false);
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

    for (auto* l : { &sourceLabel, &waveShapeLabel, &adsrLabel })
    {
        l->setColour (juce::Label::textColourId, ViolentColours::subtext);
        l->setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (11.0f).withStyle ("Bold")));
        addAndMakeVisible (*l);
    }
    sourceLabel.setText ("SOURCE", juce::dontSendNotification);
    waveShapeLabel.setText ("WAVE SHAPE", juce::dontSendNotification);
    adsrLabel.setText ("ENVELOPE", juce::dontSendNotification);

    for (auto* k : { &gainKnob, &panKnob, &velKnob,
                     &detKnob, &phaseKnob, &pwKnob, &uniKnob, &uniSpreadKnob,
                     &attKnob, &decKnob, &susKnob, &relKnob })
        addAndMakeVisible (*k);

    gainKnob      .attachTo (processor.apvts, ParamIDs::genSrcGain      (generator));
    panKnob       .attachTo (processor.apvts, ParamIDs::genSrcPan       (generator));
    velKnob       .attachTo (processor.apvts, ParamIDs::genSrcVel       (generator));
    detKnob       .attachTo (processor.apvts, ParamIDs::genSrcDet       (generator));
    phaseKnob     .attachTo (processor.apvts, ParamIDs::genSrcPhase     (generator));
    pwKnob        .attachTo (processor.apvts, ParamIDs::genSrcPW        (generator));
    uniKnob       .attachTo (processor.apvts, ParamIDs::genSrcUni       (generator));
    uniSpreadKnob .attachTo (processor.apvts, ParamIDs::genSrcUniSpread (generator));
    attKnob       .attachTo (processor.apvts, ParamIDs::genSrcAtt       (generator));
    decKnob       .attachTo (processor.apvts, ParamIDs::genSrcDec       (generator));
    susKnob       .attachTo (processor.apvts, ParamIDs::genSrcSus       (generator));
    relKnob       .attachTo (processor.apvts, ParamIDs::genSrcRel       (generator));

    srcTypeBox.onChange = [this]
    {
        const bool isSample = (srcTypeBox.getSelectedItemIndex() == (int) SourceType::Sample);
        if (! isSample) lastWaveformIndex = srcTypeBox.getSelectedItemIndex();

        srcTypeBox.setVisible      (! isSample);
        loadSampleBtn.setVisible   (isSample);
        sampleFileLabel.setVisible (isSample);

        synthModeBtn.setToggleState   (! isSample, juce::dontSendNotification);
        samplerModeBtn.setToggleState (  isSample, juce::dontSendNotification);

        resized();
    };
    srcTypeBox.onChange();
}

GeneratorCard::~GeneratorCard() {}

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
    return HEADER_H + SOURCE_H + WAVESHAPE_H + ADSR_H + 64;
}

void GeneratorCard::paint (juce::Graphics& g)
{
    const auto accent = processor.generators[(size_t) generator].colour;

    g.setColour (ViolentColours::overlay);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 8.0f);
    g.setColour (accent.withAlpha (0.6f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 8.0f, 1.5f);

    g.setColour (ViolentColours::surface);
    g.fillRoundedRectangle (waveShapeBoxBounds.toFloat().reduced (2.0f), 5.0f);
    g.fillRoundedRectangle (adsrBoxBounds.toFloat().reduced (2.0f), 5.0f);
    g.setColour (ViolentColours::overlay);
    g.drawRoundedRectangle (waveShapeBoxBounds.toFloat().reduced (2.0f), 5.0f, 1.0f);
    g.drawRoundedRectangle (adsrBoxBounds.toFloat().reduced (2.0f), 5.0f, 1.0f);
}

void GeneratorCard::resized()
{
    auto a = getLocalBounds().reduced (6, 4);

    // Header — top-left: delete/enable/name; top-right: colour swatch
    auto hdr = a.removeFromTop (HEADER_H);
    colourBtn.setBounds (hdr.removeFromRight (28).reduced (2, 6));
    removeBtn.setBounds (hdr.removeFromLeft (28).withSizeKeepingCentre (20, 20));
    enableBtn.setBounds (hdr.removeFromLeft (36).reduced (2, 6));
    nameLabel.setBounds (hdr.removeFromLeft (100).reduced (4, 6));
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

    // Source section
    sourceLabel.setBounds (a.removeFromTop (14));
    auto srcRow = a.removeFromTop (SOURCE_H - 14);
    const int sw = srcRow.getWidth() / 3;
    gainKnob.setBounds (srcRow.removeFromLeft (sw).reduced (2, 1));
    panKnob .setBounds (srcRow.removeFromLeft (sw).reduced (2, 1));
    velKnob .setBounds (srcRow.reduced (2, 1));

    a.removeFromTop (8);

    // Wave-shape box — everything that modifies the raw wave signal
    waveShapeLabel.setBounds (a.removeFromTop (14));
    auto wsOuter = a.removeFromTop (WAVESHAPE_H);
    waveShapeBoxBounds = wsOuter;
    auto wsInner = wsOuter.reduced (8, 6);
    const int wsw = wsInner.getWidth() / 5;
    detKnob      .setBounds (wsInner.removeFromLeft (wsw).reduced (2, 1));
    phaseKnob    .setBounds (wsInner.removeFromLeft (wsw).reduced (2, 1));
    pwKnob       .setBounds (wsInner.removeFromLeft (wsw).reduced (2, 1));
    uniKnob      .setBounds (wsInner.removeFromLeft (wsw).reduced (2, 1));
    uniSpreadKnob.setBounds (wsInner.reduced (2, 1));

    a.removeFromTop (8);

    // ADSR box
    adsrLabel.setBounds (a.removeFromTop (14));
    auto adsrOuter = a.removeFromTop (ADSR_H);
    adsrBoxBounds = adsrOuter;
    auto adsrInner = adsrOuter.reduced (8, 6);
    const int aw = adsrInner.getWidth() / 4;
    attKnob.setBounds (adsrInner.removeFromLeft (aw).reduced (2, 1));
    decKnob.setBounds (adsrInner.removeFromLeft (aw).reduced (2, 1));
    susKnob.setBounds (adsrInner.removeFromLeft (aw).reduced (2, 1));
    relKnob.setBounds (adsrInner.reduced (2, 1));
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
            if (!cards[(size_t) s] || forceRecreate)
            {
                cards[(size_t) s] = std::make_unique<GeneratorCard> (processor, s);
                addAndMakeVisible (*cards[(size_t) s]);
                cards[(size_t) s]->onRemove = [this, s]
                {
                    if (processor.numActiveGenerators <= 1) return;
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
            cards[(size_t) s] = nullptr;
        }
    }
    resized();
}

int GeneratorPanel::preferredHeight() const noexcept
{
    int h = 8;
    for (int s = 0; s < processor.numActiveGenerators; ++s)
        if (cards[(size_t) s]) h += cards[(size_t) s]->preferredHeight() + 8;
    h += 40; // add button
    return h;
}

void GeneratorPanel::resized()
{
    auto a = getLocalBounds().reduced (8, 4);
    for (int s = 0; s < processor.numActiveGenerators; ++s)
    {
        if (!cards[(size_t) s]) continue;
        const int h = cards[(size_t) s]->preferredHeight();
        cards[(size_t) s]->setBounds (a.removeFromTop (h));
        a.removeFromTop (8);
    }
    if (processor.numActiveGenerators < MAX_GENERATORS)
        addBtn.setBounds (a.removeFromTop (32).reduced (4, 2));
    addBtn.setVisible (processor.numActiveGenerators < MAX_GENERATORS);
}

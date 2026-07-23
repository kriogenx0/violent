#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** One FX card inside a generator's effect chain. */
class GeneratorFxCard : public juce::Component
{
public:
    static constexpr int CARD_H        = 100;
    static constexpr int FILTER_CARD_H = 150;

    GeneratorFxCard (ViolentAudioProcessor& p, int generatorIdx, int fxSlot);
    ~GeneratorFxCard() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForType (FxType t);
    int preferredHeight() const noexcept;

private:
    ViolentAudioProcessor& processor;
    int generator, slot;

    juce::TextButton removeBtn { "X" };
    juce::Label      titleLabel;

    LabelledKnob driveKnob  { "Drive",  ViolentColours::red    };
    LabelledKnob toneKnob   { "Tone",   ViolentColours::yellow };
    LabelledKnob levelKnob  { "Level",  ViolentColours::green  };
    juce::ComboBox distTypeBox;

    LabelledKnob threshKnob  { "Thresh",  ViolentColours::red    };
    LabelledKnob ratioKnob   { "Ratio",   ViolentColours::yellow };
    LabelledKnob attackKnob  { "Attack",  ViolentColours::green  };
    LabelledKnob releaseKnob { "Release", ViolentColours::blue   };
    LabelledKnob makeupKnob  { "Makeup",  ViolentColours::accent };

    LabelledKnob roomKnob    { "Room",    ViolentColours::blue   };
    LabelledKnob dampingKnob { "Damping", ViolentColours::teal   };
    LabelledKnob wetKnob     { "Wet",     ViolentColours::accent };
    LabelledKnob widthKnob   { "Width",   ViolentColours::green  };

    FilterTypeSelector filterTypeSelector;
    LabelledKnob     filterCutoffKnob { "Cutoff",    ViolentColours::blue   };
    LabelledKnob     filterResKnob    { "Resonance", ViolentColours::accent };
    FilterResponseView filterResponseView;

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CA> distTypeAtt;

    void setAllInvisible();
    void layoutKnobs (std::initializer_list<LabelledKnob*>, juce::Rectangle<int>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorFxCard)
};

//==============================================================================
/** One MIDI modifier stage: sits before the generator, and is either a pitch
    shift (transpose+octave), a key shift (quantize-to-key), or a simple
    held-note arpeggiator. Added/removed and chained just like filters and
    effects, so a generator can stack several of these in any order. */
class GeneratorMidiRow : public juce::Component
{
public:
    static constexpr int ROW_H = 58;

    GeneratorMidiRow (ViolentAudioProcessor& p, int generatorIdx, int modSlot);

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    void showForType (MidiModType t);

private:
    ViolentAudioProcessor& processor;
    int generator, slot;

    TrashButton      removeBtn;
    juce::Label      sectionLabel;
    LabelledKnob     transposeKnob { "Transpose", ViolentColours::teal   };
    LabelledKnob     octaveKnob    { "Octave",    ViolentColours::blue   };
    juce::ComboBox   keyRootBox;
    juce::ComboBox   keyScaleBox;
    LabelledKnob     arpRateKnob  { "Arp Rate", ViolentColours::yellow };

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<CA> keyRootAtt, keyScaleAtt;

    void setAllInvisible();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorMidiRow)
};

//==============================================================================
/** One complete generator card: source + FX chain (filters are just another
    selectable effect type) + level/pan. */
class GeneratorCard : public juce::Component
{
public:
    static constexpr int SOURCE_H  = 150;  // source section height
    static constexpr int HEADER_H  = 36;
    static constexpr int FOOTER_H  = 44;
    static constexpr int ARROW_H   = 20;   // gap reserved for a routing arrow

    GeneratorCard (ViolentAudioProcessor& p, int generatorIdx);
    ~GeneratorCard() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    std::function<void()> onRemove;
    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // For the nav panel: current source-type label ("Synth"/"Sampler"), and
    // the FX-card component at a given slot (nullptr if empty).
    juce::String getSourceTypeLabel() const { return nameLabel.getText(); }
    juce::Component* getFxCard (int i) const { return fxCards[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    int generator;

    // Header
    TrashButton      removeBtn;
    juce::Label      nameLabel;
    juce::ToggleButton enableBtn;
    WaveformView     waveformView;

    // Source section
    juce::TextButton synthModeBtn   { "Synth" };
    juce::TextButton samplerModeBtn { "Sampler" };
    juce::ComboBox   srcTypeBox;
    juce::Label      srcTypeLabel;
    juce::TextButton loadSampleBtn { "Load..." };
    juce::Label      sampleFileLabel;
    int lastWaveformIndex = 0;

    LabelledKnob gainKnob    { "Gain",    ViolentColours::accent  };
    LabelledKnob octKnob     { "Oct",     ViolentColours::text    };
    LabelledKnob semiKnob    { "Semi",    ViolentColours::subtext };
    LabelledKnob detKnob     { "Detune",  ViolentColours::yellow  };
    LabelledKnob phaseKnob   { "Phase",   ViolentColours::teal    };
    LabelledKnob pwKnob      { "PW",      ViolentColours::blue    };
    LabelledKnob panKnob     { "Pan",     ViolentColours::accent  };
    LabelledKnob velKnob     { "Vel",     ViolentColours::green   };
    LabelledKnob uniKnob     { "Unison",  ViolentColours::red     };
    LabelledKnob uniSpreadKnob { "Spread",ViolentColours::yellow  };

    // ADSR box
    LabelledKnob attKnob { "Attack",  ViolentColours::green };
    LabelledKnob decKnob { "Decay",   ViolentColours::teal  };
    LabelledKnob susKnob { "Sustain", ViolentColours::blue  };
    LabelledKnob relKnob { "Release", ViolentColours::red   };

    // Footer: level + pan
    LabelledKnob levelKnob { "Level", ViolentColours::accent };
    LabelledKnob generatorPan { "Pan",   ViolentColours::yellow };

    // Sends to shared FX buses — one knob per bus slot; only the first
    // processor.numFxBuses of these are made visible/laid out.
    std::array<LabelledKnob, MAX_FX_BUSES> sendKnobs {
        LabelledKnob ("Send 1", ViolentColours::teal), LabelledKnob ("Send 2", ViolentColours::teal),
        LabelledKnob ("Send 3", ViolentColours::teal), LabelledKnob ("Send 4", ViolentColours::teal),
        LabelledKnob ("Send 5", ViolentColours::teal), LabelledKnob ("Send 6", ViolentColours::teal),
        LabelledKnob ("Send 7", ViolentColours::teal), LabelledKnob ("Send 8", ViolentColours::teal)
    };

    // FX chain
    std::array<std::unique_ptr<GeneratorFxCard>, MAX_GENERATOR_FX> fxCards;
    juce::TextButton addFxBtn { "+ Effect" };

    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<BA> enableAtt;
    std::unique_ptr<CA> srcTypeAtt;

    std::unique_ptr<juce::FileChooser> fileChooser;
    void openFilePicker();

    // Y-centre (local coords) of the routing arrow drawn between the ADSR
    // section and the FX chain, computed in resized() and used by paint().
    int effectArrowY = 0;
    void drawRoutingArrow (juce::Graphics&, int y) const;

    // Bounding box of the ADSR envelope, boxed and drawn in paint().
    juce::Rectangle<int> adsrBoxBounds;

    // Construct/wire an FX card in one place, so every call site (initial
    // load, "+" button, shift-after-remove) stays consistent — in
    // particular, so removal actually gets wired up every time.
    void addFxCard (int arrayIndex, FxType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorCard)
};

//==============================================================================
/** One generator's full vertical slot in the rack: a chain of MIDI modifier
    stages (each a pitch shift, key shift, or arpeggiator) — added/removed
    just like filters/effects, since a generator can have zero or several —
    plus the generator card itself, joined by a routing arrow whenever at
    least one modifier is present. */
class GeneratorUnit : public juce::Component
{
public:
    static constexpr int ARROW_H = 20;
    static constexpr int ADD_MIDI_BTN_H = 28;

    GeneratorUnit (ViolentAudioProcessor& p, int generatorIdx);

    void resized() override;
    void paint (juce::Graphics&) override;

    int preferredHeight() const noexcept;

    std::function<void()> onRemove;
    std::function<void()> onLayoutChanged;

    // For the nav panel.
    int getNumMidiMods() const noexcept { return processor.generators[(size_t) generator].numMidiMods; }
    juce::Component* getMidiRow (int i) const { return midiRows[(size_t) i].get(); }
    GeneratorCard& getCard() { return card; }

private:
    ViolentAudioProcessor& processor;
    int generator;

    std::array<std::unique_ptr<GeneratorMidiRow>, MAX_GENERATOR_MIDI_MODS> midiRows;
    juce::TextButton addMidiBtn { "+ MIDI Modifier" };
    GeneratorCard card;
    int arrowY = 0;

    void addMidiRow (int arrayIndex, MidiModType type);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorUnit)
};

//==============================================================================
/** Main panel: vertical list of GeneratorUnits. */
class GeneratorPanel : public juce::Component
{
public:
    explicit GeneratorPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (ViolentColours::background); }

    std::function<void()> onLayoutChanged;

    int preferredHeight() const noexcept;

    // Tears down and rebuilds every card from scratch, e.g. after a preset load
    // changes generator counts/modes/filters/fx behind the UI's back.
    void refreshFromState();

    // For the nav panel.
    int getNumUnits() const noexcept { return processor.numActiveGenerators; }
    GeneratorUnit* getUnit (int i) const { return units[(size_t) i].get(); }

private:
    ViolentAudioProcessor& processor;
    std::array<std::unique_ptr<GeneratorUnit>, MAX_GENERATORS> units;
    juce::TextButton addBtn { "+ Add Generator" };

    void rebuild (bool forceRecreate = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratorPanel)
};


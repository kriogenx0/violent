#pragma once
#include <JuceHeader.h>

static constexpr int MAX_SAMPLES = 4;

//==============================================================================
/** One loaded sample buffer + metadata, used by the unified oscillator/sampler
    source when a generator's source type is Sample. */
struct SampleModule
{
    juce::AudioBuffer<float> buffer;
    juce::String             filePath;
    bool                     hasData = false;
    juce::SpinLock           lock;

    SampleModule() = default;
    JUCE_DECLARE_NON_COPYABLE (SampleModule)
};

//==============================================================================
namespace Sampler
{
    /** Reads the next sample from `buffer` at `positionInOut` (a fractional
        sample-frame position) and advances it by the pitch ratio implied by
        `note` relative to a fixed root of C4 (MIDI note 60); wraps back to 0
        once playback runs past the end of the buffer. Returns silence if the
        buffer is empty or playback is past its end for this call. */
    float tick (const juce::AudioBuffer<float>& buffer, float& positionInOut, int note) noexcept;
}

//==============================================================================
/** Owns every generator's loaded sample buffer and the format manager used to
    decode files dropped/picked in the UI. */
class SamplerEngine
{
public:
    std::array<SampleModule, MAX_SAMPLES> modules;
    juce::AudioFormatManager             formatManager;

    void prepare() { formatManager.registerBasicFormats(); }

    /** Decodes `file` synchronously and stores it in slot `slotIndex`, replacing
        whatever was loaded there. Safe to call from the message thread while
        the audio thread may be reading the same slot (see SampleModule::lock). */
    void loadSample (int slotIndex, const juce::File& file);
};

#include "Sampler.h"

float Sampler::tick (const juce::AudioBuffer<float>& buffer, float& positionInOut, int note) noexcept
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0) return 0.0f;

    const double pitchRatio = std::pow (2.0, (note - 60) / 12.0);
    const float  sample     = positionInOut < (float) numSamples
                                 ? buffer.getSample (0, (int) positionInOut)
                                 : 0.0f;

    positionInOut += (float) pitchRatio;
    if (positionInOut >= (float) numSamples) positionInOut = 0.0f;

    return sample;
}

void SamplerEngine::loadSample (int slotIndex, const juce::File& file)
{
    auto reader = std::unique_ptr<juce::AudioFormatReader> (
        formatManager.createReaderFor (file));
    if (reader == nullptr) return;

    juce::AudioBuffer<float> tmp (static_cast<int> (reader->numChannels),
                                   static_cast<int> (reader->lengthInSamples));
    reader->read (&tmp, 0, static_cast<int> (reader->lengthInSamples), 0, true, true);

    {
        juce::SpinLock::ScopedLockType lock (modules[(size_t) slotIndex].lock);
        auto& sm    = modules[(size_t) slotIndex];
        sm.buffer   = std::move (tmp);
        sm.filePath = file.getFullPathName();
        sm.hasData  = true;
    }
}

#include "MidiModifier.h"

int MidiModifier::applyOne (const juce::AudioProcessorValueTreeState& apvts, int slot, MidiModType type, int note) noexcept
{
    int result = note;

    switch (type)
    {
        case MidiModType::PitchShift:
        {
            const int transpose = static_cast<int> (apvts.getRawParameterValue (ParamIDs::midiModTranspose (slot))->load());
            const int octave    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::midiModOctave (slot))->load());
            result += transpose + octave * 12;
            break;
        }
        case MidiModType::KeyShift:
        {
            const int root  = static_cast<int> (apvts.getRawParameterValue (ParamIDs::midiModKeyRoot (slot))->load());
            const int scale = static_cast<int> (apvts.getRawParameterValue (ParamIDs::midiModKeyScale (slot))->load());
            static constexpr int majorIntervals[] { 0, 2, 4, 5, 7, 9, 11 };
            static constexpr int minorIntervals[] { 0, 2, 3, 5, 7, 8, 10 };
            const auto& intervals = (scale == 0) ? majorIntervals : minorIntervals;

            const int pitchClass = ((result - root) % 12 + 12) % 12;
            int bestInterval = intervals[0], bestDist = 999;
            for (int iv : intervals)
            {
                const int dist = juce::jmin (std::abs (pitchClass - iv), 12 - std::abs (pitchClass - iv));
                if (dist < bestDist) { bestDist = dist; bestInterval = iv; }
            }
            result += (bestInterval - pitchClass);
            break;
        }
        case MidiModType::Arp:
            break;
    }

    return juce::jlimit (0, 127, result);
}

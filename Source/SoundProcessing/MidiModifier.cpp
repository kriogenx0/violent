#include "MidiModifier.h"

int MidiModifier::apply (const juce::AudioProcessorValueTreeState& apvts, int generatorIdx, int note,
                          int numMods, const std::array<MidiModType, MAX_GENERATOR_MIDI_MODS>& types) noexcept
{
    int result = note;

    for (int m = 0; m < numMods; ++m)
    {
        switch (types[(size_t) m])
        {
            case MidiModType::PitchShift:
            {
                const int transpose = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genMidiTranspose (generatorIdx, m))->load());
                const int octave    = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genMidiOctave (generatorIdx, m))->load());
                result += transpose + octave * 12;
                break;
            }
            case MidiModType::KeyShift:
            {
                const int root  = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genMidiKeyRoot (generatorIdx, m))->load());
                const int scale = static_cast<int> (apvts.getRawParameterValue (ParamIDs::genMidiKeyScale (generatorIdx, m))->load());
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
    }

    return juce::jlimit (0, 127, result);
}

bool MidiModifier::hasArp (int numMods, const std::array<MidiModType, MAX_GENERATOR_MIDI_MODS>& types) noexcept
{
    return firstArpSlot (numMods, types) >= 0;
}

int MidiModifier::firstArpSlot (int numMods, const std::array<MidiModType, MAX_GENERATOR_MIDI_MODS>& types) noexcept
{
    for (int m = 0; m < numMods; ++m)
        if (types[(size_t) m] == MidiModType::Arp)
            return m;
    return -1;
}

#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace
{

class WaveformCaptureTests : public juce::UnitTest
{
public:
    WaveformCaptureTests() : juce::UnitTest ("Waveform capture", "Processor") {}

    void runTest() override
    {
        constexpr double sampleRate = 48000.0;
        constexpr int    blockSize  = 512;

        ViolentAudioProcessor proc;
        proc.setPlayConfigDetails (0, 2, sampleRate, blockSize);
        proc.prepareToPlay (sampleRate, blockSize);

        beginTest ("Ring buffer is sized to hold the largest window option (1s)");
        {
            const int expected = (int) sampleRate; // 1000ms of history at 48kHz
            expectEquals (proc.waveformRingSize, expected);
            expectEquals ((int) proc.waveformRing[0].size(), expected);
        }

        beginTest ("Processing a block advances the write position by its sample count");
        {
            juce::AudioBuffer<float> buffer (2, blockSize);
            juce::MidiBuffer midi;
            midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);

            proc.processBlock (buffer, midi);

            expectEquals (proc.waveformRingWritePos[0], blockSize);

            // A note is sounding, so the generator's raw waveform should have
            // actually been captured rather than left at its initial silence.
            bool anyNonZero = false;
            for (int i = 0; i < blockSize; ++i)
                if (proc.waveformRing[0][(size_t) i] != 0.0f) { anyNonZero = true; break; }
            expect (anyNonZero);
        }

        beginTest ("The write position wraps around once the ring fills up");
        {
            const int blocksToWrap = proc.waveformRingSize / blockSize + 2;
            juce::MidiBuffer empty;
            for (int b = 0; b < blocksToWrap; ++b)
            {
                juce::AudioBuffer<float> buffer (2, blockSize);
                proc.processBlock (buffer, empty);
            }

            expect (proc.waveformRingWritePos[0] >= 0
                        && proc.waveformRingWritePos[0] < proc.waveformRingSize);
        }

        beginTest ("lastMidiActivityMs is stamped by genuinely inbound MIDI, not by the preview player's synthetic notes");
        {
            // A fresh instance, rather than the shared proc above, so the
            // "never touched yet" starting state (0) is reliable regardless
            // of what earlier tests in this file already fed it.
            ViolentAudioProcessor freshProc;
            freshProc.setPlayConfigDetails (0, 2, sampleRate, blockSize);
            freshProc.prepareToPlay (sampleRate, blockSize);
            expectEquals ((juce::int64) freshProc.lastMidiActivityMs.load(), (juce::int64) 0);

            juce::AudioBuffer<float> buffer (2, blockSize);
            juce::MidiBuffer empty;
            freshProc.previewActive.store (true, std::memory_order_relaxed);
            freshProc.processBlock (buffer, empty); // preview-only block, no real inbound MIDI

            expectEquals ((juce::int64) freshProc.lastMidiActivityMs.load(), (juce::int64) 0,
                    "the preview player's synthetic notes must not register as inbound MIDI activity");

            juce::MidiBuffer realNoteOn;
            realNoteOn.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);
            freshProc.processBlock (buffer, realNoteOn);

            expect (freshProc.lastMidiActivityMs.load() > 0,
                    "a genuinely inbound MIDI message should stamp the activity light's timestamp");
        }
    }
};

static WaveformCaptureTests waveformCaptureTests;

} // namespace

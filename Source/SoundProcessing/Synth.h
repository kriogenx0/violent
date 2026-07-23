#pragma once
#include <JuceHeader.h>

static constexpr int MAX_UNISON = 8;

//==============================================================================
/** Phase-accumulator oscillator with 5 waveforms + PWM.
 *  waveform: 0=Sine  1=Saw  2=Square/PWM  3=Triangle  4=Noise */
struct OscillatorGen
{
    float phase = 0.0f;
    juce::Random rng;

    void reset (float phaseOffset = 0.0f) noexcept { phase = phaseOffset; }

    float tick (float frequencyHz, float detuneCents,
                float sampleRate, int waveform, float pulseWidth = 0.5f) noexcept
    {
        const float freq = frequencyHz * std::pow (2.0f, detuneCents / 1200.0f);
        phase += freq / sampleRate;
        if (phase >= 1.0f) phase -= std::floor (phase);

        switch (waveform)
        {
            case 0:  return std::sin (phase * juce::MathConstants<float>::twoPi);
            case 1:  return 2.0f * phase - 1.0f;
            case 2:  return phase < pulseWidth ? 1.0f : -1.0f;
            case 3:  return phase < 0.5f ? (4.0f * phase - 1.0f)
                                         : (3.0f - 4.0f * phase);
            case 4:  return rng.nextFloat() * 2.0f - 1.0f;
            default: return 0.0f;
        }
    }
};

//==============================================================================
/** Simple ADSR envelope. */
struct ADSREnv
{
    juce::ADSR env;

    void setSampleRate (double sr)                     noexcept { env.setSampleRate (sr); }
    void setParams (float a, float d, float s, float r) noexcept
    {
        juce::ADSR::Parameters p;
        p.attack = a; p.decay = d; p.sustain = s; p.release = r;
        env.setParameters (p);
    }
    float tick()          noexcept { return env.getNextSample(); }
    void  noteOn()        noexcept { env.noteOn(); }
    void  noteOff()       noexcept { env.noteOff(); }
    bool  isActive() const noexcept { return env.isActive(); }
    void  reset()          noexcept { env.reset(); }
};

//==============================================================================
/** Per-generator oscillator settings, refreshed each block from APVTS (see
    ViolentAudioProcessor::loadGeneratorParams). */
struct OscSlot
{
    bool  en = false;
    int   type = 1, octave = 0, semitone = 0, unisonVoices = 1;
    float gainLin = 1.0f, detune = 0.0f, phase = 0.0f, pulseWidth = 0.5f;
    float pan = 0.0f, velSens = 1.0f, unisonSpread = 15.0f;
    float att = 0.01f, dec = 0.1f, sus = 0.7f, rel = 0.3f;
};

//==============================================================================
/** One polyphonic voice: unison oscillators + envelope, tagged by generator.
    Shared by both the oscillator and sample-playback source paths — see
    Sampler.h, which repurposes osc[u].phase as a fractional sample-buffer
    position when the owning generator's source type is Sample. */
struct SynthVoice
{
    bool  active      = false;
    int   note        = -1;
    int   generatorIdx = 0;
    float velocity    = 1.0f;
    float baseFreqHz  = 0.0f;

    OscillatorGen osc[MAX_UNISON];
    ADSREnv       adsr;

    void prepare (double sr) noexcept { adsr.setSampleRate (sr); }
    void reset()  noexcept
    {
        active = false; note = -1;
        for (auto& o : osc) o.reset();
        adsr.reset();
    }
    bool isActive() const noexcept { return adsr.isActive(); }
};

//==============================================================================
/** Fixed voice pool shared across all generators; each voice's generatorIdx
    tags which generator it belongs to for allocation and note-off routing.
    Per-sample oscillator ticking stays in ViolentAudioProcessor::renderGenerator
    (interleaved there with sample playback, since a voice doesn't know its
    own source type ahead of render time) — this class owns voice lifecycle:
    which voices are free/active/stealable, and starting/stopping them. */
class SynthEngine
{
public:
    static constexpr int MAX_VOICES = 16;
    std::array<SynthVoice, MAX_VOICES> voices;

    void prepare (double sampleRate) noexcept
    {
        for (auto& v : voices) v.prepare (sampleRate);
    }

    int findFreeVoice (int generatorIdx) const noexcept
    {
        for (int i = 0; i < MAX_VOICES; ++i)
            if (!voices[(size_t) i].active && voices[(size_t) i].generatorIdx == generatorIdx)
                return i;
        // Also accept unassigned (default generatorIdx == 0)
        for (int i = 0; i < MAX_VOICES; ++i)
            if (!voices[(size_t) i].active) return i;
        return -1;
    }

    int findVoiceToSteal (int generatorIdx) const noexcept
    {
        // Steal the oldest released voice from this generator, else any
        for (int i = 0; i < MAX_VOICES; ++i)
            if (voices[(size_t) i].generatorIdx == generatorIdx && !voices[(size_t) i].isActive())
                return i;
        return 0;
    }

    void startVoice (int voiceIndex, int note, float velocity, int generatorIdx, const OscSlot& osc) noexcept
    {
        auto& v        = voices[(size_t) voiceIndex];
        v.note         = note;
        v.generatorIdx = generatorIdx;
        v.velocity     = velocity;
        v.baseFreqHz   = static_cast<float> (juce::MidiMessage::getMidiNoteInHertz (note));
        v.active       = true;

        for (auto& o : v.osc) o.reset (osc.phase);
        v.adsr.setParams (osc.att, osc.dec, osc.sus, osc.rel);
        v.adsr.noteOn();
    }

    void stopNote (int note) noexcept
    {
        for (auto& v : voices)
            if (v.active && v.note == note)
                v.adsr.noteOff();
    }

    void stopNoteForGenerator (int generatorIdx, int note) noexcept
    {
        for (auto& v : voices)
            if (v.active && v.generatorIdx == generatorIdx && v.note == note)
                v.adsr.noteOff();
    }

    void allNotesOff() noexcept
    {
        for (auto& v : voices) v.reset();
    }
};

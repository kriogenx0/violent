#pragma once
#include <JuceHeader.h>

static constexpr int MAX_SYNTHS  = 16;
static constexpr int MAX_SAMPLES = 4;
static constexpr int MAX_FILTERS = 4;
static constexpr int MAX_LFOS    = 4;
static constexpr int MAX_VOICES  = 16;

//==============================================================================
/** Sample playback engine with fractional-position pitch-shifting. */
struct SamplePlayer
{
    double playPos = 0.0;

    void reset() noexcept { playPos = 0.0; }

    /** Returns a mono-averaged sample with linear interpolation.
     *  pitchRatio = 2^((note - rootNote) / 12) */
    float tick (const juce::AudioBuffer<float>& buf,
                double pitchRatio, bool loop) noexcept
    {
        const int numSamples = buf.getNumSamples();
        if (numSamples == 0) return 0.0f;

        if ((int) playPos >= numSamples)
        {
            if (loop) playPos = std::fmod (playPos, static_cast<double> (numSamples));
            else      return 0.0f;
        }

        const int    pos0 = (int) playPos;
        const double frac = playPos - pos0;
        const int    pos1 = std::min (pos0 + 1, numSamples - 1);

        float out = 0.0f;
        const int numCh = buf.getNumChannels();
        for (int ch = 0; ch < numCh; ++ch)
            out += buf.getSample (ch, pos0) * (float)(1.0 - frac)
                 + buf.getSample (ch, pos1) * (float) frac;
        out /= static_cast<float> (numCh);

        playPos += pitchRatio;
        return out;
    }

    bool isFinished (int numSamples, bool loop) const noexcept
    {
        return !loop && numSamples > 0 && (int) playPos >= numSamples;
    }
};

//==============================================================================
/** Simple ADSR envelope */
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
/** Chamberlin two-pole state-variable filter (stereo, 2 channels).
 *  type: 0=LP  1=HP  2=BP  3=Notch */
struct SVFilter
{
    float low[2]  = {};
    float band[2] = {};
    float f_coeff = 0.0f, q_coeff = 0.0f;

    void reset() noexcept
    {
        for (auto& v : low)  v = 0.0f;
        for (auto& v : band) v = 0.0f;
    }

    void setCoefficients (float cutoffHz, float resonance, float sampleRate) noexcept
    {
        cutoffHz  = juce::jlimit (20.0f, sampleRate * 0.49f, cutoffHz);
        f_coeff   = 2.0f * std::sin (juce::MathConstants<float>::pi * cutoffHz / sampleRate);
        q_coeff   = 1.0f / juce::jmax (0.1f, resonance);
    }

    float process (int ch, float x, int type) noexcept
    {
        low[ch]        += f_coeff * band[ch];
        const float hi  = x - low[ch] - q_coeff * band[ch];
        band[ch]        = f_coeff * hi + band[ch];

        switch (type)
        {
            case 0:  return low[ch];
            case 1:  return hi;
            case 2:  return band[ch];
            case 3:  return hi + low[ch];
            default: return low[ch];
        }
    }
};

//==============================================================================
/** LFO oscillator.
 *  shape: 0=Sine  1=Triangle  2=Saw  3=Square */
struct LFOGen
{
    double phase = 0.0;

    void reset() noexcept { phase = 0.0; }

    float tick (float rateHz, float sampleRate, int shape) noexcept
    {
        phase += static_cast<double> (rateHz) / static_cast<double> (sampleRate);
        if (phase >= 1.0) phase -= 1.0;
        const float p = static_cast<float> (phase);

        switch (shape)
        {
            case 0:  return std::sin (p * juce::MathConstants<float>::twoPi);
            case 1:  return p < 0.5f ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
            case 2:  return 2.0f * p - 1.0f;
            case 3:  return p < 0.5f ? 1.0f : -1.0f;
            default: return 0.0f;
        }
    }
};

//==============================================================================
static constexpr int MAX_UNISON = 8;

//==============================================================================
/** One polyphonic voice: MAX_SYNTHS oscillators + envelopes. */
struct Voice
{
    bool  active     = false;
    int   note       = -1;
    float velocity   = 1.0f;
    float baseFreqHz = 0.0f;

    OscillatorGen osc [MAX_SYNTHS][MAX_UNISON];
    ADSREnv       adsr[MAX_SYNTHS];

    // Sample playback state
    SamplePlayer  smpPlayer[MAX_SAMPLES];
    ADSREnv       smpAdsr  [MAX_SAMPLES];

    // Per-block LFO modulation (written by processor, read during render)
    float detuneModCents[MAX_SYNTHS] = {};
    float volumeModMult [MAX_SYNTHS] = {};

    void prepare (double sampleRate) noexcept
    {
        for (auto& a : adsr)    a.setSampleRate (sampleRate);
        for (auto& a : smpAdsr) a.setSampleRate (sampleRate);
    }

    void reset() noexcept
    {
        active = false;
        note   = -1;
        for (auto& row : osc)       for (auto& o : row) o.reset();
        for (auto& a : adsr)        a.reset();
        for (auto& sp : smpPlayer)  sp.reset();
        for (auto& a : smpAdsr)     a.reset();
        for (auto& v : detuneModCents) v = 0.0f;
        for (auto& v : volumeModMult)  v = 0.0f;
    }

    bool isActive() const noexcept
    {
        for (const auto& a : adsr)    if (a.isActive()) return true;
        for (const auto& a : smpAdsr) if (a.isActive()) return true;
        return false;
    }
};

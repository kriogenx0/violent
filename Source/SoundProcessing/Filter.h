#pragma once
#include <JuceHeader.h>

//==============================================================================
/** Wraps JUCE's TPT state-variable filter — unconditionally stable at any
    cutoff/sample-rate ratio — with the LP/HP/BP/Notch type mapping used
    throughout Violent (0=LP 1=HP 2=BP 3=Notch); "Notch" is synthesised as
    lowpass+highpass since StateVariableTPTFilterType has no notch mode of
    its own. Shared by master filters, per-generator Filter-type FX slots,
    and shared FX buses — all three used to duplicate this logic inline. */
struct TPTFilterProcessor
{
    juce::dsp::StateVariableTPTFilter<float> filter, notchHelper;
    bool isNotch = false;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        filter.prepare (spec);
        notchHelper.prepare (spec);
    }

    void reset()
    {
        filter.reset();
        notchHelper.reset();
    }

    /** type: 0=LP 1=HP 2=BP 3=Notch. Call once per block before processSample(). */
    void setParams (int type, float cutoffHz, float resonance)
    {
        using FType = juce::dsp::StateVariableTPTFilterType;
        isNotch = (type == 3);

        filter.setType (isNotch ? FType::lowpass
                                 : (type == 1 ? FType::highpass
                                              : (type == 2 ? FType::bandpass : FType::lowpass)));
        filter.setCutoffFrequency (cutoffHz);
        filter.setResonance (resonance);

        if (isNotch)
        {
            notchHelper.setType (FType::highpass);
            notchHelper.setCutoffFrequency (cutoffHz);
            notchHelper.setResonance (resonance);
        }
    }

    float processSample (int channel, float x) noexcept
    {
        float v = filter.processSample (channel, x);
        if (isNotch) v += notchHelper.processSample (channel, x);
        return v;
    }
};

//==============================================================================
/** One master filter's DSP state — master filters are applied last, after
    every generator's own FX chain, to a chosen subset of summed generators
    (see ViolentAudioProcessor::mixGeneratorsToMaster for the routing/summing). */
struct MasterFilterDSP
{
    TPTFilterProcessor proc;

    void prepare (const juce::dsp::ProcessSpec& spec) { proc.prepare (spec); }
    void reset() { proc.reset(); }
};

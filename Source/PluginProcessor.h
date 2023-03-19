/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class THICCAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    THICCAudioProcessor();
    ~THICCAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    float* getInputPeakArr();
    float* getOutputPeakArr();

    int getVolArraySize();

private:
    //value tree params
    juce::AudioProcessorValueTreeState parameters;

    std::atomic<float>* driveParameter = nullptr;
    std::atomic<float>* outputGainPreParameter = nullptr;
    std::atomic<float>* softnessParameter = nullptr;
    std::atomic<float>* kneeParameter = nullptr;
    
    std::atomic<float>* inputGainParameter = nullptr;
    std::atomic<float>* outputGainPostParameter = nullptr;
    std::atomic<float>* lowPassParameter = nullptr;
    std::atomic<float>* highPassParameter = nullptr;
    std::atomic<float>* mixParameter = nullptr;

    std::atomic<float>* clipParameter = nullptr;
    std::atomic<float>* peakParameter = nullptr;

    std::atomic<float>* typeParameter = nullptr;
    std::atomic<float>* evenParameter = nullptr;

    std::atomic<float>* OSFactorParameter = nullptr;

    //filters
    juce::dsp::LinkwitzRileyFilter<float> highPassHigh;
    juce::dsp::LinkwitzRileyFilter<float> highPassHigh2;
    juce::dsp::LinkwitzRileyFilter<float> lowPassLow;
    juce::dsp::LinkwitzRileyFilter<float> lowPassLow2;
    juce::dsp::LinkwitzRileyFilter<float> highPassMid;
    juce::dsp::LinkwitzRileyFilter<float> lowPassMid;

    //dc blocker variables
    float* dcOutput;
    float* dcInput;

    float* dcOutput2;
    float* dcInput2;

    //volume display vars
    int volCounter;
    int volCounterMax;

    float* inputPeakArr;
    float* outputPeakArr;

    float inputPeak = 0;
    float outputPeak = 0;

    //oversampling
    std::unique_ptr<juce::dsp::Oversampling<float>> OSProcessor[5];

    int OSFactor;

    bool resetFlag;

    //==============================================================================
    float waveShape(float sampleData, int type, float softness, float knee, bool clipPeaks);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (THICCAudioProcessor)
};

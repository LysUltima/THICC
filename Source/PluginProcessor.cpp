/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//constants
#define VOL_ARRAY_SIZE 256
#define BIAS 0.4f

//==============================================================================
THICCAudioProcessor::THICCAudioProcessor()
    : parameters(*this, nullptr, juce::Identifier("THICC"), {
        std::make_unique<juce::AudioParameterFloat>("drive", "Drive", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>("outputGainPre", "Gain (pre-mix)", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>("softness", "Softness", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 100.0f),
        std::make_unique<juce::AudioParameterFloat>("knee", "Knee", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>("inputGain", "Input Gain", juce::NormalisableRange<float>(-30.0f, 30.0f, 0.1f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>("outputGainPost", "Output Gain", juce::NormalisableRange<float>(-30.0f, 30.0f, 0.1f), 0.0f),
        std::make_unique<juce::AudioParameterFloat>("lowPass", "Low Pass", juce::NormalisableRange<float>(40.0f, 20000.0f, 1.0f, 0.3f), 20000.0f),
        std::make_unique<juce::AudioParameterFloat>("highPass", "High Pass", juce::NormalisableRange<float>(20.0f, 15000.0f, 1.0f, 0.3f), 20.0f),
        std::make_unique<juce::AudioParameterFloat>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 0.5f), 100.0f),
        std::make_unique<juce::AudioParameterBool>("clip", "Clip Mode", true),
        std::make_unique<juce::AudioParameterBool>("peak", "Hard Clip Peaks", false),
        std::make_unique<juce::AudioParameterFloat>("type", "Saturation Type", juce::NormalisableRange<float>(1.0f, 4.0f, 1.0f), 1.0f),
        std::make_unique<juce::AudioParameterBool>("even", "Even Harmonics", false),
        std::make_unique<juce::AudioParameterFloat>("OSFactor", "Oversampling Factor", juce::NormalisableRange<float>(1.0f, 5.0f, 1.0f), 2.0f)
    })
#ifndef JucePlugin_PreferredChannelConfigurations
     , AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    driveParameter = parameters.getRawParameterValue("drive");
    outputGainPreParameter = parameters.getRawParameterValue("outputGainPre");
    softnessParameter = parameters.getRawParameterValue("softness");
    kneeParameter = parameters.getRawParameterValue("knee");
    inputGainParameter = parameters.getRawParameterValue("inputGain");
    outputGainPostParameter = parameters.getRawParameterValue("outputGainPost");
    lowPassParameter = parameters.getRawParameterValue("lowPass");
    highPassParameter = parameters.getRawParameterValue("highPass");
    mixParameter = parameters.getRawParameterValue("mix");
    clipParameter = parameters.getRawParameterValue("clip");
    peakParameter = parameters.getRawParameterValue("peak");
    typeParameter = parameters.getRawParameterValue("type");
    evenParameter = parameters.getRawParameterValue("even");
    OSFactorParameter = parameters.getRawParameterValue("OSFactor");

    highPassHigh = juce::dsp::LinkwitzRileyFilter<float>();
    highPassHigh2 = juce::dsp::LinkwitzRileyFilter<float>();
    lowPassLow = juce::dsp::LinkwitzRileyFilter<float>();
    lowPassLow2 = juce::dsp::LinkwitzRileyFilter<float>();
    highPassMid = juce::dsp::LinkwitzRileyFilter<float>();
    lowPassMid = juce::dsp::LinkwitzRileyFilter<float>();

    highPassHigh.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    highPassHigh2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    lowPassLow.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    lowPassLow2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    highPassMid.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    lowPassMid.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);

    int totalNumInputChannels = getTotalNumInputChannels();

    dcOutput = new float[totalNumInputChannels];
    dcInput = new float[totalNumInputChannels];
    dcOutput2 = new float[totalNumInputChannels];
    dcInput2 = new float[totalNumInputChannels];

    inputPeakArr = new float[VOL_ARRAY_SIZE];
    outputPeakArr = new float[VOL_ARRAY_SIZE];

    for (int i = 0; i < 5; i++) {
        OSProcessor[i] = std::make_unique<juce::dsp::Oversampling<float>>(totalNumInputChannels, i, juce::dsp::Oversampling<float>::FilterType::filterHalfBandPolyphaseIIR);
    }

    resetFlag = true;
}

THICCAudioProcessor::~THICCAudioProcessor()
{
    delete[] dcInput;
    delete[] dcOutput;
    delete[] dcInput2;
    delete[] dcOutput2;
    delete[] inputPeakArr;
    delete[] outputPeakArr;
}

//==============================================================================
const juce::String THICCAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool THICCAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool THICCAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool THICCAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double THICCAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int THICCAudioProcessor::getNumPrograms()
{
    return 1;
}

int THICCAudioProcessor::getCurrentProgram()
{
    return 0;
}

void THICCAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String THICCAudioProcessor::getProgramName (int index)
{
    return {};
}

void THICCAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void THICCAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto totalNumInputChannels = getTotalNumInputChannels();
    OSFactor = static_cast<int>(std::roundf(*OSFactorParameter)) - 1;

    struct juce::dsp::ProcessSpec processSpec = { static_cast<juce::uint32>(sampleRate * static_cast<int>(std::roundf(powf(2, OSFactor)))), 
        static_cast<juce::uint32>(samplesPerBlock * static_cast<int>(std::roundf(powf(2, OSFactor)))), static_cast<juce::uint32>(totalNumInputChannels) };

    highPassHigh.prepare(processSpec);
    highPassHigh2.prepare(processSpec);
    lowPassLow.prepare(processSpec);
    lowPassLow2.prepare(processSpec);
    highPassMid.prepare(processSpec);
    lowPassMid.prepare(processSpec);

    for (int i = 0; i < VOL_ARRAY_SIZE; i++) {
        inputPeakArr[i] = -96.0f;
        outputPeakArr[i] = -96.0f;
    }

    
    for (int i = 0; i < totalNumInputChannels; i++) {
        dcOutput[i] = 0;
        dcInput[i] = 0;

        dcOutput2[i] = 0;
        dcInput2[i] = 0;
    }

    volCounterMax = 2048 / samplesPerBlock;
    volCounter = 0;

    OSProcessor[OSFactor]->initProcessing(samplesPerBlock);
}

void THICCAudioProcessor::releaseResources()
{
    highPassHigh.reset();
    highPassHigh2.reset();
    lowPassLow.reset();
    lowPassLow2.reset();
    highPassMid.reset();
    lowPassMid.reset();
    inputPeak = outputPeak = 0;

    OSProcessor[OSFactor]->reset();

    for (int i = 0; i < getTotalNumInputChannels(); i++) {
        dcOutput[i] = 0;
        dcInput[i] = 0;

        dcOutput2[i] = 0;
        dcInput2[i] = 0;
    }

    resetFlag = false;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool THICCAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void THICCAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    int totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; i++)
        buffer.clear (i, 0, buffer.getNumSamples());

    //apply input gain
    buffer.applyGain(powf(10, *inputGainParameter * 0.05f));

    if (buffer.getMagnitude(0, buffer.getNumSamples()) > 0.00003f) { //if peak is under -90.4575749 dBFS, do not process

        resetFlag = true;

        //oversamples
        auto audioBlock = juce::dsp::AudioBlock<float>(buffer);
        auto OSAudioBlock = OSProcessor[OSFactor]->processSamplesUp(audioBlock);

        //waveshape vars
        int waveType = static_cast<int>(std::roundf(*typeParameter));
        float knee = 1 - (*kneeParameter / 100);
        float softness = *softnessParameter / 100;
        bool clipPeaks = *peakParameter > 0.5f;
        bool isEven = *evenParameter > 0.5f;

        //eq vars
        bool isFilterOn = *highPassParameter != 20.0f || *lowPassParameter != 20000.0f;

        //processes samples
        for (int channel = 0; channel < totalNumInputChannels; channel++)
        {
            for (int sample = 0; sample < OSAudioBlock.getNumSamples(); sample++) {
                //get sample data
                float sampleData = OSAudioBlock.getSample(channel, sample);

                //eq vars
                float otherBand1 = 0;
                float otherBand2 = 0;
                float otherBand3 = 0;
                float mainBand = 0;

                //save original for mix
                float originalSampleData = sampleData;

                //input peak
                if (abs(sampleData) > inputPeak) {
                    inputPeak = abs(sampleData);
                }

                if (isFilterOn) {
                    if (*highPassParameter < 21.0f) { //only low pass on
                        lowPassLow.setCutoffFrequency(*lowPassParameter);
                        highPassHigh.setCutoffFrequency(*lowPassParameter);

                        otherBand1 = highPassHigh.processSample(channel, sampleData); //high band

                        mainBand = lowPassLow.processSample(channel, sampleData); //low band
                    }
                    else if (*lowPassParameter > 19999.0f) { //only high pass on
                        lowPassLow.setCutoffFrequency(*highPassParameter);
                        highPassHigh.setCutoffFrequency(*highPassParameter);

                        otherBand1 = lowPassLow.processSample(channel, sampleData); //low band

                        mainBand = highPassHigh.processSample(channel, sampleData); //high band
                    }
                    else { //3 band split
                        lowPassLow.setCutoffFrequency(*highPassParameter);
                        lowPassLow2.setCutoffFrequency(*lowPassParameter);
                        highPassHigh.setCutoffFrequency(*lowPassParameter);
                        highPassHigh2.setCutoffFrequency(*lowPassParameter);
                        highPassMid.setCutoffFrequency(*highPassParameter);
                        lowPassMid.setCutoffFrequency(*lowPassParameter);

                        //low band
                        otherBand1 = lowPassLow.processSample(channel, sampleData);
                        otherBand3 = highPassHigh2.processSample(channel, otherBand1);

                        otherBand1 = lowPassLow2.processSample(channel, otherBand1);
                        //mid band
                        mainBand = highPassMid.processSample(channel, sampleData);
                        //high band
                        otherBand2 = highPassHigh.processSample(channel, mainBand);

                        mainBand = lowPassMid.processSample(channel, mainBand);
                    }

                    sampleData = mainBand;
                }

                //apply drive
                sampleData *= powf(10, *driveParameter * 0.05f);

                if (*clipParameter < 0.5f) { //saturation
                    //apply bias
                    sampleData += BIAS;
                    sampleData = waveShape(sampleData, waveType, softness, 0, clipPeaks);
                    sampleData -= BIAS;

                    if (!isEven) { //symmetric
                        sampleData = waveShape(sampleData, waveType, softness, 0, clipPeaks);
                    }

                    //dc blocker
                    dcOutput[channel] = sampleData - dcInput[channel] + 0.999f * dcOutput[channel];
                    dcInput[channel] = sampleData;
                    sampleData = dcOutput[channel];
                }
                else { //clipping
                    //waveshape
                    if (!isEven) { //symmetric
                        sampleData = waveShape(sampleData, waveType, softness, 0, clipPeaks);
                    }
                    else { //asymmetric
                        sampleData = sampleData > 0 ? waveShape(sampleData, waveType, softness, 0, clipPeaks) : sampleData;

                        //dc blocker
                        dcOutput[channel] = sampleData - dcInput[channel] + 0.999f * dcOutput[channel];
                        dcInput[channel] = sampleData;
                        sampleData = dcOutput[channel];
                    }
                }

                //tanh 2nd stage
                sampleData = waveShape(sampleData, 4, 1, knee, clipPeaks);

                //reverse drive
                sampleData /= powf(10, *driveParameter * 0.05f);

                //apply output gain pre-mix
                sampleData *= powf(10, *outputGainPreParameter * 0.05f);

                if (isFilterOn) { //adds the other bands back in and apply mix
                    sampleData = sampleData * (*mixParameter / 100) + mainBand * (1 - (*mixParameter / 100)) + otherBand1 + otherBand2 + otherBand3;
                }
                else {
                    //apply mix
                    sampleData = sampleData * (*mixParameter / 100) + originalSampleData * (1 - (*mixParameter / 100));
                }

                //output peak
                if (abs(sampleData) > outputPeak) {
                    outputPeak = abs(sampleData);
                }

                //dc blocker
                dcOutput2[channel] = sampleData - dcInput2[channel] + 0.999f * dcOutput2[channel];
                dcInput2[channel] = sampleData;
                sampleData = dcOutput2[channel];

                //set sample data
                OSAudioBlock.setSample(channel, sample, sampleData);
            }
        }

        OSProcessor[OSFactor]->processSamplesDown(audioBlock);

        
    }
    else if (resetFlag) {
        releaseResources();
    }

    if (++volCounter >= volCounterMax) {
        for (int i = VOL_ARRAY_SIZE - 1; i > 0; i--) {
            inputPeakArr[i] = inputPeakArr[i - 1];
            outputPeakArr[i] = outputPeakArr[i - 1];
        }
        inputPeakArr[0] = fmaxf(20.0f * log10(inputPeak), -96.0f) / 18.0f;
        outputPeakArr[0] = fmaxf(20.0f * log10(outputPeak), -96.0f) / 18.0f;
        inputPeak = outputPeak = 0;
        volCounter = 0;
    }

    //apply output gain
    buffer.applyGain(powf(10, *outputGainPostParameter * 0.05f));
}

//==============================================================================
bool THICCAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* THICCAudioProcessor::createEditor()
{
    return new THICCAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void THICCAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void THICCAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================

//input x, return function(x), where sampleData is x and type is the type of function to use.
float THICCAudioProcessor::waveShape(float sampleData, int type, float softness, float knee, bool clipPeaks) {
    bool isPositive = sampleData > 0;
    float waveShapedData = abs(sampleData);
    switch (type) {
        case 1: //sin(x)
            if (clipPeaks && waveShapedData >= (asin(1-knee) + knee)) {
                waveShapedData = 1;
            }
            else if (waveShapedData > knee) {
                waveShapedData = sin(waveShapedData - knee) + knee;
            }
            break; 
        case 2: // -e^-x + 1
            if (waveShapedData > knee) {
                waveShapedData = -1 * exp((waveShapedData - knee) * -1) + knee + 1;
            }
            break;
        case 3: // ln(x+1)
            if (waveShapedData > knee) {
                waveShapedData = log(waveShapedData - knee + 1) + knee;
            }
            break;
        case 4: //tanh(x)
            if (waveShapedData > knee) {
                waveShapedData = tanh(waveShapedData - knee) + knee;
            }
            break;
        default:
            waveShapedData = 0;
    }
    if (clipPeaks) {
        waveShapedData = fminf(waveShapedData, 1);
    }
    
    waveShapedData = softness * waveShapedData + (1 - softness) * fminf(abs(sampleData), 1);

    if (!isPositive) {
        waveShapedData *= -1;
    }

    return waveShapedData;
}

//returns an array of the peak values of the input audio
float* THICCAudioProcessor::getInputPeakArr() {
    return inputPeakArr;
}

//returns an array of the peak values of the output audio
float* THICCAudioProcessor::getOutputPeakArr() {
    return outputPeakArr;
}

//returns the array size of the peak volume data array
int THICCAudioProcessor::getVolArraySize() {
    return VOL_ARRAY_SIZE;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new THICCAudioProcessor();
}

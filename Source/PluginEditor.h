/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class THICCAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    THICCAudioProcessorEditor (THICCAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~THICCAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    THICCAudioProcessor& audioProcessor;
    
    //params
    juce::AudioProcessorValueTreeState& valueTreeState;

    //sliders
    juce::Slider driveSlider;
    juce::Slider outputGainPreSlider;
    juce::Slider softnessSlider;
    juce::Slider kneeSlider;
    juce::Slider inputGainSlider;
    juce::Slider outputGainPostSlider;
    juce::Slider lowPassSlider;
    juce::Slider highPassSlider;
    juce::Slider mixSlider;

    //buttons
    juce::TextButton clipButton;
    juce::TextButton peakButton;

    juce::TextButton evenButton;

    juce::TextButton displayButton;

    //combo boxes
    juce::ComboBox typeBox;
    juce::ComboBox OSFactorBox;

    //slider attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainPreAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> softnessAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainPostAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowPassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highPassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    //button attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> clipAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> peakAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> evenAttachment;

    //box attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> OSFactorAttachment;

    //waveform vars
    juce::Rectangle<int> waveformRect;

    //window sizes
    int WINDOW_HEIGHT_24;
    int WINDOW_HEIGHT_48;
    int WINDOW_WIDTH_24;
    int WINDOW_WIDTH_48;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (THICCAudioProcessorEditor)
};

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SliderLook.h"

//color defines
#define BACKGROUND_COLOR juce::Colour(25, 27, 30)
#define WAVEFORM_BACKGROUND_COLOR juce::Colour(35, 37, 40)
#define BOLD_TEXT_COLOR juce::Colour(230, 225, 225)
#define TEXT_COLOR juce::Colour(215, 210, 210)
#define SLIDER_COLOR juce::Colour(218, 207, 213)
#define SLIDER_THUMB_COLOR juce::Colour(120, 120, 150)
#define WAVEFORM_BAR_COLOR juce::Colour(150, 150, 170)
#define WAVEFORM_LINE_COLOR juce::Colour(80, 80, 100)
#define WAVEFORM_RED_COLOR juce::Colour(0.0f, 1.0f, 0.5f, 0.85f)

//==============================================================================
THICCAudioProcessorEditor::THICCAudioProcessorEditor (THICCAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{
    Timer::startTimerHz(20);
    setResizable(true, true);
    setLookAndFeel(new SliderLook());

    //-------------------------------WINDOW HEIGHT/WIDTH---------------------

    setSize(816, 480);

    WINDOW_HEIGHT_24 = static_cast<int>(std::roundf(getHeight() / 24.0f));
    WINDOW_WIDTH_24 = static_cast<int>(std::roundf(getWidth() / 24.0f));
    WINDOW_HEIGHT_48 = static_cast<int>(std::roundf(getHeight() / 48.0f));
    WINDOW_WIDTH_48 = static_cast<int>(std::roundf(getWidth() / 48.0f));

    int KNOB_WIDTH = static_cast<int>(std::roundf(getWidth() * 1.6f / 24.0f));

    //--------------------------SLIDERS-----------------------------------

    //sliders
    juce::Component::getLookAndFeel().setColour(juce::Slider::backgroundColourId, BACKGROUND_COLOR);
    juce::Component::getLookAndFeel().setColour(juce::Slider::thumbColourId, SLIDER_THUMB_COLOR);
    juce::Component::getLookAndFeel().setColour(juce::Slider::trackColourId, SLIDER_COLOR);
    juce::Component::getLookAndFeel().setColour(juce::Slider::textBoxTextColourId, TEXT_COLOR);
    juce::Component::getLookAndFeel().setColour(juce::Slider::textBoxOutlineColourId, TEXT_COLOR);

    mixSlider.setSliderStyle(juce::Slider::LinearVertical);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, WINDOW_WIDTH_48 * 3, WINDOW_HEIGHT_24);
    addAndMakeVisible(mixSlider);
    //mixSlider.setTextValueSuffix("%");
    mixAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "mix", mixSlider));


    inputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, static_cast<int>(std::roundf(WINDOW_WIDTH_24 * 1.4)), WINDOW_HEIGHT_24);
    addAndMakeVisible(inputGainSlider);
    //inputGainSlider.setTextValueSuffix("dB");
    inputGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "inputGain", inputGainSlider));

    outputGainPostSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputGainPostSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, static_cast<int>(std::roundf(WINDOW_WIDTH_24 * 1.4)), WINDOW_HEIGHT_24);
    addAndMakeVisible(outputGainPostSlider);
    //outputGainPostSlider.setTextValueSuffix("dB");
    outputGainPostAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "outputGainPost", outputGainPostSlider));

    //knobs
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, KNOB_WIDTH, WINDOW_HEIGHT_24);
    addAndMakeVisible(driveSlider);
    driveSlider.setTextValueSuffix("dB");
    driveAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "drive", driveSlider));

    outputGainPreSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainPreSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, KNOB_WIDTH, WINDOW_HEIGHT_24);
    addAndMakeVisible(outputGainPreSlider);
    outputGainPreSlider.setTextValueSuffix("dB");
    outputGainPreAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "outputGainPre", outputGainPreSlider));

    softnessSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    softnessSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, KNOB_WIDTH, WINDOW_HEIGHT_24);
    addAndMakeVisible(softnessSlider);
    softnessSlider.setTextValueSuffix("%");
    softnessAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "softness", softnessSlider));

    kneeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    kneeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, KNOB_WIDTH, WINDOW_HEIGHT_24);
    addAndMakeVisible(kneeSlider);
    kneeSlider.setTextValueSuffix("%");
    kneeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "knee", kneeSlider));

    highPassSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    highPassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, KNOB_WIDTH, WINDOW_HEIGHT_24);
    addAndMakeVisible(highPassSlider);
    highPassSlider.setTextValueSuffix(" Hz");
    highPassAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "highPass", highPassSlider));

    lowPassSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lowPassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, KNOB_WIDTH, WINDOW_HEIGHT_24);
    addAndMakeVisible(lowPassSlider);
    lowPassSlider.setTextValueSuffix(" Hz");
    lowPassAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "lowPass", lowPassSlider));

    //--------------------------BUTTONS-------------------------------------------

    //buttons
    juce::Component::getLookAndFeel().setColour(juce::TextButton::buttonColourId, BACKGROUND_COLOR);
    juce::Component::getLookAndFeel().setColour(juce::TextButton::buttonOnColourId, BACKGROUND_COLOR);
    juce::Component::getLookAndFeel().setColour(juce::TextButton::textColourOffId, TEXT_COLOR);
    juce::Component::getLookAndFeel().setColour(juce::TextButton::textColourOnId, TEXT_COLOR);

    
    clipButton.setClickingTogglesState(true);
    addAndMakeVisible(clipButton);
    clipAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "clip", clipButton));

    peakButton.setClickingTogglesState(true);
    addAndMakeVisible(peakButton);
    peakAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "peak", peakButton));

    evenButton.setClickingTogglesState(true);
    addAndMakeVisible(evenButton);
    evenAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "even", evenButton));

    displayButton.setClickingTogglesState(true);
    addAndMakeVisible(displayButton);
    displayButton.setButtonText("Toggle Display");

    //combo boxes
    addAndMakeVisible(typeBox);
    typeBox.addItem("Big", 1); //sinx
    typeBox.addItem("Thick", 2); //e^x
    typeBox.addItem("Fat", 3); //lnx
    typeBox.addItem("Hot", 4); //tanhx
    typeAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "type", typeBox));

    addAndMakeVisible(OSFactorBox);
    OSFactorBox.addItem("No Oversampling", 1);
    OSFactorBox.addItem("2x Oversampling", 2);
    OSFactorBox.addItem("4x Oversampling", 3);
    OSFactorBox.addItem("8x Oversampling", 4);
    OSFactorBox.addItem("16x Oversampling", 5);
    OSFactorAttachment.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "OSFactor", OSFactorBox));
}

THICCAudioProcessorEditor::~THICCAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    Timer::stopTimer();
}

//==============================================================================
void THICCAudioProcessorEditor::paint (juce::Graphics& g)
{

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(BACKGROUND_COLOR);

    //------------------------TEXT-------------------------------

    //title
    g.setColour(BOLD_TEXT_COLOR);
    g.setFont(getWidth() / 30.0f);
    g.drawFittedText("THICC", 0, 0, WINDOW_WIDTH_24 * 4, WINDOW_HEIGHT_24 * 2, juce::Justification::centred, 1);

    //big text
    g.setColour(BOLD_TEXT_COLOR);
    g.setFont(getWidth() / 45.0f);

    //mix input output gain
    g.drawFittedText("Mix", WINDOW_WIDTH_24 * 19, 0, WINDOW_WIDTH_48 * 3, WINDOW_HEIGHT_24 * 2, juce::Justification::centred, 1);
    g.drawFittedText("In", static_cast<int>(std::roundf(WINDOW_WIDTH_24 * 20.75)), 0, WINDOW_WIDTH_48 * 3, WINDOW_HEIGHT_24 * 2, juce::Justification::centred, 1);
    g.drawFittedText("Out", static_cast<int>(std::roundf(WINDOW_WIDTH_24 * 22.25)), 0, WINDOW_WIDTH_48 * 3, WINDOW_HEIGHT_24 * 2, juce::Justification::centred, 1);

    //saturation
    g.drawFittedText("Character", WINDOW_WIDTH_24 * 9, WINDOW_HEIGHT_24 * 20, WINDOW_WIDTH_24 * 6, WINDOW_HEIGHT_24, juce::Justification::centred, 1);

    //saturation type

    //small text
    g.setColour(TEXT_COLOR);
    g.setFont(getWidth() / 60.0f);

    //drive
    g.drawFittedText("Drive", WINDOW_WIDTH_24 * 16, WINDOW_HEIGHT_48, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24, juce::Justification::centred, 1);

    //output pre-mix (gain)
    g.drawFittedText("Gain", WINDOW_WIDTH_24 * 16, WINDOW_HEIGHT_48 * 13, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24, juce::Justification::centred, 1);

    //softness
    g.drawFittedText("Soft", WINDOW_WIDTH_24 * 16, WINDOW_HEIGHT_48 * 25, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24, juce::Justification::centred, 1);

    //knee
    g.drawFittedText("Knee", WINDOW_WIDTH_24 * 16, WINDOW_HEIGHT_48 * 37, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24, juce::Justification::centred, 1);

    //filters
    g.drawFittedText("< High Pass", WINDOW_WIDTH_24 * 2, WINDOW_HEIGHT_24 * 20, WINDOW_WIDTH_24 * 4, WINDOW_HEIGHT_24, juce::Justification::centred, 1);
    g.drawFittedText("Low Pass >", WINDOW_WIDTH_24 * 2, WINDOW_HEIGHT_24 * 23, WINDOW_WIDTH_24 * 4, WINDOW_HEIGHT_24, juce::Justification::centred, 1);

    g.setFont(12.0f);

    //db scale
    g.drawFittedText("-0", 0, WINDOW_HEIGHT_24 * 2 - 10, 21, 20, juce::Justification::centred, 1);
    g.drawFittedText("-1", 0, WINDOW_HEIGHT_24 * 3 - 10, 21, 20, juce::Justification::centred, 1);
    g.drawFittedText("-2", 0, WINDOW_HEIGHT_24 * 4 - 10, 21, 20, juce::Justification::centred, 1);
    g.drawFittedText("-3", 0, WINDOW_HEIGHT_24 * 5 - 10, 21, 20, juce::Justification::centred, 1);
    g.drawFittedText("-6", 0, WINDOW_HEIGHT_24 * 8 - 10, 21, 20, juce::Justification::centred, 1);
    g.drawFittedText("-9", 0, WINDOW_HEIGHT_24 * 11 - 10, 21, 20, juce::Justification::centred, 1);
    g.drawFittedText("-12", 0, WINDOW_HEIGHT_24 * 14 - 10, 21, 20, juce::Justification::centred, 1);
    g.drawFittedText("-15", 0, WINDOW_HEIGHT_24 * 17 - 10, 21, 20, juce::Justification::centred, 1);


    //----------------------------BUTTONS----------------------

    clipButton.setButtonText(clipButton.getToggleState() ? "Clip Mode" : "Saturation Mode");
    peakButton.setButtonText(peakButton.getToggleState() ? "Hard Clip Peaks" : "Leave Peaks");
    if (clipButton.getToggleState()) {
        evenButton.setButtonText(evenButton.getToggleState() ? "Even Harmonics" : "Odd Harmonics");
    }
    else {
        evenButton.setButtonText(evenButton.getToggleState() ? "Warm Saturation" : "Tube Saturation");
    }

    //--------------------------WAVEFORM-----------------------------------
    
    int volArraySize = audioProcessor.getVolArraySize();
    float barWidth = waveformRect.getWidth() / static_cast<float>(volArraySize);
    float* inputPeakArr = audioProcessor.getInputPeakArr();
    float* outputPeakArr = audioProcessor.getOutputPeakArr();

    g.setColour(WAVEFORM_BACKGROUND_COLOR);
    g.fillRect(waveformRect);

    g.setColour(WAVEFORM_LINE_COLOR);

    float lineLeft = static_cast<float>(waveformRect.getX());
    float lineRight = static_cast<float>(waveformRect.getRight());

    g.drawHorizontalLine(WINDOW_HEIGHT_24 * 3, lineLeft, lineRight); //-2db
    g.drawHorizontalLine(WINDOW_HEIGHT_24 * 4, lineLeft, lineRight); //-4db
    g.drawHorizontalLine(WINDOW_HEIGHT_24 * 5, lineLeft, lineRight); //-6db
    g.drawHorizontalLine(WINDOW_HEIGHT_24 * 8, lineLeft, lineRight); //-12db

    
    g.setColour(WAVEFORM_BAR_COLOR);
    for (int i = 0; i < volArraySize; i++) {
        float output = fminf(fmaxf(outputPeakArr[i], -1) + 1, 1);
        g.fillRect(waveformRect.getX() + (volArraySize - i - 1) * barWidth, waveformRect.getY() + waveformRect.getHeight() * (1 - output), barWidth, waveformRect.getHeight() * output);
    }

    g.setColour(WAVEFORM_RED_COLOR);
    for (int i = 0; i < volArraySize; i++) {
        if (displayButton.getToggleState()) {
            if (inputPeakArr[i] > outputPeakArr[i]) {
                float output = fminf(inputPeakArr[i] - outputPeakArr[i], 1);
                g.fillRect(waveformRect.getX() + (volArraySize - i - 1) * barWidth, static_cast<float>(waveformRect.getY()), barWidth, waveformRect.getHeight() * output);
            }
        }
        else {
            float input = fminf(fmaxf(inputPeakArr[i], -1) + 1, 1);
            float output = fminf(fmaxf(outputPeakArr[i], -1) + 1, 1);
            if (input > output) {
                g.fillRect(waveformRect.getX() + (volArraySize - i - 1) * barWidth, waveformRect.getY() + waveformRect.getHeight() * (1 - input), barWidth, waveformRect.getHeight() * (input - output));
            }
        }
    }

    g.setColour(SLIDER_THUMB_COLOR);
    g.drawRect(waveformRect, 2);
}

void THICCAudioProcessorEditor::resized()
{
    WINDOW_HEIGHT_24 = static_cast<int>(std::roundf(getHeight() / 24.0f));
    WINDOW_WIDTH_24 = static_cast<int>(std::roundf(getWidth() / 24.0f));
    WINDOW_HEIGHT_48 = static_cast<int>(std::roundf(getHeight() / 48.0f));
    WINDOW_WIDTH_48 = static_cast<int>(std::roundf(getWidth() / 48.0f));
    
    waveformRect = juce::Rectangle<int>(21, WINDOW_HEIGHT_24 * 2, WINDOW_WIDTH_24 * 16 - 21, WINDOW_HEIGHT_24 * 18);

    //knobs
    driveSlider.setBounds(WINDOW_WIDTH_24 * 16, WINDOW_HEIGHT_48 * 3, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24 * 4);
    outputGainPreSlider.setBounds(WINDOW_WIDTH_24 * 16, WINDOW_HEIGHT_48 * 15, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24 * 4);

    softnessSlider.setBounds(WINDOW_WIDTH_24 * 16, WINDOW_HEIGHT_48 * 27, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24 * 4);
    kneeSlider.setBounds(WINDOW_WIDTH_24 * 16, WINDOW_HEIGHT_48 * 39, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24 * 4);

    highPassSlider.setBounds(/*WINDOW_WIDTH_24*/ 0, WINDOW_HEIGHT_24 * 20, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24 * 4);
    lowPassSlider.setBounds(WINDOW_WIDTH_24 * 5, WINDOW_HEIGHT_24 * 20, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_24 * 4);

    //sliders
    mixSlider.setBounds(WINDOW_WIDTH_24 * 19, WINDOW_HEIGHT_24 * 2, WINDOW_WIDTH_48 * 3, WINDOW_HEIGHT_24 * 21);
    inputGainSlider.setBounds(static_cast<int>(std::roundf(WINDOW_WIDTH_24 * 20.75)), WINDOW_HEIGHT_24 * 2, WINDOW_WIDTH_48 * 3, WINDOW_HEIGHT_24 * 21);
    outputGainPostSlider.setBounds(static_cast<int>(std::roundf(WINDOW_WIDTH_24 * 22.25)), WINDOW_HEIGHT_24 * 2, WINDOW_WIDTH_48 * 3, WINDOW_HEIGHT_24 * 21);

    //buttons
    int TOP_BUTTON_Y = static_cast<int>(std::roundf(getHeight() / 96.0f));

    clipButton.setBounds(WINDOW_WIDTH_24 * 4, TOP_BUTTON_Y, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_48 * 3);
    peakButton.setBounds(WINDOW_WIDTH_24 * 8, TOP_BUTTON_Y, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_48 * 3);

    evenButton.setBounds(WINDOW_WIDTH_24 * 13, WINDOW_HEIGHT_48 * 43, WINDOW_WIDTH_24 * 2, WINDOW_HEIGHT_24 * 2);

    displayButton.setBounds(waveformRect.getX() + WINDOW_HEIGHT_48, waveformRect.getBottom() - WINDOW_HEIGHT_24 * 2, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_48 * 3);

    //combo box
    typeBox.setBounds(WINDOW_WIDTH_24 * 9, WINDOW_HEIGHT_48 * 43, WINDOW_WIDTH_24 * 2, WINDOW_HEIGHT_24 * 2);
    OSFactorBox.setBounds(WINDOW_WIDTH_24 * 12, TOP_BUTTON_Y, WINDOW_WIDTH_24 * 3, WINDOW_HEIGHT_48 * 3);
}

void THICCAudioProcessorEditor::timerCallback()
{
    repaint(waveformRect);
}
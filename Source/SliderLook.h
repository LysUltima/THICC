/*
  ==============================================================================

    SliderLook.h
    Created: 21 Feb 2023 1:40:32am
    Author:  Lys

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define LINE_THICKNESS 2
#define LINE_THICKNESS_KNOB 3.0f

class SliderLook : public juce::LookAndFeel_V4 {
public:
    SliderLook() {

    }
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float minSliderPos, float maxSliderPos,
        const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        float scale = 0.4f;
        x = static_cast<int>(std::round((1 - scale) / 2 * width));
        width = static_cast<int>(std::round(width * scale));
        
        juce::Rectangle<int> rect = juce::Rectangle<int>(x, y, width, height);
        juce::Rectangle<int> rectSelect = juce::Rectangle<int>(x, static_cast<int>(sliderPos), width, height - static_cast<int>(sliderPos) + 12);

        g.setColour(slider.findColour(juce::Slider::backgroundColourId));
        g.fillRect(rect);

        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.drawRect(rect, LINE_THICKNESS);
        g.fillRect(rectSelect);

        int pin = 20;
        juce::Rectangle<int> rectPin = juce::Rectangle<int>(x, static_cast<int>(sliderPos) - (pin / 2), width, pin);
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillRect(rectPin);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override {
        //helper variables
        float radius = fminf(width / 2.5f, height / 2.5f);
        float centerX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
        float centerY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
        float rx = centerX - radius;
        float ry = centerY - radius;
        float rw = radius * 2.0f;
        float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour(slider.findColour(juce::Slider::trackColourId));
        g.fillEllipse(rx, ry, rw, rw);

        juce::Path p;
        float pointerLength = radius * 0.25f;
        float pointerThickness = LINE_THICKNESS_KNOB;
        p.addRectangle(-pointerThickness * 0.5f, LINE_THICKNESS_KNOB - radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centerX, centerY));

        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillPath(p);
    }
};

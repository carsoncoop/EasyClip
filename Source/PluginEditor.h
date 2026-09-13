#pragma once

#include <juce_audio_utils/gui/juce_AudioVisualiserComponent.h>
#include "PluginProcessor.h"
#include "Distortion.h"




//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;


    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    AudioPluginAudioProcessor& processorRef;

    juce::Label distortionHeader{"distortionHeader", "DISTORTION"};

    //Distortion
    juce::TextButton algButton;

    juce::Slider satSlider;
    juce::Slider threshSlider;
    juce::Slider outputSlider;
    juce::Slider mixSlider;

    juce::Label satLabel{"Drive Label", "Drive (+dB)"};
    juce::Label threshLabel{"Thresh Label", "Thresh (+dB)"};
    juce::Label outputLabel{"Output Label", "Output (+dB)"};
    juce::Label mixLabel{"Mix Label", "Mix (%)"};
    juce::AudioProcessorValueTreeState::SliderAttachment satSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment threshSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment outputSliderAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment mixSliderAttachment;

    juce::AudioVisualiserComponent visual {1};
};


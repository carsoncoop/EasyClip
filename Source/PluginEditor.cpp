#include "PluginProcessor.h"
#include "PluginEditor.h"

void linkMinMaxSliders (juce::Slider& minSlider, juce::Slider& maxSlider){
    minSlider.onValueChange = [&minSlider, &maxSlider] {
        if (minSlider.getValue() > maxSlider.getValue())
            maxSlider.setValue (minSlider.getValue());
    };

    maxSlider.onValueChange = [&minSlider, &maxSlider] {
        if (maxSlider.getValue() < minSlider.getValue())
            minSlider.setValue (maxSlider.getValue());
    };
}
//==============================================================================
void AudioPluginAudioProcessorEditor::timerCallback()
{
    visual.pushBuffer(processorRef.visualizerBuffer);
}
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p),
    satSliderAttachment(processorRef.getState(), "drive", satSlider),
    threshSliderAttachment(processorRef.getState(), "thresh", threshSlider),
    outputSliderAttachment(processorRef.getState(), "output", outputSlider),
    mixSliderAttachment(processorRef.getState(), "mix", mixSlider)
    /*cutoffSliderAttachment(processorRef.getState(), "cutoff", cutoffSlider),
    resoSliderAttachment(processorRef.getState(), "resonance", resoSlider),*/
{
    auto setupLabel = [this] (juce::Label& label, const juce::Font& font = juce::Font(), const juce::String& text = {}) {
        label.setJustificationType(juce::Justification::centred);
        if (font.getHeight() > 0.0f)
            label.setFont(font);
        if (text.isNotEmpty())
            label.setText(text, juce::dontSendNotification);
        addAndMakeVisible(label);
    };
    auto setupKnob = [this, &setupLabel] (juce::Slider& slider, juce::Label& label,
                                         bool popupDisplay = true, bool menuEnabled = true,
                                         juce::Component* popupOwner = nullptr,
                                         const juce::Font& font = juce::Font(juce::FontOptions { 14.0f }))
    {
        juce::Component* owner = popupOwner ? popupOwner : this;
        slider.setPopupDisplayEnabled(popupDisplay, false, owner);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setPopupMenuEnabled(menuEnabled);
        setupLabel(label, font);
        addAndMakeVisible(slider);
    };

    visual.setBufferSize(64);
    visual.setSamplesPerBlock(64);
    visual.setRepaintRate(60);
    visual.setColours(juce::Colours::black, juce::Colours::blueviolet);
    startTimerHz(60);
    addAndMakeVisible(visual);

    //Distortion Parameters---------------------------------------------------------------------------------------------

    setupKnob(satSlider, satLabel);
    setupKnob(threshSlider, threshLabel);
    setupKnob(outputSlider, outputLabel);
    setupKnob(mixSlider, mixLabel);
    addAndMakeVisible(algButton);

    //Initialize algButton text
    const juce::StringArray algChoices { "Soft Clip", "Hard Clip", "Foldback", "Downsample"};
    int idx = static_cast<int>(std::round(processorRef.getState().getRawParameterValue("algButton")->load()));
    algButton.setButtonText(algChoices[idx]);

    algButton.onClick = [this]() {
        juce::PopupMenu algMenu;
        algMenu.addItem(1, "Soft Clip");
        algMenu.addItem(2, "Hard Clip");
        algMenu.addItem(3, "Foldback");
        algMenu.addItem(4, "Downsample");

        algMenu.showMenuAsync(juce::PopupMenu::Options(),
            [this] (int result) {
                if (result == 0) {//If no selection is made

                }
                else if (result == 1) {
                    algButton.setButtonText("Soft Clip");
                    processorRef.getDistortion().setDistortionType(SoftClip);
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 2) {
                    algButton.setButtonText("Hard Clip");
                    processorRef.getDistortion().setDistortionType(HardClip);
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 3) {
                    algButton.setButtonText("Foldback");
                    processorRef.getDistortion().setDistortionType(Foldback);
                    threshLabel.setText("Thresh (+dB)", juce::dontSendNotification);
                    satLabel.setText("Drive (+dB)", juce::dontSendNotification);
                }
                else if (result == 4) {
                    algButton.setButtonText("Downsample");
                    processorRef.getDistortion().setDistortionType(Downsample);
                    threshLabel.setText("(NA)", juce::dontSendNotification);
                    satLabel.setText("Crush", juce::dontSendNotification);
                }
        });
    };

    setSize (300, 600);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void AudioPluginAudioProcessorEditor::resized()
{
    visual.setBounds(getWidth() / 2 - 75,20, 150 ,150);
    distortionHeader.setBounds(getWidth() / 5 - 62,-40, 150 ,150);

    //Distortion Drawings-----------------------------------------------------------------------------------------------
    algButton.setBounds(getWidth() / 2 - 40, getHeight() / 5 + 40, 80, 30);

    satSlider.setBounds(getWidth() / 2 - 45, getHeight() / 15 * 3 + 85, 90, 90);
    threshSlider.setBounds(getWidth() / 2 - 45, getHeight() / 15 * 5 + 100, 90, 90);
    outputSlider.setBounds(getWidth() / 2 - 45, getHeight() / 15 * 7 + 115, 90, 90);
    mixSlider.setBounds(getWidth() / 2 - 45, getHeight() / 15 * 9 + 130, 90, 90);

    satLabel.setBounds(satSlider.getX(), satSlider.getBottom() - 10, satSlider.getWidth(), 18);
    threshLabel.setBounds(threshSlider.getX(), threshSlider.getBottom() - 10, threshSlider.getWidth(), 18);
    outputLabel.setBounds(outputSlider.getX(), outputSlider.getBottom() - 10, outputSlider.getWidth(), 18);
    mixLabel.setBounds(mixSlider.getX(), mixSlider.getBottom() - 10, mixSlider.getWidth(), 18);

}

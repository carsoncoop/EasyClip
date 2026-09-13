#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>


//==============================================================================



void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();



    // SmoothedValue Preparation Lambda
    auto prepareSmoothed = [&](juce::SmoothedValue<float>& s, const juce::String& paramId,
        float timeMs = 0.01f, std::function<float(float)> transform = {})
    {
        s.reset (sampleRate, timeMs);
        if (auto* p = state.getRawParameterValue(paramId))
        {
            const float v = p->load();
            s.setTargetValue(transform ? transform(v) : v);
        }
    };

    const auto numProcessingChannels = std::max(getTotalNumInputChannels(), getTotalNumOutputChannels());

    //Distortion Preparation--------------------------------------------------------------------------------------------
    prepareSmoothed(smoothedDrive, "drive");
    prepareSmoothed(smoothedThresh, "thresh");
    prepareSmoothed(smoothedOutput, "output");
    prepareSmoothed(smoothedMix, "mix");

    distortion.prepare(getSampleRate(),
        smoothedDrive.getCurrentValue(), smoothedThresh.getCurrentValue(),
        smoothedMix.getCurrentValue(), smoothedOutput.getCurrentValue());

    //Filter preparation------------------------------------------------------------------------------------------------
    /*filter.prepare();
    smoothedCutoff.reset(sampleRate, 0.01f);
    smoothedReso.reset(sampleRate, 0.01f);
    smoothedCutoff.setTargetValue(state.getRawParameterValue("cutoff")->load());
    smoothedReso.setTargetValue(state.getRawParameterValue("resonance")->load());*/

    //Visualizer--------------------------------------------------------------------------------------------------------
    visualizerBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}


void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    smoothedDrive.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("drive")->load()));
    smoothedThresh.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("thresh")->load()));
    smoothedOutput.setTargetValue(juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load()));
    smoothedMix.setTargetValue(state.getRawParameterValue("mix")->load());


    //Used for sample & hold
    float hold = 0;
    float counter = 0;

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        //Assign parameters to class variables
        distortion.setDrive(smoothedDrive.getNextValue());
        distortion.setThresh(smoothedThresh.getNextValue());
        distortion.setMix(smoothedMix.getNextValue());
        distortion.setOutput(smoothedOutput.getNextValue());

        //Channel processing
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* input = buffer.getWritePointer(channel);
            //Distortion
            if (distortion.getDistortionType() == Downsample) {
                float dryInput = input[sample];
                if (counter == 0) {
                    hold = input[sample];
                }
                input[sample] = hold;
                counter += 0.5;
                if (counter >= static_cast<int>(juce::Decibels::gainToDecibels(distortion.getDrive()))) {
                    counter = 0;
                }
                input[sample] *= distortion.getOutput();
                float mix = distortion.getMix();
                input[sample] = dryInput * (1.0f - mix / 100) + (mix / 100) * input[sample];
            }
            else {
                input[sample] = distortion.process(input[sample]);
            }
        }
    }

    visualizerBuffer.makeCopyOf(buffer);
}

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      ), state(*this, nullptr, "parameters", createParameters()), distortion() {
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to have a built-in UI or not)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}


juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addFloatParam = [&layout](const juce::String& id,
                                  const juce::String& name,
                                  const juce::NormalisableRange<float>& range,
                                  float defaultValue,
                                  const juce::String& suffix = {})
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id },
            name,
            range,
            defaultValue,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction([suffix](float value, int) {
                    return juce::String(value, 2) + suffix;
                })));
    };

    auto addBoolParam = [&](const juce::String& id, const juce::String& name, bool def) {
        layout.add(std::make_unique<juce::AudioParameterBool>(id, name, def));
    };

    auto addChoiceParam = [&](const juce::String& id, const juce::String& name,
        const juce::StringArray& choices, int defaultIndex) {
        layout.add(std::make_unique<juce::AudioParameterChoice>(id, name, choices, defaultIndex));
    };

    addChoiceParam("algButton", "algButton", juce::StringArray { "Soft Clip", "Hard Clip", "Foldback", "Downsample" }, 0);

    juce::NormalisableRange<float> cutoffRange(20.0f, 20000.0f);
    cutoffRange.setSkewForCentre(1000.0f);

    addFloatParam("drive", "drive", juce::NormalisableRange<float>(-36.0f, 36.0f), 0.0f, " dB");
    addFloatParam("thresh", "thresh", juce::NormalisableRange<float>(-36.0f, 0.0f), 0.0f, " dB");
    addFloatParam("output", "output", juce::NormalisableRange<float>(-36.0f, 36.0f), 0.0f, " dB");
    addFloatParam("mix", "mix", juce::NormalisableRange<float>(0.0f, 100.0f), 100.0f, "%");
    //addFloatParam("cutoff", "cutoff", cutoffRange, 20000.0f, "Hz");
    //addFloatParam("resonance", "resonance", juce::NormalisableRange<float>(0.01f, 6.0f), 0.7f, "");
    return layout;
}

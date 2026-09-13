#include "Distortion.h"

Distortion::Distortion(){}

Distortion::Distortion(float sampleRate_, float drive_, float thresh_, float mix_, float output_) {
    sampleRate = sampleRate_;
    drive = drive_;
    thresh = thresh_;
    mix = mix_;
    output = output_;
}


DistortionType Distortion::getDistortionType() const {
    return distortionAlg;
}

float Distortion::getMix() const {
    return mix;
}

float Distortion::getOutput() const {
    return output;
}

float Distortion::getDrive() const {
    return drive;
}

void Distortion::setDrive(const float drive_) {
    drive = drive_;
}

void Distortion::setThresh(const float thresh_) {
    thresh = thresh_;
}

void Distortion::setMix(const float mix_) {
    mix = mix_;
}

void Distortion::setOutput(const float output_) {
    output = output_;
}

void Distortion::setDistortionType(DistortionType type) {
    distortionAlg = type;
}

void Distortion::prepare(const float sampleRate_, const float drive_, const float thresh_, const float mix_, const float output_) {
    sampleRate = sampleRate_;
    drive = drive_;
    thresh = thresh_;
    mix = mix_;
    output = output_;
    distortionAlg = SoftClip;
}

float Distortion::process(const float inputSample) const {
    float signal = 0.0f;
    switch (distortionAlg) {
        case SoftClip:
            signal = thresh * std::tanh((inputSample * drive) / thresh);
            signal *= output;
            signal = inputSample * (1.0f - mix / 100) + (mix / 100) * signal;

            return signal;
        case HardClip:
            signal = juce::jlimit(-thresh,thresh, (inputSample * drive));
            signal *= output;
            signal = inputSample * (1.0f - mix / 100) + (mix / 100) * signal;
            return signal;
        case Foldback:
            signal = std::abs(std::abs(fmod(((inputSample * drive) - thresh), (4.0f * thresh))) - 2.0f * thresh) - thresh;
            signal *= output;
            signal = inputSample * (1.0f - mix / 100) + (mix / 100) * signal;
            return signal;
        case Downsample:
            break;
            //Downsample case must be handled in the process block because it requires multiple samples
    }
    return 0.0f;
}

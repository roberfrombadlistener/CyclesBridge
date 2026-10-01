#pragma once
#include <JuceHeader.h>
#include "UsbAudioSender.h"

class MainComponent final : public juce::Component, private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void resized() override;

private:
    void timerCallback() override;
    void trySelectCycles();

    juce::AudioDeviceManager deviceManager;
    UsbAudioSender sender;
    std::unique_ptr<juce::AudioDeviceSelectorComponent> selector;
    juce::Label title;
    juce::Label status;
    juce::Label hint;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include <array>

class CyclesBridgeAudioProcessorEditor final
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit CyclesBridgeAudioProcessorEditor(CyclesBridgeAudioProcessor&);
    ~CyclesBridgeAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    CyclesBridgeAudioProcessor& processor;

    juce::Label title;
    juce::Label status;
    juce::Label stats;
    juce::Label routingHint;

    std::array<juce::Label, cyclesbridge::channels> trackLabels;
    std::array<float, cyclesbridge::channels> meterLevels{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        CyclesBridgeAudioProcessorEditor)
};

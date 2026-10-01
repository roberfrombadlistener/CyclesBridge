#pragma once

#include <JuceHeader.h>

#include "AudioRingBuffer.h"
#include "NetworkReceiver.h"

#include <array>
#include <atomic>
#include <vector>

class CyclesBridgeAudioProcessor final : public juce::AudioProcessor
{
public:
    CyclesBridgeAudioProcessor();
    ~CyclesBridgeAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "CyclesBridge"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}

    const juce::String getProgramName(int index) override
    {
        return index == 0 ? "Default" : juce::String();
    }

    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool receiverActive() const noexcept { return receiver.isReceiving(); }
    uint64_t packetsReceived() const noexcept { return receiver.getPacketsReceived(); }
    uint64_t sequenceGaps() const noexcept { return receiver.getSequenceGaps(); }
    uint64_t droppedSamples() const noexcept { return ring.getDropped(); }
    int bufferedSamples() const noexcept { return ring.ready(); }

    bool hostRateIsValid() const noexcept
    {
        return validHostRate.load(std::memory_order_relaxed);
    }

    float getTrackLevel(int trackIndex) const noexcept;

private:
    void clearScratch(int numSamples);

    AudioRingBuffer ring;
    NetworkReceiver receiver;

    std::atomic<bool> validHostRate{false};

    std::array<std::vector<float>, cyclesbridge::channels> scratch;
    std::array<std::atomic<float>, cyclesbridge::channels> trackLevels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclesBridgeAudioProcessor)
};

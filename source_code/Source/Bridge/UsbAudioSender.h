#pragma once
#include <JuceHeader.h>
#include "../Common/BridgeProtocol.h"

class UsbAudioSender final : public juce::AudioIODeviceCallback
{
public:
    UsbAudioSender();
    ~UsbAudioSender() override = default;

    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                          int numInputChannels,
                                          float* const* outputChannelData,
                                          int numOutputChannels,
                                          int numSamples,
                                          const juce::AudioIODeviceCallbackContext&) override;
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;

    double getSampleRate() const noexcept { return currentSampleRate.load(); }
    int getInputChannels() const noexcept { return currentInputs.load(); }
    uint64_t getPacketsSent() const noexcept { return packetsSent.load(); }
    juce::String getLastError() const;

private:
    juce::DatagramSocket socket{false};
    std::atomic<double> currentSampleRate{0.0};
    std::atomic<int> currentInputs{0};
    std::atomic<uint64_t> packetsSent{0};
    std::atomic<uint32_t> sequence{0};
    mutable juce::CriticalSection errorLock;
    juce::String lastError;
};

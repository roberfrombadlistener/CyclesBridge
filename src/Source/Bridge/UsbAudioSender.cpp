#include "UsbAudioSender.h"

UsbAudioSender::UsbAudioSender()
{
    socket.setEnablePortReuse(true);
}

void UsbAudioSender::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                       int numInputChannels,
                                                       float* const* outputChannelData,
                                                       int numOutputChannels,
                                                       int numSamples,
                                                       const juce::AudioIODeviceCallbackContext&)
{
    for (int ch = 0; ch < numOutputChannels; ++ch)
        if (outputChannelData[ch] != nullptr)
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);

    currentInputs.store(numInputChannels, std::memory_order_relaxed);

    for (int start = 0; start < numSamples; start += cyclesbridge::packetFrames)
    {
        cyclesbridge::AudioPacket packet;
        packet.sequence = sequence.fetch_add(1, std::memory_order_relaxed);
        packet.frames = static_cast<uint16_t>(juce::jmin(cyclesbridge::packetFrames, numSamples - start));

        for (int ch = 0; ch < cyclesbridge::channels; ++ch)
        {
            if (ch < numInputChannels && inputChannelData[ch] != nullptr)
                juce::FloatVectorOperations::copy(packet.samples[ch], inputChannelData[ch] + start, packet.frames);
        }

        const int bytes = static_cast<int>(offsetof(cyclesbridge::AudioPacket, samples)
                          + sizeof(float) * cyclesbridge::channels * cyclesbridge::packetFrames);
        const int result = socket.write("127.0.0.1", cyclesbridge::udpPort, &packet, bytes);
        if (result > 0)
            packetsSent.fetch_add(1, std::memory_order_relaxed);
    }
}

void UsbAudioSender::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    currentSampleRate.store(device != nullptr ? device->getCurrentSampleRate() : 0.0,
                            std::memory_order_relaxed);
}

void UsbAudioSender::audioDeviceStopped()
{
    currentSampleRate.store(0.0, std::memory_order_relaxed);
    currentInputs.store(0, std::memory_order_relaxed);
}

void UsbAudioSender::audioDeviceError(const juce::String& errorMessage)
{
    const juce::ScopedLock lock(errorLock);
    lastError = errorMessage;
}

juce::String UsbAudioSender::getLastError() const
{
    const juce::ScopedLock lock(errorLock);
    return lastError;
}

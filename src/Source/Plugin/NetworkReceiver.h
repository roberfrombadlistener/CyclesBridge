#pragma once

#include <JuceHeader.h>

#include "AudioRingBuffer.h"
#include "../Common/BridgeProtocol.h"

#include <atomic>

class NetworkReceiver final : private juce::Thread
{
public:
    explicit NetworkReceiver(AudioRingBuffer& ringBuffer);
    ~NetworkReceiver() override;

    bool start();
    void stop();

    bool isReceiving() const noexcept;

    uint64_t getPacketsReceived() const noexcept
    {
        return packetCount.load(std::memory_order_relaxed);
    }

    uint64_t getSequenceGaps() const noexcept
    {
        return sequenceGaps.load(std::memory_order_relaxed);
    }

private:
    void run() override;

    AudioRingBuffer& ring;
    juce::DatagramSocket socket{false};

    std::atomic<uint64_t> packetCount{0};
    std::atomic<uint64_t> sequenceGaps{0};
    std::atomic<uint32_t> lastPacketTime{0};

    uint32_t expectedSequence = 0;
    bool haveSequence = false;
};

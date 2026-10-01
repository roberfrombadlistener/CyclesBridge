#pragma once
#include <JuceHeader.h>
#include "../Common/BridgeProtocol.h"
#include <array>
#include <vector>

class AudioRingBuffer
{
public:
    explicit AudioRingBuffer(int capacitySamples = cyclesbridge::sampleRate * 2)
        : fifo(capacitySamples), capacity(capacitySamples)
    {
        for (auto& c : data)
            c.resize(static_cast<size_t>(capacitySamples), 0.0f);
    }

    void push(const float* const* channels, int numChannels, int numSamples)
    {
        int start1, size1, start2, size2;
        fifo.prepareToWrite(numSamples, start1, size1, start2, size2);
        copyIn(channels, numChannels, 0, start1, size1);
        copyIn(channels, numChannels, size1, start2, size2);
        fifo.finishedWrite(size1 + size2);
        if (size1 + size2 < numSamples)
            dropped.fetch_add(static_cast<uint64_t>(numSamples - size1 - size2), std::memory_order_relaxed);
    }

    int pop(float* const* channels, int numChannels, int numSamples)
    {
        int start1, size1, start2, size2;
        fifo.prepareToRead(numSamples, start1, size1, start2, size2);
        copyOut(channels, numChannels, 0, start1, size1);
        copyOut(channels, numChannels, size1, start2, size2);
        fifo.finishedRead(size1 + size2);
        return size1 + size2;
    }

    int ready() const noexcept { return fifo.getNumReady(); }
    uint64_t getDropped() const noexcept { return dropped.load(std::memory_order_relaxed); }

private:
    void copyIn(const float* const* src, int numChannels, int srcOffset, int dstOffset, int count)
    {
        if (count <= 0) return;
        for (int ch = 0; ch < cyclesbridge::channels; ++ch)
        {
            auto* dst = data[static_cast<size_t>(ch)].data() + dstOffset;
            if (ch < numChannels && src[ch] != nullptr)
                juce::FloatVectorOperations::copy(dst, src[ch] + srcOffset, count);
            else
                juce::FloatVectorOperations::clear(dst, count);
        }
    }

    void copyOut(float* const* dst, int numChannels, int dstOffset, int srcOffset, int count)
    {
        if (count <= 0) return;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            if (dst[ch] == nullptr) continue;
            if (ch < cyclesbridge::channels)
                juce::FloatVectorOperations::copy(dst[ch] + dstOffset,
                                                  data[static_cast<size_t>(ch)].data() + srcOffset,
                                                  count);
            else
                juce::FloatVectorOperations::clear(dst[ch] + dstOffset, count);
        }
    }

    juce::AbstractFifo fifo;
    const int capacity;
    std::array<std::vector<float>, cyclesbridge::channels> data;
    std::atomic<uint64_t> dropped{0};
};

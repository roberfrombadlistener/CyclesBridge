#include "NetworkReceiver.h"

NetworkReceiver::NetworkReceiver(AudioRingBuffer& ringBuffer)
    : juce::Thread("CyclesBridge receiver"), ring(ringBuffer)
{
}

NetworkReceiver::~NetworkReceiver()
{
    stop();
}

bool NetworkReceiver::start()
{
    if (isThreadRunning())
        return true;

    socket.setEnablePortReuse(true);

    if (!socket.bindToPort(cyclesbridge::udpPort, "127.0.0.1"))
        return false;

    startThread(juce::Thread::Priority::high);
    return true;
}

void NetworkReceiver::stop()
{
    signalThreadShouldExit();
    socket.shutdown();
    stopThread(1000);
}

bool NetworkReceiver::isReceiving() const noexcept
{
    const auto last = lastPacketTime.load(std::memory_order_relaxed);

    if (last == 0)
        return false;

    const uint32_t now = juce::Time::getMillisecondCounter();
    return static_cast<uint32_t>(now - last) < 1000u;
}

void NetworkReceiver::run()
{
    cyclesbridge::AudioPacket packet;

    while (!threadShouldExit())
    {
        const int ready = socket.waitUntilReady(true, 200);

        if (ready <= 0)
            continue;

        const int bytes = socket.read(&packet, sizeof(packet), false);

        if (bytes <= 0
            || packet.magic != cyclesbridge::packetMagic
            || packet.channelCount != cyclesbridge::channels
            || packet.frames == 0
            || packet.frames > cyclesbridge::packetFrames)
        {
            continue;
        }

        packetCount.fetch_add(1, std::memory_order_relaxed);
        lastPacketTime.store(
            juce::Time::getMillisecondCounter(),
            std::memory_order_relaxed);

        if (haveSequence && packet.sequence != expectedSequence)
        {
            const uint32_t difference = packet.sequence - expectedSequence;
            sequenceGaps.fetch_add(
                static_cast<uint64_t>(difference),
                std::memory_order_relaxed);
        }

        expectedSequence = packet.sequence + 1;
        haveSequence = true;

        const float* pointers[cyclesbridge::channels];

        for (int channel = 0; channel < cyclesbridge::channels; ++channel)
            pointers[channel] = packet.samples[channel];

        ring.push(pointers, cyclesbridge::channels, packet.frames);
    }
}

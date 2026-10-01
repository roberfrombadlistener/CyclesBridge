#pragma once
#include <JuceHeader.h>

namespace cyclesbridge
{
constexpr uint32_t packetMagic = 0x43594236; // CYB6
constexpr int channels = 6;
constexpr int packetFrames = 32;
constexpr int sampleRate = 48000;
constexpr int udpPort = 48136;

#pragma pack(push, 1)
struct AudioPacket
{
    uint32_t magic = packetMagic;
    uint32_t sequence = 0;
    uint16_t frames = 0;
    uint16_t channelCount = channels;
    float samples[channels][packetFrames]{};
};
#pragma pack(pop)

static_assert(sizeof(AudioPacket) < 1024, "Packet should stay comfortably below localhost UDP limits");
}

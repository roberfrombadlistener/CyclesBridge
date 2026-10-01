#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

CyclesBridgeAudioProcessor::CyclesBridgeAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withOutput("Main Out", juce::AudioChannelSet::stereo(), true)
              .withOutput("Track 1", juce::AudioChannelSet::mono(), true)
              .withOutput("Track 2", juce::AudioChannelSet::mono(), true)
              .withOutput("Track 3", juce::AudioChannelSet::mono(), true)
              .withOutput("Track 4", juce::AudioChannelSet::mono(), true)
              .withOutput("Track 5", juce::AudioChannelSet::mono(), true)
              .withOutput("Track 6", juce::AudioChannelSet::mono(), true)),
      receiver(ring)
{
    for (auto& level : trackLevels)
        level.store(0.0f, std::memory_order_relaxed);

    receiver.start();
}

CyclesBridgeAudioProcessor::~CyclesBridgeAudioProcessor()
{
    receiver.stop();
}

void CyclesBridgeAudioProcessor::prepareToPlay(double sampleRate,
                                               int samplesPerBlock)
{
    validHostRate.store(
        std::abs(sampleRate - static_cast<double>(cyclesbridge::sampleRate)) < 1.0,
        std::memory_order_relaxed);

    const int size = juce::jmax(samplesPerBlock, 2048);

    for (auto& channel : scratch)
        channel.resize(static_cast<size_t>(size), 0.0f);
}

void CyclesBridgeAudioProcessor::releaseResources()
{
}

bool CyclesBridgeAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const
{
    if (!layouts.inputBuses.isEmpty())
        return false;

    if (layouts.outputBuses.size() != 7)
        return false;

    if (layouts.getChannelSet(false, 0) != juce::AudioChannelSet::stereo())
        return false;

    for (int bus = 1; bus < 7; ++bus)
    {
        const auto set = layouts.getChannelSet(false, bus);

        // Hosts such as Live may temporarily disable aux buses while
        // negotiating or when the user has not routed them yet.
        if (!set.isDisabled() && set != juce::AudioChannelSet::mono())
            return false;
    }

    return true;
}

void CyclesBridgeAudioProcessor::clearScratch(int numSamples)
{
    for (auto& channel : scratch)
    {
        if (static_cast<int>(channel.size()) < numSamples)
            channel.resize(static_cast<size_t>(numSamples), 0.0f);

        juce::FloatVectorOperations::clear(channel.data(), numSamples);
    }
}

void CyclesBridgeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    buffer.clear();

    const int numSamples = buffer.getNumSamples();

    if (!validHostRate.load(std::memory_order_relaxed))
        return;

    clearScratch(numSamples);

    float* channelPointers[cyclesbridge::channels];

    for (int ch = 0; ch < cyclesbridge::channels; ++ch)
        channelPointers[ch] = scratch[static_cast<size_t>(ch)].data();

    const int samplesReceived = ring.pop(
        channelPointers,
        cyclesbridge::channels,
        numSamples);

    if (samplesReceived < numSamples)
    {
        for (int ch = 0; ch < cyclesbridge::channels; ++ch)
        {
            juce::FloatVectorOperations::clear(
                channelPointers[ch] + samplesReceived,
                numSamples - samplesReceived);
        }
    }

    // Update lightweight UI meters from the received data.
    for (int ch = 0; ch < cyclesbridge::channels; ++ch)
    {
        float peak = 0.0f;

        if (numSamples > 0)
        {
            peak = juce::FloatVectorOperations::findMinAndMax(
                       channelPointers[ch], numSamples)
                       .getEnd();

            const auto range = juce::FloatVectorOperations::findMinAndMax(
                channelPointers[ch], numSamples);

            peak = juce::jmax(std::abs(range.getStart()), std::abs(range.getEnd()));
        }

        trackLevels[static_cast<size_t>(ch)].store(
            juce::jlimit(0.0f, 1.0f, peak),
            std::memory_order_relaxed);
    }

    // Main Out mirrors Track 1 for easy monitoring/debugging.
    auto mainOut = getBusBuffer(buffer, false, 0);

    if (mainOut.getNumChannels() >= 2)
    {
        juce::FloatVectorOperations::copy(
            mainOut.getWritePointer(0),
            channelPointers[0],
            numSamples);

        juce::FloatVectorOperations::copy(
            mainOut.getWritePointer(1),
            channelPointers[0],
            numSamples);
    }

    // Aux outputs 1-6 each receive the corresponding Model:Cycles mono track.
    for (int track = 0; track < cyclesbridge::channels; ++track)
    {
        auto trackOut = getBusBuffer(buffer, false, track + 1);

        if (trackOut.getNumChannels() > 0)
        {
            juce::FloatVectorOperations::copy(
                trackOut.getWritePointer(0),
                channelPointers[track],
                numSamples);
        }
    }
}

float CyclesBridgeAudioProcessor::getTrackLevel(int trackIndex) const noexcept
{
    if (trackIndex < 0 || trackIndex >= cyclesbridge::channels)
        return 0.0f;

    return trackLevels[static_cast<size_t>(trackIndex)].load(
        std::memory_order_relaxed);
}

juce::AudioProcessorEditor* CyclesBridgeAudioProcessor::createEditor()
{
    return new CyclesBridgeAudioProcessorEditor(*this);
}

void CyclesBridgeAudioProcessor::getStateInformation(juce::MemoryBlock&)
{
}

void CyclesBridgeAudioProcessor::setStateInformation(const void*, int)
{
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CyclesBridgeAudioProcessor();
}

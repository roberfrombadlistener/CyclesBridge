#include "PluginEditor.h"

CyclesBridgeAudioProcessorEditor::CyclesBridgeAudioProcessorEditor(
    CyclesBridgeAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    title.setText("CyclesBridge", juce::dontSendNotification);
    title.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    title.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(title);

    status.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(status);

    stats.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(stats);

    routingHint.setText(
        "Use one CyclesBridge instance. In Live, create Audio Tracks and choose "
        "the CyclesBridge Track 1-6 outputs as their Audio From source.",
        juce::dontSendNotification);
    routingHint.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(routingHint);

    for (int i = 0; i < cyclesbridge::channels; ++i)
    {
        trackLabels[static_cast<size_t>(i)].setText(
            "TRACK " + juce::String(i + 1),
            juce::dontSendNotification);
        trackLabels[static_cast<size_t>(i)].setJustificationType(
            juce::Justification::centred);
        addAndMakeVisible(trackLabels[static_cast<size_t>(i)]);
    }

    setSize(680, 330);
    startTimerHz(20);
}

CyclesBridgeAudioProcessorEditor::~CyclesBridgeAudioProcessorEditor()
{
    stopTimer();
}

void CyclesBridgeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff181818));

    auto bounds = getLocalBounds().toFloat().reduced(14.0f);
    g.setColour(juce::Colour(0xff343434));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    auto meters = getLocalBounds().reduced(24);
    meters.removeFromTop(130);
    meters.removeFromBottom(70);

    const int width = meters.getWidth() / cyclesbridge::channels;

    for (int i = 0; i < cyclesbridge::channels; ++i)
    {
        auto cell = meters.removeFromLeft(width).reduced(8);
        auto meter = cell.reduced(16, 4);

        g.setColour(juce::Colour(0xff2b2b2b));
        g.fillRoundedRectangle(meter.toFloat(), 4.0f);

        const float level = juce::jlimit(
            0.0f,
            1.0f,
            meterLevels[static_cast<size_t>(i)]);

        const int filledHeight = static_cast<int>(
            static_cast<float>(meter.getHeight()) * level);

        auto fill = meter.withTop(meter.getBottom() - filledHeight);

        g.setColour(juce::Colour(0xffb8e986));
        g.fillRoundedRectangle(fill.toFloat(), 4.0f);
    }
}

void CyclesBridgeAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(24);

    title.setBounds(area.removeFromTop(42));
    status.setBounds(area.removeFromTop(28));
    stats.setBounds(area.removeFromTop(28));

    area.removeFromTop(12);

    auto trackArea = area.removeFromTop(150);
    const int width = trackArea.getWidth() / cyclesbridge::channels;

    for (int i = 0; i < cyclesbridge::channels; ++i)
    {
        auto cell = trackArea.removeFromLeft(width);
        trackLabels[static_cast<size_t>(i)].setBounds(
            cell.removeFromBottom(24).reduced(4, 0));
    }

    area.removeFromTop(8);
    routingHint.setBounds(area);
}

void CyclesBridgeAudioProcessorEditor::timerCallback()
{
    if (!processor.hostRateIsValid())
    {
        status.setText(
            "Status: Live must run at 48 kHz",
            juce::dontSendNotification);
    }
    else if (processor.receiverActive())
    {
        status.setText(
            "Status: Receiving six Model:Cycles tracks",
            juce::dontSendNotification);
    }
    else
    {
        status.setText(
            "Status: Waiting for CyclesBridge standalone app",
            juce::dontSendNotification);
    }

    stats.setText(
        "Packets " + juce::String(processor.packetsReceived())
            + "  |  Buffered " + juce::String(processor.bufferedSamples())
            + "  |  Gaps " + juce::String(processor.sequenceGaps())
            + "  |  Dropped " + juce::String(processor.droppedSamples()),
        juce::dontSendNotification);

    for (int i = 0; i < cyclesbridge::channels; ++i)
    {
        const float target = processor.getTrackLevel(i);
        auto& shown = meterLevels[static_cast<size_t>(i)];

        if (target > shown)
            shown = target;
        else
            shown *= 0.82f;
    }

    repaint();
}

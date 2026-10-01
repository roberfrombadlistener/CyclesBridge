#include "MainComponent.h"

MainComponent::MainComponent()
{
    title.setText("CyclesBridge", juce::dontSendNotification);
    title.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    addAndMakeVisible(title);

    status.setText("Starting audio device…", juce::dontSendNotification);
    addAndMakeVisible(status);

    hint.setText("Select Elektron Model:Cycles as the input device. The patched firmware must expose 6 inputs at 48 kHz.",
                 juce::dontSendNotification);
    hint.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(hint);

    deviceManager.initialise(cyclesbridge::channels, 0, nullptr, true);
    trySelectCycles();

    selector = std::make_unique<juce::AudioDeviceSelectorComponent>(deviceManager,
                                                                    cyclesbridge::channels,
                                                                    cyclesbridge::channels,
                                                                    0, 0,
                                                                    false, false, false, false);
    addAndMakeVisible(*selector);
    deviceManager.addAudioCallback(&sender);

    setSize(720, 520);
    startTimerHz(5);
}

MainComponent::~MainComponent()
{
    stopTimer();
    deviceManager.removeAudioCallback(&sender);
    deviceManager.closeAudioDevice();
}

void MainComponent::trySelectCycles()
{
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.inputDeviceName = "Elektron Model:Cycles";
    setup.outputDeviceName.clear();
    setup.sampleRate = cyclesbridge::sampleRate;
    setup.bufferSize = 0;
    setup.useDefaultInputChannels = false;
    setup.inputChannels.clear();
    for (int ch = 0; ch < cyclesbridge::channels; ++ch)
        setup.inputChannels.setBit(ch);

    const auto error = deviceManager.setAudioDeviceSetup(setup, true);
    if (error.isNotEmpty())
        status.setText("Auto-select failed: " + error + " — choose the device below.", juce::dontSendNotification);
}

void MainComponent::timerCallback()
{
    auto* device = deviceManager.getCurrentAudioDevice();
    if (device == nullptr)
    {
        status.setText("No audio input open.", juce::dontSendNotification);
        return;
    }

    const auto sr = sender.getSampleRate();
    const auto ins = sender.getInputChannels();
    const auto packets = sender.getPacketsSent();
    juce::String text = "Device: " + device->getName()
                      + " | " + juce::String(sr, 0) + " Hz"
                      + " | inputs seen: " + juce::String(ins)
                      + " | packets: " + juce::String(packets);

    if (sr != cyclesbridge::sampleRate || ins < cyclesbridge::channels)
        text += "  ⚠ Expected 48 kHz / 6 inputs";

    const auto err = sender.getLastError();
    if (err.isNotEmpty())
        text += " | error: " + err;

    status.setText(text, juce::dontSendNotification);
}

void MainComponent::resized()
{
    auto r = getLocalBounds().reduced(16);
    title.setBounds(r.removeFromTop(36));
    status.setBounds(r.removeFromTop(28));
    hint.setBounds(r.removeFromTop(50));
    r.removeFromTop(8);
    if (selector != nullptr)
        selector->setBounds(r);
}

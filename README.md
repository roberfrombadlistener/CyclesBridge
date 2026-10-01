# CyclesBridge
A companion app for model:cycles running custom firmware that allows for similar functionality to Elektron's overbridge.

CyclesBridge is a vst3 plugin with a required standalone companion app. It allows a model:cycles running modded firmware to send 6 tracks of audio over usb that can be monitored by other audio tracks in ableton.

Full disclosure, this software was fully vibe coded using an LLM.
I really don't have a ton of technical knowledge regarding how this works, beyond that it was made using JUCE.
I have only tested this on Windows11 with Ableton 12 Suite using ASIO4ALL and a Behringer UMC1820 interface - I have no idea what other configurations will or won't work.

**Companion App:**

<img width="722" height="552" alt="companion app" src="https://github.com/user-attachments/assets/823aef7e-2663-4fb6-9bd3-9093b3f3ab24" />

**VST:**

<img width="682" height="362" alt="vst" src="https://github.com/user-attachments/assets/26a9f648-cd44-4186-b81e-a58a75276cb2" />




[Download ZIP](https://github.com/roberfrombadlistener/CyclesBridge/raw/refs/heads/main/CyclesBridge.zip)

How to use:

**step 1.**

flash a firmware onto the model:cycles that exposes 6 audio tracks over usb. this can be accomplished - using this tool:
https://18nelli18.github.io/Modded-Cycles/flasher/

**step 2.**

you will need at least two audio devices, one of which must be an ASIO driver that can host the model cycles.
i have had success so far with asio4all: https://asio4all.org/

**step 3.**

move the entire CyclesBridge.vst3 folder to your vst3 plugins directory.
usually this is found at "C:\Program Files\Common Files\VST3"

**step 4.**

open Ableton and it should find the CyclesBridge vst. Add it to a midi track

**step 5.**

open the standalone CyclesBridge app and select your asio device. 
open the configuration panel and configure it to use the model:cycles

**step 6.**

in ableton, use a different audio device than the device that the CyclesBridge app is using. (tested with a Behringer UMC1820)
at this point, if you trigger sounds on channel 1 of the model:cycles, you should hear it coming out of the track in ableton with the CyclesBridge vst.
if not - there's something off about your audio configuration in the standalone app

**Basic Routing Setup:**

mute the cycles bridge app and make 6 audio tracks
set the "Audio From" for each track to the track with the CyclesBridge vst 
change the audio source for each track from "Post Mixer" to Track 1, Track 2, and so on.
Set the monitor to "in" for each of the tracks to hear them playing through ableton


**Note on Latency:**

Using two audio devices like this can be CPU heavy and can create latency. Try to set the buffer size for both audio devices as low as possible without creating distortion. 
also the sample rate for both standalone app and Ableton must be set to 48000
If there is a noticeable delay in the clock when sending clock from Ableton to the cycles, open the "Tempo and MIDI" tab in preferences, click the arrow next to the in and out ports for cycles to expose additional settings
adjust the MIDI Clock Sync Delay setting until it resolves the issue - I had to set mine to around - 10 ms while using a buffer size of 128 samples for both my interface and the standalone app

**CyclesBridge is an unofficial community project and is not affiliated with or endorsed by Elektron Music Machines. Elektron and Model:Cycles are trademarks of their respective owners. No Elektron firmware is included in this repository.**

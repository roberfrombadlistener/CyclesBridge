CyclesBridge Multi-Output Drop-In
=================================

This package changes the VST3 to use ONE plugin instance with seven output buses:

  Main Out  - stereo, mirrors Track 1 for easy monitoring
  Track 1   - mono
  Track 2   - mono
  Track 3   - mono
  Track 4   - mono
  Track 5   - mono
  Track 6   - mono

The standalone app remains responsible for opening the Model:Cycles through
FlexASIO and sending all six channels to the VST over localhost.

IMPORTANT
---------
Keep your existing files that are not included in this zip, especially:

  Source/Common/BridgeProtocol.h
  Source/Plugin/AudioRingBuffer.h
  Source/Plugin/NetworkReceiver.h
  Source/Plugin/NetworkReceiver.cpp
  Source/Bridge/*

Replace only the files in this package at their matching paths.

BUILD
-----
From C:\Users\Rob\Desktop\CyclesBridge:

  rmdir /s /q build
  cmake -S . -B build -G "Visual Studio 18 2026" -A x64
  cmake --build build --config Release

Install the VST3 by replacing:

  C:\Program Files\Common Files\VST3\CyclesBridge.vst3

with:

  build\CyclesBridgePlugin_artefacts\Release\VST3\CyclesBridge.vst3

Run the standalone app from approximately:

  build\CyclesBridgeApp_artefacts\Release\CyclesBridge.exe

FLEXASIO
--------
Use FlexASIO as the standalone app's ASIO device.
Model:Cycles capture must be 6 channels at 48 kHz.

ABLETON LIVE ROUTING
--------------------
1. Add ONE instance of CyclesBridge to a MIDI track.
2. Start the standalone CyclesBridge app and confirm the plugin packet counter rises.
3. Create six Audio Tracks.
4. On each Audio Track, set Audio From to the CyclesBridge track.
5. In the second routing chooser, select the corresponding Track 1 through Track 6 output.
6. Set monitoring to In (or arm the track and use Auto).

If Live only exposes the Main Out and not Track 1-6, check Live's routing chooser
and plugin output configuration before creating additional plugin instances.

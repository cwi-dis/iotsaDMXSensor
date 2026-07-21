# DMX Sensor controller

![build-platformio](https://github.com/cwi-dis/iotsaDMXSensor/workflows/build-platformio/badge.svg)
![build-arduino](https://github.com/cwi-dis/iotsaDMXSensor/workflows/build-arduino/badge.svg)

This iotsa application listens for Estimote sensors over Bluetooth LE and provides the readings of the individual sensors as DMX slider values, using ArtNet over Wifi.

## History

Created around 2020 for the *Lit Lace* project, where ballet dancers wore Estimote sensors on
their limbs and their arm/leg movements affected the theatre lighting via DMX/Art-Net.

In late 2025 the Estimote-listening code was forked into a separate repo,
[iotsaEstimotes](https://github.com/cwi-dis/iotsaEstimotes), for the *Visboeck* project, which
needs the raw sensor readings (not DMX output) for a Unity-based VR application. The two repos'
Estimote-parsing code has since diverged. A new project that would reuse this DMX/Art-Net
functionality may be coming up, which raises the question of whether the two repos should be
unified (with Art-Net output becoming optional) — see the tracking issue for that discussion.

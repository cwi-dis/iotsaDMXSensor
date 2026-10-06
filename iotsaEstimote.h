#ifndef _IOTSAESTIMOTE_H_
#define _IOTSAESTIMOTE_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaDMX.h"

// NimBLEDevice.h sets up #define-based compat aliases (BLEDevice, BLEScan,
// BLEAdvertisedDevice, etc. -> their Nim* equivalents), which this file
// deliberately doesn't use (see cwi-dis/iotsa#185). Including the old
// legacy header names directly is ambiguous: depending on the toolchain,
// they can resolve to the ESP32 core's own bundled (and here unwanted)
// classic BLE library instead of NimBLE-Arduino.
#include <NimBLEDevice.h>

struct Estimote {
  uint8_t id[8];
  int8_t x, y, z;
  bool seen;
};

class IotsaEstimoteMod : public IotsaModule, public NimBLEScanCallbacks {
public:
  IotsaEstimoteMod(IotsaApplication &_app, bool early=false)
  : IotsaModule(_app, early),
    pBLEScan(NULL),
    nKnownEstimote(0),
    nNewEstimote(0),
    estimotes(NULL),
    dmx(NULL)
  {}

  void setup() override;
  void lateSetup() override;
  void loop() override;
  String info() override;
  void setDMX(IotsaDMXMod *_dmx, int portIndex);
  // BLE scan callbacks
  void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;
  void onScanEnd(const NimBLEScanResults& scanResults, int reason) override;
protected:
  bool getHandler(const char *path, JsonObject& reply) override;
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply) override;
  void configLoad() override;
  void configSave() override;
  void webHandler() override;
  bool _allSensorsSeen();
  void _resetSensorsSeen();
  void _sensorData(uint8_t *id, int8_t x, int8_t y, int8_t z);
  String argument;
  NimBLEScan* pBLEScan;
  int nKnownEstimote;
  int nNewEstimote;
  struct Estimote *estimotes;
  IotsaDMXMod *dmx;
  uint8_t sliderBuffer[512];
};

#endif

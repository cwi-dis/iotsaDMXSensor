#ifndef _IOTSAESTIMOTE_H_
#define _IOTSAESTIMOTE_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaDMX.h"

// NimBLEDevice.h sets up #define-based compat aliases (BLEDevice, BLEScan,
// BLEAdvertisedDevice, etc. -> their Nim* equivalents). Including the old
// legacy header names directly is ambiguous: depending on the toolchain,
// they can resolve to the ESP32 core's own bundled (and here unwanted)
// classic BLE library instead of NimBLE-Arduino.
#include <NimBLEDevice.h>

#ifdef IOTSA_WITH_API
#define IotsaEstimoteModBaseMod IotsaApiMod
#else
#define IotsaEstimoteModBaseMod IotsaMod
#endif

struct Estimote {
  uint8_t id[8];
  int8_t x, y, z;
  bool seen;
};

class IotsaEstimoteMod : public IotsaEstimoteModBaseMod, public NimBLEScanCallbacks {
public:
  IotsaEstimoteMod(IotsaApplication &_app, IotsaAuthenticationProvider *_auth=NULL, bool early=false)
  : IotsaEstimoteModBaseMod(_app, _auth, early),
    pBLEScan(NULL),
    nKnownEstimote(0),
    nNewEstimote(0),
    estimotes(NULL),
    dmx(NULL)
  {}

  void setup() override;
  void serverSetup() override;
  void loop() override;
  String info() override;
  void setDMX(IotsaDMXMod *_dmx, int portIndex);
  // BLE scan callbacks
  void onResult(const BLEAdvertisedDevice *advertisedDevice) override;
  void onScanEnd(const NimBLEScanResults& scanResults, int reason) override;
protected:
  bool getHandler(const char *path, JsonObject& reply) override;
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply) override;
  void configLoad() override;
  void configSave() override;
  void handler();
  bool _allSensorsSeen();
  void _resetSensorsSeen();
  void _sensorData(uint8_t *id, int8_t x, int8_t y, int8_t z);
  String argument;
  BLEScan* pBLEScan;
  int nKnownEstimote;
  int nNewEstimote;
  struct Estimote *estimotes;
  IotsaDMXMod *dmx;
  uint8_t sliderBuffer[512];
};

#endif

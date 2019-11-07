#ifndef _IOTSAESTIMOTE_H_
#define _IOTSAESTIMOTE_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaDMX.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#ifdef IOTSA_WITH_API
#define IotsaEstimoteModBaseMod IotsaApiMod
#else
#define IotsaEstimoteModBaseMod IotsaMod
#endif

class IotsaEstimoteMod : public IotsaEstimoteModBaseMod, public BLEAdvertisedDeviceCallbacks {
public:
  using IotsaEstimoteModBaseMod::IotsaEstimoteModBaseMod;
  void setup();
  void serverSetup();
  void loop();
  String info();
  void setDMX(IotsaDMXMod *dmx);
  // BLE callback
  void onResult(BLEAdvertisedDevice advertisedDevice);
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  String argument;
  BLEScan* pBLEScan;
};

#endif

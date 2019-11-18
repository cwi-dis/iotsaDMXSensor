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

struct Estimote {
  uint8_t id[8];
  int8_t x, y, z;
  bool seen;
};

class IotsaEstimoteMod : public IotsaEstimoteModBaseMod, public BLEAdvertisedDeviceCallbacks {
public:
  IotsaEstimoteMod(IotsaApplication &_app, IotsaAuthenticationProvider *_auth=NULL, bool early=false)
  : IotsaEstimoteModBaseMod(_app, _auth, early),
    pBLEScan(NULL),
    nKnownEstimote(0),
    nNewEstimote(0),
    estimotes(NULL),
    wantToSendDMX(false),
    dmx(NULL)
  {}

  void setup();
  void serverSetup();
  void loop();
  String info();
  void setDMX(IotsaDMXMod *_dmx);
  // BLE callback
  void onResult(BLEAdvertisedDevice advertisedDevice);
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  bool _allSensorsSeen();
  void _resetSensorsSeen();
  void _sensorData(uint8_t *id, int8_t x, int8_t y, int8_t z);
  String argument;
  BLEScan* pBLEScan;
  int nKnownEstimote;
  int nNewEstimote;
  struct Estimote *estimotes;
  bool wantToSendDMX;
  IotsaDMXMod *dmx;
  uint8_t sliderBuffer[512];
};

#endif

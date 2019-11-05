#ifndef _IOTSAESTIMOTE_H_
#define _IOTSAESTIMOTE_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include "iotsaDMX.h"

#ifdef IOTSA_WITH_API
#define IotsaEstimoteModBaseMod IotsaApiMod
#else
#define IotsaEstimoteModBaseMod IotsaMod
#endif

class IotsaEstimoteMod : public IotsaEstimoteModBaseMod {
public:
  using IotsaEstimoteModBaseMod::IotsaEstimoteModBaseMod;
  void setup();
  void serverSetup();
  void loop();
  String info();
  void setDMX(IotsaDMXMod *dmx);
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  String argument;
};

#endif

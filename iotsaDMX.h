#ifndef _IOTSADMX_H_
#define _IOTSADMX_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include <WiFiUdp.h>

#ifdef IOTSA_WITH_API
#define IotsaDMXModBaseMod IotsaApiMod
#else
#define IotsaDMXModBaseMod IotsaMod
#endif

class IotsaDMXHandler {
public:
  virtual ~IotsaDMXHandler() {};
  virtual void dmxCallback() = 0;
};

class IotsaDMXMod : public IotsaDMXModBaseMod {
public:
  IotsaDMXMod(IotsaApplication& app)
  : IotsaDMXModBaseMod(app),
    buffer(NULL),
    count(0),
    dmxHandler(NULL),
    shortName(""),
    longName(""),
    network(0),
    subnet(0),
    universe(0),
    firstIndex(0),
    udp()
  {}
  void setup();
  void serverSetup();
  void loop();
  String info();
  void setHandler(uint8_t *_buffer, size_t _count, IotsaDMXHandler *_dmxHandler);
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  void fillPollReply();
  uint8_t *buffer; 
  size_t count;
  IotsaDMXHandler *dmxHandler;
  String shortName;
  String longName;
  int network;
  int subnet;
  int universe;
  int firstIndex;
  WiFiUDP udp;
};

#endif

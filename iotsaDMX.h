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

class IotsaDMXOutputHandler {
public:
  virtual ~IotsaDMXOutputHandler() {};
  virtual void dmxOutputChanged() = 0;
};

class IotsaDMXMod : public IotsaDMXModBaseMod {
public:
  IotsaDMXMod(IotsaApplication& app)
  : IotsaDMXModBaseMod(app),
    buffer(NULL),
    count(0),
    inputBuffer(NULL),
    inputCount(0),
    dmxOutputHandler(NULL),
    shortName(""),
    longName(""),
    network(0),
    subnet(0),
    universe(0),
    firstIndex(0),
    inputIndex(-1),
    sendDMXPacket(false),
    sendAddress(255,255,255,255),
    udp()
  {}
  void setup();
  void serverSetup();
  void loop();
  String info();
  void setDMXOutputHandler(uint8_t *_buffer, size_t _count, IotsaDMXOutputHandler *_dmxHandler);
  void setDMXInputHandler(uint8_t *_buffer, size_t _count, int inputIndex);
  void dmxInputChanged();
protected:
  bool getHandler(const char *path, JsonObject& reply);
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply);
  void configLoad();
  void configSave();
  void handler();
  void fillPollReply();
  uint8_t *buffer; 
  size_t count;
  uint8_t *inputBuffer;
  size_t inputCount;
  IotsaDMXOutputHandler *dmxOutputHandler;
  String shortName;
  String longName;
  int network;
  int subnet;
  int universe;
  int firstIndex;
  int inputIndex;
  bool sendDMXPacket;
  IPAddress sendAddress;
  WiFiUDP udp;
};

#endif

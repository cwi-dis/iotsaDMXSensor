#ifndef _IOTSADMX_H_
#define _IOTSADMX_H_
#include "iotsa.h"
#include "iotsaApi.h"
#include <WiFiUdp.h>

class IotsaDMXOutputHandler {
public:
  virtual ~IotsaDMXOutputHandler() {};
  virtual void dmxOutputChanged() = 0;
};

class IotsaDMXMod : public IotsaModule {
public:
  IotsaDMXMod(IotsaApplication& app)
  : IotsaModule(app),
    outputPort(-1),
    outputFirstIndex(0),
    outputBuffer(NULL),
    outputCount(0),
    dmxOutputHandler(NULL),

    inputPort(-1),
    inputBuffer(NULL),
    inputCount(0),

    shortName(""),
    longName(""),
    firstUniverse(0),
    sendDMXPacket(false),
    sendAddress(255,255,255,255),
    packetSequence(0),
    udp()
  {}
  void setup() override;
  void lateSetup() override;
  void loop() override;
  String info() override;
  void setDMXOutputHandler(int outputPort, uint8_t *_buffer, size_t _count, IotsaDMXOutputHandler *_dmxHandler);
  void setDMXInputHandler(int inputPort, uint8_t *_buffer, size_t _count);
  void dmxInputChanged();
protected:
  bool getHandler(const char *path, JsonObject& reply) override;
  bool putHandler(const char *path, const JsonVariant& request, JsonObject& reply) override;
  void configLoad() override;
  void configSave() override;
  void webHandler() override;
  void fillPollReply();

  int outputPort;
  int outputFirstIndex;
  uint8_t *outputBuffer; 
  size_t outputCount;
  IotsaDMXOutputHandler *dmxOutputHandler;

  int inputPort;
  uint8_t *inputBuffer;
  size_t inputCount;

  String shortName;
  String longName;
  int firstUniverse;
  bool sendDMXPacket;
  IPAddress sendAddress;
  uint8_t packetSequence;
  WiFiUDP udp;
};

#endif

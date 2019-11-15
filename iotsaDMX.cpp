#include "iotsa.h"
#include "iotsaDMX.h"
#include "iotsaConfigFile.h"

#define ARTNET_PORT 6454

#pragma pack(push)
#pragma pack(1)
struct ArtnetPacket {
  char ident[8];
  uint16_t opcode;
  union {
    struct  {
      uint16_t protocolVersion;
    } pollRequest;
    struct {
      uint32_t ipaddr;
      uint16_t port;
      uint16_t version;
      uint8_t dmxNetwork;
      uint8_t dmxSubnet;
      uint16_t oem;
      uint8_t ubea;
      uint8_t status;
      uint16_t esta;
      char shortName[18];
      char longName[64];
      char report[64];
      uint16_t numPorts;
      uint8_t portType[4];
      uint8_t inputStatus[4];
      uint8_t outputStatus[4];
      uint8_t inputPort[4];
      uint8_t outputPort[4];
      uint8_t filler1[7];
      uint8_t mac[6];
      uint32_t bindip;
      uint8_t bindIndex;
      uint8_t status2;
      uint8_t filler2[26];
    } pollReply;
    struct {
      uint16_t protocolVersion;
      uint8_t seq;
      uint8_t physical;
      uint16_t universe;
      uint16_t length;
      uint8_t data[512];
    } data;
  };
} inPkt;
#pragma pack(pop)

#define ARTNET_MIN_PACKET_SIZE 10

struct ArtnetPacket outPkt = {
  .ident={'A', 'r', 't', '-', 'N', 'e', 't', 0},
  .opcode=0x2100,
};

#ifdef IOTSA_WITH_WEB
void
IotsaDMXMod::handler() {
  bool anyChanged = false;
  if (server->hasArg("shortName")) {
    if (needsAuthentication()) return;
    shortName = server->arg("shortName");
    anyChanged = true;
  }
  if (server->hasArg("longName")) {
    if (needsAuthentication()) return;
    longName = server->arg("longName");
    anyChanged = true;
  }
  if (server->hasArg("network")) {
    if (needsAuthentication()) return;
    network = server->arg("network").toInt();
    anyChanged = true;
  }
  if (server->hasArg("subnet")) {
    if (needsAuthentication()) return;
    subnet = server->arg("subnet").toInt();
    anyChanged = true;
  }
  if (server->hasArg("universe")) {
    if (needsAuthentication()) return;
    universe = server->arg("universe").toInt();
    anyChanged = true;
  }
  if (server->hasArg("firstIndex")) {
    if (needsAuthentication()) return;
    firstIndex = server->arg("firstIndex").toInt();
    anyChanged = true;
  }
  if (server->hasArg("sendAddress")) {
    IPAddress newSendAddress;
    if (newSendAddress.fromString(server->arg("sendAddress"))) {
      sendAddress = newSendAddress;
      anyChanged = true;
    }
  }

  if (anyChanged) {
    configSave();
    fillPollReply();
  }

  String message = "<html><head><title>Boilerplate module</title></head><body><h1>Boilerplate module</h1>";
  message += "<form method='get'>Short name: <input name='shortName' value='" + htmlEncode(shortName) + "'><br>";
  message += "Long name: <input name='longName' value='" + htmlEncode(longName) + "'><br>";
  message += "DMX Network: <input name='network' value='" + String(network) + "'><br>";
  message += "DMX Subnet: <input name='subnet' value='" + String(subnet) + "'><br>";
  message += "DMX Universe: <input name='universe' value='" + String(universe) + "'><br>";
  if (count) {
    message += "Number of dimmers: " + String(count) + "<br>";
    message += "Index of first dimmer within universe: <input name='firstIndex' value='" + String(firstIndex) + "'><br>";
  } else {
    message += "(No DMX output ports registered by this iotsa device)<br>";
  }
  if (inputCount) {
    message += "Number of sliders: " + String(inputCount) + "<br>";
    message += "Port number for sliders: " + String(firstIndex) + "<br>";
    message += "Universe for sliders: " + String(universe+firstIndex) + "<br>";
    message += "Transmission IP address: <input name='sendAddress' value='" + sendAddress.toString() + "'><br>";
  } else {
    message += "(No DMX input ports registered by this iotsa device)<br>";
  }
  message += "<input type='submit'></form>";
  server->send(200, "text/html", message);
}

String IotsaDMXMod::info() {
  String message = "<p>Built with Art-Net DMX module. See <a href=\"/dmx\">/dmx</a> to change the DMX parameters.</p>";
  return message;
}
#endif // IOTSA_WITH_WEB

void IotsaDMXMod::setup() {
  configLoad();
  fillPollReply();
}

#ifdef IOTSA_WITH_API
bool IotsaDMXMod::getHandler(const char *path, JsonObject& reply) {
  reply["shortName"] = shortName;
  reply["longName"] = longName;
  reply["network"] = network;
  reply["subnet"] = subnet;
  reply["universe"] = universe;
  reply["firstIndex"] = firstIndex;
  reply["sendAddress"] = sendAddress.toString();
  return true;
}

bool IotsaDMXMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  bool anyChanged = false;
  JsonObject& reqObj = request.as<JsonObject>();
  if (reqObj.containsKey("shortName")) {
    shortName = reqObj.get<String>("shortName");
    anyChanged = true;
  }
  if (reqObj.containsKey("longName")) {
    longName = reqObj.get<String>("longName");
    anyChanged = true;
  }
  if (reqObj.containsKey("network")) {
    network = reqObj.get<int>("network");
    anyChanged = true;
  }
  if (reqObj.containsKey("subnet")) {
    subnet = reqObj.get<int>("subnet");
    anyChanged = true;
  }
  if (reqObj.containsKey("universe")) {
    universe = reqObj.get<int>("universe");
    anyChanged = true;
  }
  if (reqObj.containsKey("firstIndex")) {
    firstIndex = reqObj.get<int>("firstIndex");
    anyChanged = true;
  }
  if (reqObj.containsKey("sendAddress")) {
    IPAddress newAddr;
    if (newAddr.fromString(reqObj.get<String>("sendAddress"))) {
      sendAddress = newAddr;
      anyChanged = true;
    }
  }
  if (anyChanged) {
    configSave();
    fillPollReply();
  }
  return anyChanged;
}
#endif // IOTSA_WITH_API

void IotsaDMXMod::serverSetup() {
#ifdef IOTSA_WITH_WEB
  server->on("/dmx", std::bind(&IotsaDMXMod::handler, this));
#endif
#ifdef IOTSA_WITH_API
  api.setup("/api/dmx", true, true);
  name = "dmx";
#endif
  udp.begin(ARTNET_PORT);
}

void IotsaDMXMod::configLoad() {
  IotsaConfigFileLoad cf("/config/dmx.cfg");
  cf.get("shortName", shortName, "iotsaDMXLedstrip");
  cf.get("longName", longName, "iotsaDMXLedstrip");
  cf.get("network", network, 0);
  cf.get("subnet", subnet, 0);
  cf.get("universe", universe, 0);
  cf.get("firstIndex", firstIndex, 0);
  String addrStr;
  cf.get("sendAddress", addrStr, "255.255.255.255");
  sendAddress.fromString(addrStr);
}

void IotsaDMXMod::configSave() {
  IotsaConfigFileSave cf("/config/dmx.cfg");
  cf.put("shortName", shortName);
  cf.put("longName", longName);
  cf.put("network", network);
  cf.put("subnet", subnet);
  cf.put("universe", universe);
  cf.put("firstIndex", firstIndex);
  cf.put("sendAddress", sendAddress.toString());
}

void IotsaDMXMod::setDMXOutputHandler(uint8_t *_buffer, size_t _count, IotsaDMXOutputHandler *_dmxOutputHandler) {
  buffer = _buffer;
  count = _count;
  dmxOutputHandler = _dmxOutputHandler;
  fillPollReply();
}

void IotsaDMXMod::setDMXInputHandler(uint8_t *_buffer, size_t _count, int _inputIndex) {
  inputBuffer = _buffer;
  inputCount = _count;
  inputIndex = _inputIndex;
  fillPollReply();
}

void IotsaDMXMod::fillPollReply() {
  uint32_t ip = WiFi.localIP();
  if (ip == 0) {
  	ip = WiFi.softAPIP();
  }
  outPkt.pollReply.ipaddr = ip;
  outPkt.pollReply.port=ARTNET_PORT;
  outPkt.pollReply.version=1;
  outPkt.pollReply.dmxNetwork = network;
  outPkt.pollReply.dmxSubnet = subnet;
  outPkt.pollReply.oem=0;
  outPkt.pollReply.ubea=0;
  outPkt.pollReply.status=0x10;
  outPkt.pollReply.esta=0x7ff0;
  strncpy(outPkt.pollReply.shortName, shortName.c_str(), sizeof(outPkt.pollReply.shortName));
  strncpy(outPkt.pollReply.longName, longName.c_str(), sizeof(outPkt.pollReply.longName));
  strncpy(outPkt.pollReply.report, "#0001 [0000] All is well", sizeof(outPkt.pollReply.report));
 
  int nPorts = 1;
  outPkt.pollReply.numPorts = ntohs(nPorts);
  memset(outPkt.pollReply.portType, 0, sizeof(outPkt.pollReply.portType));
  for (int i=0; i<nPorts; i++) {
    if (i == inputIndex) {
      outPkt.pollReply.portType[i] = 0x40; // DMX512->Artnet
    } else if (count && i <= count/512) {
      outPkt.pollReply.portType[i] = 0x80;  // Artnet->DMX512
    }
  }
  memset(outPkt.pollReply.inputStatus, 0, sizeof(outPkt.pollReply.inputStatus));
  memset(outPkt.pollReply.outputStatus, 0, sizeof(outPkt.pollReply.outputStatus));
  memset(outPkt.pollReply.inputPort, 0, sizeof(outPkt.pollReply.inputPort));
  if (inputIndex >= 0) {
    outPkt.pollReply.inputPort[inputIndex] = universe + inputIndex;
  }
  memset(outPkt.pollReply.outputPort, 0, sizeof(outPkt.pollReply.outputPort));
  for (int i=0; i<nPorts; i++) {
    outPkt.pollReply.outputPort[0] = universe+i;
  }

  memset(outPkt.pollReply.mac, 0, sizeof(outPkt.pollReply.mac));
  outPkt.pollReply.bindip = ip;
  outPkt.pollReply.bindIndex=0;
  outPkt.pollReply.status2=0x09;

}

void IotsaDMXMod::dmxInputChanged() {
  sendDMXPacket = true;
}

void IotsaDMXMod::loop() {
  size_t packetSize = udp.parsePacket();
  if (packetSize > ARTNET_MIN_PACKET_SIZE) {
    if (packetSize > sizeof(inPkt)) packetSize = sizeof(inPkt);
    if (udp.read((char *)&inPkt, sizeof(inPkt)) != (int)packetSize) {
      IFDEBUG IotsaSerial.println("Ignoring incomplete packet");
      return;
    }
    if (strcmp(inPkt.ident, "Art-Net") != 0 || ntohs(inPkt.pollRequest.protocolVersion) != 14) {
      IFDEBUG IotsaSerial.print("Ignoring unknown packet, hdr=");
      IFDEBUG IotsaSerial.print(inPkt.ident);
      IFDEBUG IotsaSerial.print(", version=");
      IFDEBUG IotsaSerial.println(inPkt.pollRequest.protocolVersion);
      return;
    }
    uint16_t opcode = inPkt.opcode;
    if (opcode == 0x5000) {
      if (inPkt.data.universe != universe) {
        //IFDEBUG IotsaSerial.print("Ignore data for universe=");
        //IFDEBUG IotsaSerial.println(inPkt.data.universe);
        return;
      }
      if (buffer == NULL || count == 0 || dmxOutputHandler == NULL) {
        IFDEBUG IotsaSerial.println("Ignore data, no buffer/handler set");
      }
      bool anyChange = false;
      int nValues = ntohs(inPkt.data.length);
      if ((int)count < nValues) nValues = count;
      IFDEBUG IotsaSerial.printf("xxxjack count=%d\r\n", nValues);
      for(int i=0; i<nValues; i++) {
        if (inPkt.data.data[i] != buffer[i]) {
          buffer[i] = inPkt.data.data[i];
          anyChange = true;
        }
      }
      if (anyChange) {
        IFDEBUG IotsaSerial.println("Data, and callback");
        dmxOutputHandler->dmxOutputChanged();
      }
    } else if (opcode == 0x2000) {
      IFDEBUG IotsaSerial.println("Poll packet");
      udp.beginPacket(udp.remoteIP(), 6454);
      udp.write((uint8_t *)&outPkt, sizeof(outPkt));
      udp.endPacket();
    } else if (opcode == 0x2100) {
      IFDEBUG IotsaSerial.println("PollReply packet");
    } else {
      IFDEBUG IotsaSerial.print("Ignoring packet opcode=");
      IFDEBUG IotsaSerial.println(opcode, HEX);
    }
  }
  if (sendDMXPacket) {
    // We re-use inPkt, we don't want to overwrite outPkt which has the poll reply.
    memcpy(inPkt.ident, (const void *)"Art-Net", 8);
    inPkt.opcode = 0x5000;
    inPkt.data.universe = universe + inputIndex;
    inPkt.data.physical = inputIndex;
    inPkt.data.protocolVersion = htons(14);
    inPkt.data.seq = 0;
    inPkt.data.length = htons(inputCount);
    memcpy(inPkt.data.data, inputBuffer, inputCount);
    udp.beginPacket(sendAddress, ARTNET_PORT);
    size_t dataSize = sizeof(inPkt)-512+inputCount; 
    udp.write((uint8_t *)&inPkt, dataSize);
    udp.endPacket();
    IFDEBUG IotsaSerial.printf("sent %d byte DMX packet to %s\n", dataSize, sendAddress.toString().c_str());
    sendDMXPacket = false;
    // xxxjack should also send periodically
  }
}

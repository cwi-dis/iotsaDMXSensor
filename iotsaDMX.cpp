#include "iotsa.h"
#include "iotsaDMX.h"
#include "iotsaConfigFile.h"

#define ARTNET_PORT 6454

#ifndef ESP32
// It seems that on the esp8266 udp.flush() actually does the same as endPacket(),
// i.e. it writes out a packet (so it behaves like output flush, not input flush).
#define UDP_FLUSH_IS_BROKEN
#endif

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

// How the device sits in the DMX network, the same on the page and in the
// API. Changing it needs "dmx" (lateSetup()): it is operational, not how the
// device is built. The names go into the Art-Net poll reply, which has room
// for 17 and 63 characters.
template<class Ctx> void IotsaDMXMod::_args(Ctx& ctx) {
  String newShortName = shortName;
  String newLongName = longName;
  int newFirstIndex = outputFirstIndex;
  String newSendAddress = sendAddress.toString();
  bool changed = ctx.arg("shortName", newShortName, IotsaContext::Saved);
  changed |= ctx.arg("longName", newLongName, IotsaContext::Saved);
  changed |= ctx.arg("universe", firstUniverse, IotsaContext::Saved);
  changed |= ctx.arg("firstIndex", newFirstIndex, IotsaContext::Saved);
  bool addressChanged = ctx.arg("sendAddress", newSendAddress, IotsaContext::Saved);
  IPAddress newAddr;
  if (newShortName.length() > sizeof(outPkt.pollReply.shortName)-1) ctx.error(400, "shortName: at most 17 characters");
  if (newLongName.length() > sizeof(outPkt.pollReply.longName)-1) ctx.error(400, "longName: at most 63 characters");
  if (newFirstIndex < 0 || newFirstIndex > 511) ctx.error(400, "firstIndex: 0 to 511");
  if (addressChanged && !newAddr.fromString(newSendAddress)) ctx.error(400, "sendAddress: an IP address");
  if (ctx.failed()) return;
  shortName = newShortName;
  longName = newLongName;
  outputFirstIndex = newFirstIndex;
  if (addressChanged) sendAddress = newAddr;
  if (changed || addressChanged) fillPollReply();
}

#ifdef IOTSA_WITH_WEB
void IotsaDMXMod::formHandler_args(IotsaWebContext& ctx) {
  _args(ctx);
}

void IotsaDMXMod::formHandler_fields(IotsaWebContext& ctx) {
  if (ctx.includesEveryday()) {
    int network = (firstUniverse >> 8) & 0x7f;
    int subnet = (firstUniverse >> 4) & 0xf;
    ctx.showStatus("Network and subnet", "", String(network) + ", " + String(subnet) + " (from the universe)");
    if (outputCount) {
      String universes;
      for (size_t i=0; i<outputCount; i += 512) {
        universes += String(firstUniverse + (i/512)) + " ";
      }
      ctx.showStatus("Dimmers", "", String(outputCount) + " on port " + String(outputPort) + ", universe(s) " + universes);
    } else {
      ctx.showStatus("Dimmers", "", "none: no DMX output ports on this device");
    }
    if (inputCount) {
      ctx.showStatus("Sliders", "", String(inputCount) + " on port " + String(inputPort) + ", universe " + String(firstUniverse+inputPort));
    } else {
      ctx.showStatus("Sliders", "", "none: no DMX input ports on this device");
    }
    return;
  }
  IotsaWebContext::Form form(ctx);
  ctx.showField("Short name", "shortName", shortName, IotsaWebContext::Saved, "at most 17 characters");
  ctx.showField("Long name", "longName", longName, IotsaWebContext::Saved, "at most 63 characters");
  ctx.showField("Universe of first port", "universe", firstUniverse, IotsaWebContext::Saved, "later ports get the next universes");
  if (outputCount) ctx.showField("Index of first dimmer within universe", "firstIndex", outputFirstIndex, IotsaWebContext::Saved, "0 to 511");
  if (inputCount) ctx.showField("Transmission IP address", "sendAddress", sendAddress.toString(), IotsaWebContext::Saved);
}
#endif // IOTSA_WITH_WEB

String IotsaDMXMod::info() {
  String message = "<p>Built with Art-Net DMX module. See <a href=\"/dmx\">/dmx</a> to change the DMX parameters.</p>";
  return message;
}

void IotsaDMXMod::setup() {
  configLoad();
  fillPollReply();
}

void IotsaDMXMod::getHandler(IotsaApiContext& ctx) {
  ctx.show("shortName", shortName);
  ctx.show("longName", longName);
  ctx.show("universe", firstUniverse);
  ctx.show("firstIndex", outputFirstIndex);
  ctx.show("sendAddress", sendAddress.toString());
}

void IotsaDMXMod::putHandler(IotsaApiContext& ctx) {
  _args(ctx);
}

void IotsaDMXMod::lateSetup() {
  api.setup("dmx", true, true);
  name = "dmx";
  changesNeedRight("dmx");
  udp.begin(ARTNET_PORT);
}

void IotsaDMXMod::configLoad() {
  IotsaConfigFileLoad cf("/config/dmx.cfg");
  // What DMX controllers see in the poll reply; by default the device's name.
  cf.get("shortName", shortName, iotsaConfig.hostName.substring(0, sizeof(outPkt.pollReply.shortName)-1));
  cf.get("longName", longName, iotsaConfig.hostName.substring(0, sizeof(outPkt.pollReply.longName)-1));
  cf.get("universe", firstUniverse, 0);
  cf.get("firstIndex", outputFirstIndex, 0);
  String addrStr;
  cf.get("sendAddress", addrStr, "255.255.255.255");
  sendAddress.fromString(addrStr);
}

void IotsaDMXMod::configSave() {
  IotsaConfigFileSave cf("/config/dmx.cfg");
  cf.put("shortName", shortName);
  cf.put("longName", longName);
  cf.put("universe", firstUniverse);
  cf.put("firstIndex", outputFirstIndex);
  cf.put("sendAddress", sendAddress.toString());
}

void IotsaDMXMod::setDMXOutputHandler(int portIndex, uint8_t *_buffer, size_t _count, IotsaDMXOutputHandler *_dmxOutputHandler) {
  outputBuffer = _buffer;
  outputCount = _count;
  dmxOutputHandler = _dmxOutputHandler;
  outputPort = portIndex;
  fillPollReply();
}

void IotsaDMXMod::setDMXInputHandler(int portIndex, uint8_t *_buffer, size_t _count) {
  inputBuffer = _buffer;
  inputCount = _count;
  inputPort = portIndex;
  fillPollReply();
}

void IotsaDMXMod::fillPollReply() {
  uint32_t ip = WiFi.localIP();
  if (ip == 0) {
  	ip = WiFi.softAPIP();
  }
  int network = (firstUniverse >> 8) & 0x7f;
  int subnet = (firstUniverse >> 4) & 0xf;
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
 
  int firstOutputPort = outputPort;
  int lastOutputPort = outputPort + (outputCount / 512);
  int nPorts = lastOutputPort+1;
  if (inputPort+1 > nPorts) nPorts = inputPort+1;
  outPkt.pollReply.numPorts = ntohs(nPorts);
  memset(outPkt.pollReply.portType, 0, sizeof(outPkt.pollReply.portType));
  memset(outPkt.pollReply.inputPort, 0, sizeof(outPkt.pollReply.inputPort));
  memset(outPkt.pollReply.outputPort, 0, sizeof(outPkt.pollReply.outputPort));
  for (int i=0; i<nPorts; i++) {
    uint8_t portType = 0;
    if (i == inputPort) {
      portType |= 0x40; // DMX512->Artnet
      outPkt.pollReply.inputPort[i] = (firstUniverse + i) & 0xf;
    } else if (i >= firstOutputPort && i <= lastOutputPort) {
      portType |= 0x80;  // Artnet->DMX512
      outPkt.pollReply.outputPort[i] = (firstUniverse + i) & 0xf;
    }
    outPkt.pollReply.portType[i] = portType;
  }
  memset(outPkt.pollReply.inputStatus, 0, sizeof(outPkt.pollReply.inputStatus));
  memset(outPkt.pollReply.outputStatus, 0, sizeof(outPkt.pollReply.outputStatus));

  memset(outPkt.pollReply.mac, 0, sizeof(outPkt.pollReply.mac));
  outPkt.pollReply.bindip = ip;
  outPkt.pollReply.bindIndex=0;
  outPkt.pollReply.status2=0x09;

}

void IotsaDMXMod::dmxInputChanged() {
  sendDMXPacket = true;
}

void IotsaDMXMod::loop() {
  //
  // Do we want to transmit anything?
  //
  if (sendDMXPacket) {
    // We re-use inPkt, we don't want to overwrite outPkt which has the poll reply.
    memcpy(inPkt.ident, (const void *)"Art-Net", 8);
    inPkt.opcode = 0x5000;
    inPkt.data.universe = firstUniverse + inputPort;
    inPkt.data.physical = inputPort;
    inPkt.data.protocolVersion = htons(14);
    inPkt.data.seq = packetSequence++;
    inPkt.data.length = htons(inputCount);
    memcpy(inPkt.data.data, inputBuffer, inputCount);
    udp.beginPacket(sendAddress, ARTNET_PORT);
    size_t dataSize = sizeof(inPkt)-512+inputCount; 
    udp.write((uint8_t *)&inPkt, dataSize);
    udp.endPacket();
    IOTSA_LOG_DEBUG("dmx", "sent %d byte DMX packet to %s", dataSize, sendAddress.toString().c_str());
    sendDMXPacket = false;
    // xxxjack should also send periodically
  }
  //
  // Did we receive anything?
  //
  size_t packetSize = udp.parsePacket();
  if (packetSize < ARTNET_MIN_PACKET_SIZE) {
#ifndef UDP_FLUSH_IS_BROKEN
    udp.flush();
#endif
    return;
  }
  if (packetSize > sizeof(inPkt)) packetSize = sizeof(inPkt);
  if (udp.read((char *)&inPkt, sizeof(inPkt)) != (int)packetSize) {
    IOTSA_LOG_DEBUG("dmx", "ignoring incomplete packet");
#ifndef UDP_FLUSH_IS_BROKEN
    udp.flush();
#endif
    return;
  }
#ifndef UDP_FLUSH_IS_BROKEN
  udp.flush();
#endif
  if (strcmp(inPkt.ident, "Art-Net") != 0 || ntohs(inPkt.pollRequest.protocolVersion) != 14) {
    IOTSA_LOG_DEBUG("dmx", "ignoring unknown packet, hdr=%.8s, version=%d", inPkt.ident, ntohs(inPkt.pollRequest.protocolVersion));
    return;
  }
  uint16_t opcode = inPkt.opcode;
  if (opcode == 0x5000) {
    if (inPkt.data.universe != firstUniverse) {
      // xxxjack this is wrong. It doesn't allow for multiple ports...
      //IFDEBUG IotsaSerial.print("Ignore data for universe=");
      //IFDEBUG IotsaSerial.println(inPkt.data.universe);
      return;
    }
    if (outputBuffer == NULL || outputCount == 0 || dmxOutputHandler == NULL) {
      IOTSA_LOG_DEBUG("dmx", "ignoring data, no output buffer/handler set");
      return;
    }
    bool anyChange = false;
    // Our dimmers start at outputFirstIndex within the universe.
    int nValues = ntohs(inPkt.data.length) - outputFirstIndex;
    if (nValues > (int)sizeof(inPkt.data.data) - outputFirstIndex) nValues = sizeof(inPkt.data.data) - outputFirstIndex;
    if ((int)outputCount < nValues) nValues = outputCount;
    for(int i=0; i<nValues; i++) {
      uint8_t value = inPkt.data.data[outputFirstIndex + i];
      if (value != outputBuffer[i]) {
        outputBuffer[i] = value;
        anyChange = true;
      }
    }
    if (anyChange) {
      IOTSA_LOG_DEBUG("dmx", "data, and callback");
      dmxOutputHandler->dmxOutputChanged();
    }
  } else if (opcode == 0x2000) {
    IOTSA_LOG_DEBUG("dmx", "poll packet");
    udp.beginPacket(udp.remoteIP(), 6454);
    udp.write((uint8_t *)&outPkt, sizeof(outPkt));
    udp.endPacket();
  } else if (opcode == 0x2100) {
    IOTSA_LOG_DEBUG("dmx", "pollReply packet");
  } else {
    IOTSA_LOG_DEBUG("dmx", "ignoring packet opcode=0x%x", opcode);
  }
}

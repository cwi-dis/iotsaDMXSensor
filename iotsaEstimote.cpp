#include "iotsa.h"
#include "iotsaEstimote.h"
#include "iotsaConfigFile.h"

static bool isScanning;
static uint32_t dontScanBefore;
static bool continueScanning = false;

static void scanCompleteCB(BLEScanResults results) {
  dontScanBefore = millis() + 1000;
  continueScanning = true;
  isScanning = false;
}

#ifdef IOTSA_WITH_WEB
void
IotsaEstimoteMod::handler() {
  bool anyChanged = false;
  if( server->hasArg("argument")) {
    if (needsAuthentication()) return;
    argument = server->arg("argument");
    anyChanged = true;
  }
  if (anyChanged) configSave();

  String message = "<html><head><title>Estimote module</title></head><body><h1>Estimote module</h1>";
  message += "<form method='get'>Argument: <input name='argument' value='";
  message += htmlEncode(argument);
  message += "'><br><input type='submit'></form>";
  server->send(200, "text/html", message);
}

String IotsaEstimoteMod::info() {
  String message = "<p>Built with estimote module. See <a href=\"/estimote\">/estimote</a> to change devices and settings.</p>";
  return message;
}
#endif // IOTSA_WITH_WEB

void IotsaEstimoteMod::setup() {
  configLoad();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(this);
  isScanning = false;
#if 0
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
#endif

}

#ifdef IOTSA_WITH_API
bool IotsaEstimoteMod::getHandler(const char *path, JsonObject& reply) {
  reply["argument"] = argument;
  return true;
}

bool IotsaEstimoteMod::putHandler(const char *path, const JsonVariant& request, JsonObject& reply) {
  bool anyChanged = false;
  JsonObject& reqObj = request.as<JsonObject>();
  if (reqObj.containsKey("argument")) {
    argument = reqObj.get<String>("argument");
    anyChanged = true;
  }
  if (anyChanged) configSave();
  return anyChanged;
}
#endif // IOTSA_WITH_API

void IotsaEstimoteMod::serverSetup() {
#ifdef IOTSA_WITH_WEB
  server->on("/estimote", std::bind(&IotsaEstimoteMod::handler, this));
#endif
#ifdef IOTSA_WITH_API
  api.setup("/api/estimote", true, true);
  name = "estimote";
#endif
}

void IotsaEstimoteMod::configLoad() {
  IotsaConfigFileLoad cf("/config/estimote.cfg");
  cf.get("argument", argument, "");
 
}

void IotsaEstimoteMod::configSave() {
  IotsaConfigFileSave cf("/config/estimote.cfg");
  cf.put("argument", argument);
}

void IotsaEstimoteMod::setDMX(IotsaDMXMod *dmx) {
  
}

void IotsaEstimoteMod::loop() {
  if (!isScanning && millis() > dontScanBefore) {
    IFDEBUG IotsaSerial.print("SCAN ");
    isScanning = pBLEScan->start(1, scanCompleteCB, continueScanning);
    IFDEBUG IotsaSerial.println("started");
#if 0
    IFDEBUG IotsaSerial.print("Devices found: ");
    IFDEBUG IotsaSerial.println(foundDevices.getCount());
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    noScanBefore = millis() + 2000;
#endif
  }
}

void IotsaEstimoteMod::onResult(BLEAdvertisedDevice advertisedDevice) {
  IFDEBUG IotsaSerial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
}
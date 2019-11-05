#include "iotsa.h"
#include "iotsaEstimote.h"
#include "iotsaConfigFile.h"

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
}

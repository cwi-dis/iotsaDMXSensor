//
// Listens for Estimote sensors over BLE and turns their readings into DMX
// slider values, transmitted over Art-Net (DMX-over-IP).
//

#include "iotsa.h"
#include "iotsaWifi.h"
#include "iotsaLed.h"
#include "iotsaConfigFile.h"
#include "iotsaDMX.h"
#include "iotsaEstimote.h"

// CHANGE: Add application includes and declarations here

#define WITH_OTA    // Enable Over The Air updates from ArduinoIDE. Needs at least 1MB flash.

IotsaApplication application("Iotsa DMX Sensor Server");
IotsaWifiMod wifiMod(application);

#ifdef WITH_OTA
#include "iotsaOta.h"
IotsaOtaMod otaMod(application);
#endif

IotsaDMXMod dmxMod(application);
IotsaEstimoteMod estimoteMod(application);

// Standard setup() method, hands off most work to the application framework
void setup(void){
  estimoteMod.setDMX(&dmxMod, 0); // Transmit sensor values as sliders on port 0
  application.setup();
  application.serverSetup();
}
 
// Standard loop() routine, hands off most work to the application framework
void loop(void){
  application.loop();
}


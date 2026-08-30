//
// Listens for Estimote sensors over BLE and turns their readings into DMX
// slider values, transmitted over Art-Net (DMX-over-IP).
//

#include "iotsa.h"
#include "iotsaWifi.h"
#include "iotsaOta.h"
#include "iotsaDMX.h"
#include "iotsaEstimote.h"

IotsaApplication application("Iotsa DMX Sensor Server");
IotsaWifiMod wifiMod(application);
IotsaOtaMod otaMod(application);        // OTA firmware update

IotsaDMXMod dmxMod(application);
IotsaEstimoteMod estimoteMod(application);

// Standard setup() method, hands off most work to the application framework
void setup(void){
  estimoteMod.setDMX(&dmxMod, 0); // Transmit sensor values as sliders on port 0
  application.setup();
  application.lateSetup();
}

// Standard loop() routine, hands off most work to the application framework
void loop(void){
  application.loop();
}

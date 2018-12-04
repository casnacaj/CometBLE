/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include "CometBLE.hpp"

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  Serial.println(CometBLE::scan(10).c_str());

  CometBLE pComet = CometBLE("54:77:37:05:4A:81");

  pComet.getDeviceName();
  pComet.getDateTime();
  pComet.getTemperatures();
  pComet.getBattery();
  pComet.setTemperature(56);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}

#include "Arduino.h"
#include "coyote.h"

#include <NimBLEDevice.h>
#include <memory>

enum scan_callback_result { SCAN_None, SCAN_Coyote };
extern short debug_mode;

enum scan_callback_result check_scan_data(const char* ble_manufacturer_specific_data, int length, int rssi) {
  if (length > 2 && ble_manufacturer_specific_data[1] == 0x19 && ble_manufacturer_specific_data[0] == 0x96) {
    Serial.printf("Found DG-LAB\n");
    return SCAN_Coyote;
  }
  return SCAN_None;
}

NimBLEServer *pServer = nullptr;
NimBLECharacteristic * pTxCharacteristic;
bool device_connected = false;
NimBLEScan* pBLEScan;
int scanTime = 5; //In seconds

NimBLEAdvertisedDevice* coyote_device = nullptr;
extern std::unique_ptr<Coyote> coyote_controller;

class BubblerAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
      //Serial.printf("Advertised Device: %s \n", advertisedDevice->toString().c_str());
      auto res = check_scan_data(advertisedDevice->getManufacturerData().c_str(), advertisedDevice->getManufacturerData().length(), advertisedDevice->getRSSI());
      if ( res == SCAN_Coyote ) {
        // can't connect while scanning is going on - it locks up everything.
        coyote_device = new NimBLEAdvertisedDevice(*advertisedDevice);
        NimBLEDevice::getScan()->stop();
      }
    }
};

void comms_init(short myid) {
  NimBLEDevice::init("x");
  NimBLEDevice::setPower(ESP_PWR_LVL_P6, ESP_BLE_PWR_TYPE_ADV); // send advertisements with 6 dbm
  pBLEScan = NimBLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new BubblerAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
  pBLEScan->setInterval(250);
  pBLEScan->setWindow(125);  // less or equal setInterval value
  Serial.println("Started ble scanning");
}

void comms_stop_scan() {
  Serial.println("Stopped ble scanning");
}

void scan_loop() {
  if ( !coyote_controller || !coyote_controller->get_isconnected() ) {
    NimBLEScanResults foundDevices = pBLEScan->start(scanTime, false);

    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

    if ( coyote_device ) {
      coyote_controller->connect_to_device(coyote_device);
      delete coyote_device;
      coyote_device = nullptr;      
    }
  } else {
    // everything is happening in callbacks / timers
    delay(1000);
  }
}

void comms_uart_colorpicker(void) {
  if (Serial.available()<1)
    return;

  int command = Serial.read();
  if (command != '!')
     return;

  command = Serial.read();
  if (command == 'R') { 
      Serial.printf("rebooting\n");
      delay(100);
      ESP.restart();
  } else if (command == 'D') {
    command = Serial.read();
    debug_mode = command - '0';
    Serial.printf("ok\n");
  }
}


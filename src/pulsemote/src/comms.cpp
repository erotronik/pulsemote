#include "Arduino.h"
#include "coyote.h"

#ifdef ESP32
#include "NimBLEDevice.h"
#endif /* ESP32 */
#ifndef ESP32
#include <bluefruit.h>
#endif

enum scan_callback_result { None, Coyote };
extern short debug_mode;

enum scan_callback_result check_scan_data(const char* ble_manufacturer_specific_data, int length, int rssi) {
  if (length > 2 && ble_manufacturer_specific_data[1] == 0x19 && ble_manufacturer_specific_data[0] == 0x96) {
    Serial.printf("Found DG-LAB\n");
    return Coyote;
  }
  return None;
}

#ifndef ESP32
BLEUart bleuart;

void comms_init(short myid) {
  Bluefruit.setTxPower(4);
  Bluefruit.setName("x");
  Bluefruit.Periph.setConnectCallback(comms_connect_callback);
  bleuart.begin();

  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterRssi(-128);
  Bluefruit.Scanner.setInterval(400, 200);   // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(false); // only need an rssi
  Bluefruit.Scanner.start(0); // 0 = Don't stop scanning after n seconds

  coyote_setup();
}

void scan_callback(ble_gap_evt_adv_report_t *report)
{
  uint8_t buffer[32];
  uint8_t len = Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_MANUFACTURER_SPECIFIC_DATA, buffer, sizeof(buffer));

  auto res = check_scan_data((char*)buffer, len, report->rssi);

  if ( res == Coyote )
    Bluefruit.Central.connect(report);
    Bluefruit.Scanner.resume();
}

#else /* ESP32 */

#include <NimBLEDevice.h>

NimBLEServer *pServer = NULL;
NimBLECharacteristic * pTxCharacteristic;
bool device_connected = false;
NimBLEScan* pBLEScan;
int scanTime = 5; //In seconds

NimBLEAdvertisedDevice* coyote_device = nullptr;

class BubblerAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
      //Serial.printf("Advertised Device: %s \n", advertisedDevice->toString().c_str());
      auto res = check_scan_data(advertisedDevice->getManufacturerData().c_str(), advertisedDevice->getManufacturerData().length(), advertisedDevice->getRSSI());
      if ( res == Coyote ) {
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
  if ( !coyote_get_isconnected()) {
    NimBLEScanResults foundDevices = pBLEScan->start(scanTime, false);

    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

    if ( coyote_device ) {
      connect_to_coyote(coyote_device);
      delete coyote_device;
      coyote_device = nullptr;      
    }
  } else {
    // everything is happening in callbacks / timers
    delay(1000);
  }
}
#endif

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
#ifndef ESP32
      NVIC_SystemReset();
#else
      ESP.restart();
#endif
  } else if (command == 'D') {
    command = Serial.read();
    debug_mode = command - '0';
    Serial.printf("ok\n");
  }
}


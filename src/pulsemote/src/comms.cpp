#include "coyote.hpp"

#include <NimBLEDevice.h>
#include <memory>
#include <esp_log.h>

NimBLEServer *pServer = nullptr;
NimBLECharacteristic * pTxCharacteristic;
NimBLEScan* pBLEScan;

bool has_init = false;
bool device_connected = false;
int scanTime = 5 * 1000; // In ms

NimBLEAdvertisedDevice* coyote_device = nullptr;
extern std::unique_ptr<Coyote> coyote_controller;

class PulsemoteAdvertisedDeviceCallbacks: public NimBLEScanCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
      ESP_LOGD("comms", "Advertised Device: %s", advertisedDevice->toString().c_str());
      if (Coyote::is_coyote(advertisedDevice)) {
        // can't connect while scanning is going on - it locks up everything.
        coyote_device = new NimBLEAdvertisedDevice(*advertisedDevice);
        NimBLEDevice::getScan()->stop();
      }
    }
};

void comms_init(short myid) {
  NimBLEDevice::init("x");
  NimBLEDevice::setPower(ESP_PWR_LVL_P6, ESP_BLE_PWR_TYPE_ADV); // send advertisements with 6 dbm
  pBLEScan = NimBLEDevice::getScan(); // create new scan
  pBLEScan->setScanCallbacks(new PulsemoteAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // active scan uses more power, but get results faster
  pBLEScan->setInterval(250);
  pBLEScan->setWindow(125);  // less or equal setInterval value
  ESP_LOGD("comms", "Started ble scanning");
  has_init = true;
}

void comms_stop_scan() {
  ESP_LOGI("comms", "Stopped ble scanning");
}

void scan_loop() {
  if ( !has_init ) {
    comms_init(0);
  }

  if ( !coyote_controller || !coyote_controller->get_isconnected() ) {
    ESP_LOGI("comms", "Scanning");
    pBLEScan->start(scanTime, false);

    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

    if ( coyote_device ) {
      coyote_controller->connect_to_device(coyote_device);
      delete coyote_device;
      coyote_device = nullptr;      
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  } else {
    // everything is happening in callbacks / timers
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/*
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
  }
} */
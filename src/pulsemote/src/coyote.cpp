// DG-Lab estim box support.
// References:
// https://rezreal.github.io/coyote/web-bluetooth-example.html
// https://github.com/OpenDGLab/OpenDGLab-Connect/blob/master/src/services/DGLab.js

#include <functional>

#include "coyote.h"
#include "coyote-modes.h"

constexpr uint8_t COYOTE_SERVICE_UUID[] = { 0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x0b, 0x18, 0x5a, 0x95 };
constexpr uint8_t CONFIG_CHAR_UUID[] = { 0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x07, 0x15, 0x5a, 0x95 };
constexpr uint8_t POWER_CHAR_UUID[] = { 0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x04, 0x15, 0x5a, 0x95 };
constexpr uint8_t A_CHAR_UUID[] = { 0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x06, 0x15, 0x5a, 0x95 };
constexpr uint8_t B_CHAR_UUID[] = { 0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x05, 0x15, 0x5a, 0x95 };
constexpr uint8_t BATTERY_UUID[] = { 0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x0a, 0x18, 0x5a, 0x95 };
constexpr uint8_t BATTERY_CHAR_UUID[] = { 0xad, 0xe8, 0xf3, 0xd4, 0xb8, 0x84, 0x94, 0xa0, 0xaa, 0xf5, 0xe2, 0x0f, 0x00, 0x15, 0x5a, 0x95 };

NimBLEUUID COYOTE_SERVICE_BLEUUID(COYOTE_SERVICE_UUID, 16, false);
NimBLEUUID CONFIG_CHAR_BLEUUID(CONFIG_CHAR_UUID, 16, false);
NimBLEUUID POWER_CHAR_BLEUUID(POWER_CHAR_UUID, 16, false);
NimBLEUUID A_CHAR_BLEUUID(A_CHAR_UUID, 16, false);
NimBLEUUID B_CHAR_BLEUUID(B_CHAR_UUID, 16, false);
NimBLEUUID BATTERY_SERVICE_BLEUUID(BATTERY_UUID, 16, false);
NimBLEUUID BATTERY_CHAR_BLEUUID(BATTERY_CHAR_UUID, 16, false);

std::map<TimerHandle_t, Coyote*> coyote_timer_map;

boolean Coyote::get_isconnected() {
  return coyote_connected;
}

// go up or down in steps of 1%, but never above 70% of max
void Coyote::put_powerup(short a, short b) {
  if (a > 5 || b > 5) return;  // max 5% in one go, sanity check
  int pc = coyote_maxPower / 100;
  int max = coyote_maxPower * .70;  // HARD LIMIT HERE just for sanity
  if (a < 0 && coyote_powerAwanted < -a * pc) return;
  if (b < 0 && coyote_powerBwanted < -b * pc) return;
  coyote_powerAwanted += a * pc;
  coyote_powerBwanted += b * pc;
  if (coyote_powerAwanted > max) {
    coyote_powerAwanted = max;
  }
  if (coyote_powerBwanted > max) {
    coyote_powerBwanted = max;
  }
}

int Coyote::get_powera_pc() {
  return int(coyote_powerA * 100 / coyote_maxPower);
}

int Coyote::get_powerb_pc() {
  return int(coyote_powerB * 100 / coyote_maxPower);
}

uint8_t Coyote::get_batterylevel() {
  return coyote_batterylevel;
}

short Coyote::get_modea() {
  return wantedmodea;
}

short Coyote::get_modeb() {
  return wantedmodeb;
}

void Coyote::put_setmode(short a, short b) {
  wantedmodea = a;
  wantedmodeb = b;
  Serial.printf("Set WaveMode %d %d\n", a, b);
}

void Coyote::set_callback(coyote_callback c) {
  update_callback = c;
}

// needs rewriting if we add more than one mode
void Coyote::put_toggle() {
  put_setmode(1 - wantedmodea, 1 - wantedmodeb);
}

void Coyote::notify (coyote_type_of_change change) {
  if ( update_callback )
    update_callback(change);
}

void Coyote::parse_power(const uint8_t* buf) {
  // notify/write: 3 bytes: flipFirstAndThirdByte(zero(2) ~ uint(11).as("powerLevelB") ~uint(11).as("powerLevelA")
  coyote_powerA = (buf[2] * 256 + buf[1]) >> 3;
  coyote_powerB = (buf[1] * 256 + buf[0]) & 0b0000011111111111;
  Serial.printf("coyote power A=%d (%d%%) B=%d (%d%%)\n", coyote_powerA, int(coyote_powerA * 100 / coyote_maxPower), coyote_powerB, int(coyote_powerB * 100 / coyote_maxPower));
}

void Coyote::encode_power(uint8_t* buf, int xpowerA, int xpowerB) {
  // notify/write: 3 bytes: flipFirstAndThirdByte(zero(2) ~ uint(11).as("powerLevelB") ~uint(11).as("powerLevelA")
  buf[2] = (xpowerA & 0b11111100000) >> 5;
  buf[1] = (xpowerA & 0b11111) << 3 | (xpowerB & 0b11100000000) >> 8;
  buf[0] = xpowerB & 0xff;
  Serial.printf("coyote power A=%d B=%d\n", xpowerA, xpowerB);
}

void Coyote::encode_pattern(uint8_t* buf, int ax, int ay, int az) {
  // flipFirstAndThirdByte(zero(4) ~ uint(5).as("az") ~ uint(10).as("ay") ~ uint(5).as("ax"))
  buf[2] = (az & 0b11111) >> 1;
  buf[1] = ((az & 0b1) * 128) | ((ay & 0b1111111000) >> 3);
  buf[0] = (ax & 0b11111) | ((ay & 0b111) << 5);
  //Serial.printf("encode pattern %02x %02x %02x ax=%d ay=%d az=%d\n", buf[2],buf[1],buf[0],ax,ay,az);
}

void Coyote::parse_pattern(const uint8_t* buf, int* ax, int* ay, int* az) {
  // flipFirstAndThirdByte(zero(4) ~ uint(5).as("az") ~ uint(10).as("ay") ~ uint(5).as("ax"))
  *az = ((buf[2] * 256 + buf[1]) & 0b0000111110000000) >> 7;
  *ay = ((buf[2] * 256 * 256 + buf[1] * 256 + buf[0]) & 0b000000000111111111100000) >> 5;
  *ax = (buf[0]) & 0b11111;
  //Serial.printf("parse pattern %02x %02x %02x ax=%d ay=%d az=%d\n", buf[2],buf[1],buf[0],*ax,*ay,*az);
}

void coyote_timer_callback(TimerHandle_t xTimerID) {
  try {
    auto instance = coyote_timer_map.at(xTimerID);
    instance->timer_callback(xTimerID);
  } catch ( const std::out_of_range& e ) {
    Serial.println("Could not find timer callback instance");
  }
}

// This is called every 100mS to provide the coyote box with what to do next
//
void Coyote::timer_callback(TimerHandle_t xTimerID) {
  uint8_t buf[4];

  if (!coyote_connected) return;

  // Want to switch modes, only if no current mode or end of a cycle - no glitching!
  if ((wavemodea == 0 || waveclocka == 0) && (wantedmodea != wavemodea)) {
    wavemodea = wantedmodea;
    waveclocka = 0;
    notify(C_WAVEMODE_A);
  }
  if ((wavemodeb == 0 || waveclockb == 0) && (wantedmodeb != wavemodeb)) {
    wavemodeb = wantedmodeb;
    waveclockb = 0;
    notify(C_WAVEMODE_B);
  }
  // this sets the power to where we want it (also stop you changing it on the rocker switches)
  if (coyote_powerA != coyote_powerAwanted || coyote_powerB != coyote_powerBwanted) {
    encode_power(buf, coyote_powerAwanted, coyote_powerBwanted);
    if (!powerCharacteristic->writeValue(buf, 3))
      Serial.println("Failed to write power");
  }
  // Do WaveA
  if (coyote_powerA > 0 && wavemodea != 0) {
    coyote_wave_ramp(&waveclocka, &wavemodea, &coyote_ax, &coyote_ay, &coyote_az);
    encode_pattern(buf, coyote_ax, coyote_ay, coyote_az);
    if (!patternACharacteristic->writeValue(buf, 3))
      Serial.println("Failed to write pattern A");
  }
  // Do WaveB
  if (coyote_powerB > 0 && wavemodeb != 0) {
    coyote_wave_ramp(&waveclockb, &wavemodeb, &coyote_bx, &coyote_by, &coyote_bz);
    encode_pattern(buf, coyote_bx, coyote_by, coyote_bz);
    if (!patternBCharacteristic->writeValue(buf, 3)) {
      Serial.println("Failed to write pattern B");
    }
  }
}

void Coyote::connected_callback() {
    Serial.println("Client onConnect");
}

void Coyote::disconnected_callback() {
    coyote_connected = false;
    xTimerStop(coyoteTimer, 0);
    Serial.println("Client onDisconnect");
    notify(C_DISCONNECTED);
}

class CoyoteNimBLEClientCallback : public NimBLEClientCallbacks {
public:
  CoyoteNimBLEClientCallback(Coyote* instance) {
    coyote_instance = instance;
  }

  void onConnect(NimBLEClient* pclient) {
    coyote_instance->connected_callback();
  }

  void onDisconnect(NimBLEClient* pclient) {
    coyote_instance->disconnected_callback();
  }

private:
  Coyote* coyote_instance;
};

bool Coyote::getService(NimBLERemoteService*& service, NimBLEUUID uuid) {
  Serial.printf("Getting service\n");
  service = bleClient->getService(uuid);
  if (service == nullptr) {
    Serial.print("Failed to find service UUID: ");
    Serial.println(uuid.toString().c_str());
    return false;
  }
  return true;
}

bool getCharacteristic(NimBLERemoteService* service, NimBLERemoteCharacteristic*& c, NimBLEUUID uuid, notify_callback notifyCallback = nullptr) {
  Serial.printf("Getting characteristic\n");
  c = service->getCharacteristic(uuid);
  if (c == nullptr) {
    Serial.print("Failed to find characteristic UUID: ");
    Serial.println(uuid.toString().c_str());
    return false;
  }

  if (!notifyCallback)
    return true;

  // we want notifications
  if (c->canNotify() && c->subscribe(true, notifyCallback))
    return true;
  else {
    Serial.print("Failed to register for notifications for characteristic UUID: ");
    Serial.println(uuid.toString().c_str());
    return false;
  }
}

void Coyote::batterylevel_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify) {
  coyote_batterylevel = data[0];
  Serial.printf("coyote batterycb: %d\n", coyote_batterylevel);
}

void Coyote::power_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify) {
  if (length >= 3) {
    parse_power(data);
    notify(C_POWER);
  }
  else
    Serial.println("Power callback with incorrect length");
}

Coyote::Coyote() {
}

Coyote::~Coyote() {
  if ( coyoteTimer )
    coyote_timer_map.erase(coyoteTimer);
}

bool Coyote::connect_to_device(NimBLEAdvertisedDevice* coyote_device) {
  if (coyote_connected)
    return false;

  if (!bleClient) {
    bleClient = NimBLEDevice::createClient();
    bleClient->setClientCallbacks(new CoyoteNimBLEClientCallback(this));
  }

  Serial.printf("Will try to connect to Coyote at %s\n", coyote_device->getAddress().toString().c_str());
  if (!bleClient->connect(coyote_device)) {
    Serial.println("Connection failed");
    return false;
  }
  Serial.println("Connection established");

  bool res = true;
  res &= getService(coyoteService, COYOTE_SERVICE_BLEUUID);
  res &= getService(batteryService, BATTERY_SERVICE_BLEUUID);

  if (res == false) {
    Serial.println("Missing service");
    bleClient->disconnect();
    return false;
  }
  
  res &= getCharacteristic(batteryService, batteryLevelCharacteristic, BATTERY_CHAR_BLEUUID, std::bind(&Coyote::batterylevel_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  res &= getCharacteristic(coyoteService, configCharacteristic, CONFIG_CHAR_BLEUUID);
  res &= getCharacteristic(coyoteService, powerCharacteristic, POWER_CHAR_BLEUUID, std::bind(&Coyote::power_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  res &= getCharacteristic(coyoteService, patternACharacteristic, A_CHAR_BLEUUID);
  res &= getCharacteristic(coyoteService, patternBCharacteristic, B_CHAR_BLEUUID);

  if (res == false) {
    Serial.println("Missing characteristic");
    bleClient->disconnect();
    return false;
  }

  Serial.println("Found services and characteristics");

  coyote_connected = true;

  coyote_batterylevel = batteryLevelCharacteristic->readValue<uint8_t>();
  Serial.printf("Battery level: %u\n", coyote_batterylevel);

  auto configData = configCharacteristic->readValue();
  coyote_maxPower = (configData[2] & 0xf) * 256 + configData[1];
  coyote_powerStep = configData[0];
  Serial.printf("coyote maxPower: %d\n", coyote_maxPower);
  Serial.printf("coyote powerStep: %d\n", coyote_powerStep);

  auto powerData = powerCharacteristic->readValue();
  if (powerData.size() >= 3)
    parse_power(powerData.begin());

  auto patternAData = patternACharacteristic->readValue();
  auto patternBData = patternBCharacteristic->readValue();
  if (patternAData.size() < 3 || patternBData.size() < 3) {
    Serial.println("No pattern data?");
  }
  parse_pattern(patternAData.begin(), &coyote_ax, &coyote_ay, &coyote_az);
  parse_pattern(patternBData.begin(), &coyote_bx, &coyote_by, &coyote_bz);

  uint8_t buf[5];
  encode_power(buf, start_powerA, start_powerB);
  if (!powerCharacteristic->writeValue(buf, 3))
    Serial.println("Failed to write powerCharacteristic");

  wavemodea = 1;
  wavemodeb = 1;

  if (!coyoteTimer) {
    coyoteTimer = xTimerCreate(nullptr, pdMS_TO_TICKS(100), true, nullptr, coyote_timer_callback);
    coyote_timer_map[coyoteTimer] = this;
  }
  xTimerStart(coyoteTimer, 0);

  Serial.println("coyote connected");
  notify(C_CONNECTED); // perhaps this should be in the connected callback?
  return true;
}
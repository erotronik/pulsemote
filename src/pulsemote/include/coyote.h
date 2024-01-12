#pragma once

#define start_powerA 0 // max is usually 2000
#define start_powerB 0 // max is usually 2000


#include <NimBLEDevice.h>

class CoyoteNimBLEClientCallback;

enum coyote_mode { M_NONE, M_BREATH };
enum coyote_type_of_change { C_POWER, C_WAVEMODE_A, C_WAVEMODE_B, C_DISCONNECTED, C_CONNECTED };
typedef std::function<void (coyote_type_of_change change)> coyote_callback;

class Coyote {
public:
    Coyote();
    ~Coyote();

    bool get_isconnected();
    bool connect_to_device(NimBLEAdvertisedDevice* coyote_device);

    void put_powerup(short a, short b);
    int get_powera_pc();
    int get_powerb_pc();
    uint8_t get_batterylevel();
    coyote_mode get_modea();
    coyote_mode get_modeb();
    void put_setmode(coyote_mode a, coyote_mode b);
    void setup();
    void timer_callback(TimerHandle_t xTimerID);

    void set_callback(coyote_callback);
    // returns true if an advertised device is a coyote
    static bool is_coyote(NimBLEAdvertisedDevice* advertisedDevice);

private:
    friend class CoyoteNimBLEClientCallback;

    void batterylevel_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify);
    void power_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify);

    void parse_power(const uint8_t* buf);
    void encode_power(uint8_t* buf, int xpowerA, int xpowerB);
    void encode_pattern(uint8_t* buf, int ax, int ay, int az);
    void parse_pattern(const uint8_t* buf, int* ax, int* ay, int* az);

    bool getService(NimBLERemoteService*& service, NimBLEUUID uuid);

    // called when connection/disconnection is signalled
    void connected_callback();
    void disconnected_callback();
    void notify(coyote_type_of_change);

    int coyote_ax = 0; int coyote_ay = 0; int coyote_az = 0;
    int coyote_bx = 0; int coyote_by = 0; int coyote_bz = 0;

    short waveclocka = 0;
    short waveclockb = 0;
    unsigned short coyote_powerA;
    unsigned short coyote_powerB;
    unsigned short coyote_powerAwanted = start_powerA;
    unsigned short coyote_powerBwanted = start_powerB;
    coyote_mode wavemodea = M_NONE;
    coyote_mode wavemodeb = M_NONE;
    coyote_mode wantedmodea = M_NONE;
    coyote_mode wantedmodeb = M_NONE;
    uint8_t coyote_powerStep = 0;
    uint8_t coyote_batterylevel = 0;
    uint16_t coyote_maxPower = 1;
    bool coyote_connected = false;
    NimBLEClient* bleClient = nullptr;

    TimerHandle_t coyoteTimer = nullptr;

    NimBLERemoteService* coyoteService;
    // deletion handled by coyoteService
    NimBLERemoteCharacteristic* configCharacteristic;
    NimBLERemoteCharacteristic* powerCharacteristic;
    NimBLERemoteCharacteristic* patternACharacteristic;
    NimBLERemoteCharacteristic* patternBCharacteristic;
    NimBLERemoteService* batteryService;
    // deletion handled by batteryService
    NimBLERemoteCharacteristic* batteryLevelCharacteristic;

    coyote_callback update_callback;
};


#pragma once

#define start_powerA 0 // max is usually 2000
#define start_powerB 0 // max is usually 2000

#include <NimBLEDevice.h>

class CoyoteNimBLEClientCallback;

class Coyote {
public:
    Coyote();
    ~Coyote();

    boolean get_isconnected();
    bool connect_to_device(NimBLEAdvertisedDevice* coyote_device);


    void put_powerup(short a, short b);
    int get_powera_pc();
    int get_powerb_pc();
    uint8_t get_batterylevel();
    short get_modea();
    short get_modeb();
    void put_setmode(short a, short b);
    void put_toggle();
    void setup();
    void timer_callback(TimerHandle_t xTimerID);

private:
    friend class CoyoteNimBLEClientCallback;

    void batterylevel_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify);
    void power_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify);

    void parse_power(const uint8_t* buf);
    void encode_power(uint8_t* buf, int xpowerA, int xpowerB);
    void encode_pattern(uint8_t* buf, int ax, int ay, int az);
    void parse_pattern(const uint8_t* buf, int* ax, int* ay, int* az);

    // called when connection/disconnection is signalled
    void connected_callback();
    void disconnected_callback();

    int coyote_ax = 0; int coyote_ay = 0; int coyote_az = 0;
    int coyote_bx = 0; int coyote_by = 0; int coyote_bz = 0;

    short waveclocka = 0;
    short waveclockb = 0;
    unsigned short coyote_powerA;
    unsigned short coyote_powerB;
    unsigned short coyote_powerAwanted = start_powerA;
    unsigned short coyote_powerBwanted = start_powerB;
    short wavemodea = 0;
    short wavemodeb = 0;
    short wantedmodea = 0;
    short wantedmodeb = 0;
    uint8_t coyote_powerStep = 0;
    uint8_t coyote_batterylevel = 0;
    uint16_t coyote_maxPower = 1;
    boolean coyote_connected = false;

    TimerHandle_t coyoteTimer = nullptr;
};


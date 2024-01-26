#pragma once

#define start_power 0 // max is usually 2000

#include <memory>
#include <NimBLEDevice.h>

class CoyoteNimBLEClientCallback;
class Coyote;

// The maximum outut is quite high. Limit to x% instead and adjust all display and set
// options respectively
constexpr int coyote_max_power_percent = 50;

struct coyote_pattern {
    int pulse_length = 0; // 0-31 ms
    int pause_length = 0; // 0-1023 ms
    int amplitude = 0; // 0-31
};

enum coyote_mode { M_NONE, M_BREATH, M_WAVES, M_CUSTOM };
enum coyote_type_of_change { C_NONE, C_POWER, C_WAVEMODE_A, C_WAVEMODE_B, C_DISCONNECTED, C_CONNECTING, C_CONNECTED };

typedef std::function<void (coyote_type_of_change change)> coyote_callback;
typedef std::function<coyote_pattern (uint32_t &waveclock, uint32_t &cyclecount)> coyote_mode_function;

coyote_pattern coyote_mode_nothing(uint32_t &waveclock, uint32_t &cyclecount);

class CoyoteChannel {
public:
    void put_power_diff(short);
    // change output power directly. Be careful with this.
    void put_power_pc(short);
    int get_power_pc() const;
    coyote_mode get_mode() const { return wavemode; }
    void put_setmode(coyote_mode);
    void put_setmode(coyote_mode_function);

private:
    friend class Coyote;

    CoyoteChannel(Coyote* coyote, std::string name);
    void update_pattern();

    Coyote* parent;

    uint32_t waveclock = 0;
    uint32_t cyclecount = 0;
    bool wavemode_changed = false;
    coyote_mode wavemode = M_NONE;
    coyote_mode_function mode_function = coyote_mode_nothing;
    coyote_mode_function wanted_mode_function = coyote_mode_nothing;
    unsigned short coyote_power;
    unsigned short coyote_power_wanted = start_power;
    coyote_pattern pattern;
    std::string channel_name;
};

class Coyote {
public:
    Coyote();
    ~Coyote();

    CoyoteChannel* chan_a() { return channel_a.get(); }
    CoyoteChannel* chan_b() { return channel_b.get(); }

    bool get_isconnected();
    bool connect_to_device(NimBLEAdvertisedDevice* coyote_device);

    uint8_t get_batterylevel();
    void setup();
    void timer_callback(TimerHandle_t xTimerID);

    void set_callback(coyote_callback);
    // returns true if an advertised device is a coyote
    static bool is_coyote(NimBLEAdvertisedDevice* advertisedDevice);
private:
    friend class CoyoteNimBLEClientCallback;
    friend class CoyoteChannel;

    std::unique_ptr<CoyoteChannel> channel_a;
    std::unique_ptr<CoyoteChannel> channel_b;

    void batterylevel_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify);
    void power_callback(NimBLERemoteCharacteristic* chr, uint8_t* data, size_t length, bool isNotify);

    void parse_power(const std::vector<uint8_t>);
    static std::vector<uint8_t> encode_power(int xpowerA, int xpowerB);
    static std::vector<uint8_t> encode_pattern(coyote_pattern);
    static coyote_pattern parse_pattern(const std::vector<uint8_t>);

    bool getService(NimBLERemoteService*& service, NimBLEUUID uuid);

    // called when connection/disconnection is signalled
    void connected_callback();
    void disconnected_callback(int reason);
    void notify(coyote_type_of_change);

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
    bool connection_fully_established = false;
};


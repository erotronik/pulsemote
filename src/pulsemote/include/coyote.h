#define start_powerA 0 // max is usually 2000
#define start_powerB 0 // max is usually 2000

boolean coyote_get_isconnected();
void coyote_put_powerup(short a, short b);
int coyote_get_powera_pc();
int coyote_get_powerb_pc();
uint8_t coyote_get_batterylevel();
short coyote_get_modea();
short coyote_get_modeb();
void coyote_put_setmode(short a, short b);
void coyote_put_toggle();
void coyote_setup();
bool connect_to_coyote(void* coyote_device);
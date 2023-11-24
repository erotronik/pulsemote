// Save power by setting the CPU Frequency lower, 160MHz for example

#ifdef ARDUINO_M5STACK_CORE2
#define CORE2
#endif

#ifdef CORE2
#include <M5Core2.h>
#define LGFX_M5STACK_CORE2
#else
#include <M5Stack.h>
#define LGFX_M5STACK
#endif

#ifdef ESP32
#include "NimBLEDevice.h"
#endif /* ESP32 */
#ifndef ESP32
#include <bluefruit.h>
#endif

#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

static LGFX lcd;
//static LGFX_Sprite sprite(&lcd);
short debug_mode = 0;

// this belongs into comms.ino. But it has to be in a separate file because - Arduino.
enum scan_callback_result { None, Coyote };

boolean need_display_update = false;
boolean need_display_timer_update = false;

void update_display() {
  need_display_update = true;
  need_display_timer_update = true;
}

unsigned int blockunsel = 0x2222ffU;
unsigned int blocksel = 0xff2222U;

char *modes[] = { "Off", "Breath", "Random", "" };
short mode_now = 0;
typedef enum { STATE_MODE, STATE_B, STATE_A, STATE_LAST } states;
short select_state = STATE_MODE;

void update_display_timer() {
  unsigned int bc;
  bc = blockunsel;
  if (select_state == STATE_MODE)
    bc = blocksel;
  lcd.setTextColor(0xFFFFFFU, bc);

  if (mode_now == 2) {
    lcd.setCursor(100 + 30, 95);
    lcd.printf("%3d", random_time_left());
    lcd.setCursor(100 + 30, 65);
    lcd.printf("%3s", modes[mode_now]);  // Random
  }
}

void update_display_if_needed() {
  unsigned int bc;

  if (need_display_timer_update && !need_display_update) {
    update_display_timer();
    need_display_timer_update = false;
    return;
  }

  if (!need_display_update) return;
  need_display_update = false;
  need_display_timer_update = false;

  if (!coyote_get_isconnected()) {
    lcd.clearDisplay();
    lcd.setCursor(0, 0);
    lcd.setFont(&fonts::Font2);
    lcd.setTextColor(0xFFFFFFU);
    lcd.printf("github.com/erotronik/\npulsemote\n\n");
    lcd.setTextColor(0xFFccbbU);
    lcd.printf("Bluetooth remote\nfor DG-LAB 2.0\n\n");
    lcd.setTextColor(0xFFFFFFU);
    lcd.printf("Scanning...");
    lcd.setFont(NULL);
    return;
  }

  bc = blockunsel;
  if (select_state == STATE_A)
    bc = blocksel;

  //lcd.setTextDatum( middle_center);

  lcd.fillRect(0, 0, 90, 140, bc);
  lcd.setCursor(29, 10);
  lcd.setTextColor(0xFFFFFFU, bc);
  lcd.printf(" A ");

  lcd.setCursor(28 - 12, 75 - 30);
  if (coyote_get_isconnected()) {
    lcd.setFont(&fonts::Font4);
    lcd.printf("%02d", coyote_get_powera_pc());
    lcd.setFont(NULL);
    lcd.setCursor(28 - 14, 105);
    lcd.printf("%s", modes[coyote_get_modea()]);
  }
  //else
  // lcd.printf("   ");

  bc = blockunsel;
  if (select_state == STATE_B)
    bc = blocksel;

  lcd.fillRect(320 - 90, 0, 90, 140, bc);
  lcd.setTextColor(0xFFFFFFU, bc);
  lcd.setCursor(320 - 90 + 29, 10);
  lcd.printf(" B ");

  lcd.setCursor(320 - 90 + 28 - 12, 75 - 30);
  if (coyote_get_isconnected()) {
    lcd.setFont(&fonts::Font4);
    lcd.printf("%02d", coyote_get_powerb_pc());
    lcd.setFont(NULL);
    lcd.setCursor(320 - 90 + 28 - 14, 105);
    lcd.printf("%s", modes[coyote_get_modeb()]);
  }
  //else
  //  lcd.printf("   ") ;

  bc = blockunsel;
  if (select_state == STATE_MODE)
    bc = blocksel;

  lcd.fillRect(100, 0, 120, 140, bc);
  lcd.setTextColor(0xFFFFFFU, bc);

  lcd.setCursor(100 + 20, 10);
  lcd.printf(" Mode ");
  update_display_timer();

  if (coyote_get_isconnected()) {
    if (mode_now == 0) {
      lcd.setCursor(100 + 30, 70);
      lcd.printf("%s", modes[coyote_get_modea()]);
    }
    if (mode_now == 1) {
      lcd.setCursor(100 + 30, 70);
      lcd.printf("On");
    }
  }

  lcd.setTextColor(0xFFFFFFU, 0);

  if (coyote_get_isconnected()) {
    lcd.setCursor(0, 170);
    lcd.printf("  Status: Connected %d%%  ", coyote_get_batterylevel());
  } else {
    lcd.setCursor(0, 170);
    lcd.printf("  Status: Scanning           ");
  }

  if (coyote_get_isconnected()) {
    lcd.setTextColor(0xFFFFFFU, blocksel);
    lcd.setCursor(30, 225);
    lcd.printf("Choose");
    lcd.setCursor(131, 225);
    if (select_state != STATE_MODE)
      lcd.printf("  -  ");
    else
      lcd.printf("     ");
    lcd.setCursor(225, 225);
    lcd.printf("  +  ");
    lcd.setTextColor(0xFFFFFFU, 0);
  }
}

byte button, oldbutton = 0;
unsigned long repeatbutton = 0;
unsigned short rcount = 0;

// A = 1, B = 2, C = 3
#ifdef CORE2
bool get_individual_button_state(byte button) {
  switch ( button ) {
    case 1:
      return M5.BtnA.isPressed();
    case 2:
      return M5.BtnB.isPressed();
    case 3:
      return M5.BtnC.isPressed();
    default:
      return false;
  }
}
#else
bool get_individual_button_state(byte button) {
  auto pin = BUTTON_A_PIN;
  if ( button == 2 )
    pin = BUTTON_B_PIN;
  else if ( button == 3 )
    pin = BUTTON_C_PIN;

  return digitalRead(pin) == LOW;
}
#endif

byte get_button() {
  button = 0;
  if (get_individual_button_state(1))
    button = 1;
  else if (get_individual_button_state(2))
    button = 2;
  else if (get_individual_button_state(3))
    button = 3;

  if (button != oldbutton) {
    oldbutton = button;
    if (button != 0) {
      repeatbutton = millis();
      rcount = 0;
      return button;
    }
  }
  // If you hold a button more than XmS then it'll repeat every YmS, but only for 10 times, so you can't end up with your finger stuck on +

  unsigned short startrepeatpress;
  unsigned short maxrepeats;
  unsigned short continuerepeatpress;

  if (button ==3) {  // +
    startrepeatpress = 400;
    maxrepeats = 10;
    continuerepeatpress = 200;    
  } else if (button ==2) {  // -
    startrepeatpress = 400;
    maxrepeats = 1000;
    continuerepeatpress = 100;    
    if (rcount > 10) { // accelerate down
      continuerepeatpress = 20;
    }
  } else if (button ==1) {  // choose
    startrepeatpress = 400;
    maxrepeats = 10;
    continuerepeatpress = 400;    
  }
 
  if (rcount == 0 && millis() - repeatbutton > startrepeatpress) {
      rcount++;
      repeatbutton = millis();
      return button;
  }
  if (rcount !=0 && millis() - repeatbutton > continuerepeatpress) {
    if (rcount < maxrepeats-1) {
      rcount++;
      repeatbutton = millis();
      return button;
    }
  }
  return 0;
}

unsigned long random_timer = 0;
unsigned long random_display_refresh = 0;

short random_time_left() {
  short a = ((random_timer - millis()) / 1000);
  if (a > 500 || a < 0) return 0;
  return a;
}

void handle_random_mode() {
  if (mode_now != 2) return;
  if (millis() > random_timer) {
    Serial.println("Random time to switch");
    if (coyote_get_modea() == 0) {
      random_timer = millis() + random(10000, 30000);  // On time is 10-30 seconds
    } else {
      random_timer = millis() + random(30000, 50000);  // Off time is 30-50 seconds
    }
    coyote_put_toggle();
  }
  if (millis() > random_display_refresh + 500) {  // update the display every .5 seconds with countdown
    random_display_refresh = millis();
    need_display_timer_update = true;
  }
}

void handle_buttons() {
  byte b = get_button();
  if (b == 1) {
    select_state++;
    if (select_state >= STATE_LAST) {
      select_state = STATE_MODE;
    }
    update_display();
    return;
  }
  if (select_state == STATE_MODE) {
    if (b == 3) {
      mode_now++;
      if (modes[mode_now] == "") {
        mode_now = 0;
      }
      Serial.printf("Set mode %d\n", mode_now);
      if (mode_now == 1) {
        coyote_put_setmode(1, 1);
      } else {
        coyote_put_setmode(0, 0);
      }
    }
  } else if (select_state == STATE_A) {
    if (b == 2) {
      coyote_put_powerup(-1, 0);
    } else if (b == 3) {
      coyote_put_powerup(1, 0);
    }
  } else if (select_state == STATE_B) {
    if (b == 2) {
      coyote_put_powerup(0, -1);
    } else if (b == 3) {
      coyote_put_powerup(0, 1);
    }
  }
}


void setup() {
  M5.begin(true, false);
  Serial.begin(115200);

  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
  lcd.setColorDepth(16);
  lcd.clearDisplay();
  lcd.setTextSize(2);

  delay(100);
  comms_init(0);

  // M5.Speaker.begin(); Speaker unused, does not work like this on Core2

  xTaskCreate(TaskMain, "Main", 10000, nullptr, 1, nullptr);
#if defined(ESP32)
  xTaskCreate(TaskScan, "Scan", 10000, nullptr, 2, nullptr);
#endif
  need_display_update = true;
}

void main_loop() {
  M5.update();

  handle_buttons();
  handle_random_mode();

  comms_uart_colorpicker();
  update_display_if_needed();
  delay(10);
}

// Leave empty - we are using FreeRTOS tasks instead of the Arduino main loop
void loop() {
}

void TaskMain(void *pvParameters) {
  vTaskDelay(200);
  while (true)
    main_loop();
}

// On the ESP32, we scan in a separate task - scanning is a blocking
// operation. All the communication with the Coyote also happens
// in this task.
#if defined(ESP32)
void TaskScan(void *pvParameters) {
  while (true) {
    scan_loop();
  }
}
#endif

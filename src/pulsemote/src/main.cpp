// Save power by setting the CPU Frequency lower, 160MHz for example
#include <M5Unified.h>
#include <memory>

#include "coyote.h"
#include "comms.h"

#include <freertos/task.h>

extern "C" {
void app_main();
}

static M5GFX lcd;
std::unique_ptr<Coyote> coyote_controller;

bool need_display_clear = false;
bool need_display_update = false;
bool need_display_timer_update = false;
coyote_type_of_change last_change = C_NONE;

uint32_t millis() {
  return esp_timer_get_time()/1000;
}

int random_range(int min, int max){
    int n = max - min + 1;
    int remainder = RAND_MAX % n;
    int x;
    do{
        x = rand();
    }while (x >= RAND_MAX - remainder);
    return min + x % n;
}

void update_display(bool clear_display) {
  if ( clear_display )
    need_display_clear = true;
  need_display_update = true;
  need_display_timer_update = true;
}

unsigned int blockunsel = 0x2222ffU;
unsigned int blocksel = 0xff2222U;

std::array<std::string, 4> modes = { "Off", "Breath", "Waves", "Random"};
short mode_now = 0;
typedef enum { STATE_MODE, STATE_B, STATE_A, STATE_LAST } states;
short select_state = STATE_MODE;

unsigned long random_timer = 0;
unsigned long random_display_refresh = 0;

short random_time_left() {
  short a = ((random_timer - millis()) / 1000);
  if (a > 500 || a < 0) return 0;
  return a;
}

void update_display_timer() {
  unsigned int bc;
  bc = blockunsel;
  if (select_state == STATE_MODE)
    bc = blocksel;
  lcd.setTextColor(0xFFFFFFU, bc);

  if (mode_now == 3) {
    lcd.setCursor(100 + 30, 95);
    lcd.printf("%3d", random_time_left());
    lcd.setCursor(100 + 30, 65);
    lcd.printf("%3s", modes[mode_now].c_str());  // Random
  }
}

void update_display_if_needed() {
  unsigned int bc;

  if (need_display_clear) {
    need_display_clear = false;
    lcd.clearDisplay();
  }

  if (need_display_timer_update && !need_display_update) {
    update_display_timer();
    need_display_timer_update = false;
    return;
  }

  if (!need_display_update) return;
  need_display_update = false;
  need_display_timer_update = false;

  if (!coyote_controller->get_isconnected()) {
    lcd.clearDisplay();
    lcd.setCursor(0, 0);
    lcd.setFont(&fonts::Font2);
    lcd.setTextColor(0xFFFFFFU);
    lcd.printf("github.com/erotronik/\npulsemote\n\n");
    lcd.setTextColor(0xFFccbbU);
    lcd.printf("Bluetooth remote\nfor DG-LAB 2.0\n\n");
    lcd.setTextColor(0xFFFFFFU);
    if ( last_change == C_CONNECTING )
      lcd.printf("Connecting...");
    else
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
  if (coyote_controller->get_isconnected()) {
    lcd.setFont(&fonts::Font4);
    lcd.printf("%02d", coyote_controller->chan_a()->get_power_pc());
    lcd.setFont(NULL);
    lcd.setCursor(28 - 14, 105);
    lcd.printf("%s", modes[coyote_controller->chan_a()->get_mode()].c_str());
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
  if (coyote_controller->get_isconnected()) {
    lcd.setFont(&fonts::Font4);
    lcd.printf("%02d", coyote_controller->chan_b()->get_power_pc());
    lcd.setFont(NULL);
    lcd.setCursor(320 - 90 + 28 - 14, 105);
    lcd.printf("%s", modes[coyote_controller->chan_b()->get_mode()].c_str());
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

  if (coyote_controller->get_isconnected()) {
    if (mode_now == 0) {
      lcd.setCursor(100 + 30, 70);
      lcd.printf("%s", modes[coyote_controller->chan_a()->get_mode()].c_str());
    }
    if (mode_now == 1 || mode_now == 2) {
      lcd.setCursor(100 + 30, 70);
      lcd.printf("On");
    }
  }

  lcd.setTextColor(0xFFFFFFU, 0);

  if (coyote_controller->get_isconnected()) {
    lcd.setCursor(0, 170);
    lcd.printf("  Status: Connected %d%%  ", coyote_controller->get_batterylevel());
  } else {
    lcd.setCursor(0, 170);
    lcd.printf("  Status: Scanning           ");
  }

  if (coyote_controller->get_isconnected()) {
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

uint8_t button, oldbutton = 0;
unsigned long repeatbutton = 0;
unsigned short rcount = 0;

// A = 1, B = 2, C = 3
bool get_individual_button_state(uint8_t button) {
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

uint8_t get_button() {
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
  unsigned short startrepeatpress = 0;
  unsigned short maxrepeats = 0;
  unsigned short continuerepeatpress = 0;

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

// needs an update if we have more than one mode.
void handle_random_mode() {
  if (mode_now != 3) return;
  if (millis() > random_timer) {
    ESP_LOGD("main", "Random time to switch");
    if (coyote_controller->chan_a()->get_mode() == M_NONE) {
      random_timer = millis() + random_range(10000, 30000);  // On time is 10-30 seconds
      coyote_controller->chan_a()->put_setmode(M_BREATH);
      coyote_controller->chan_b()->put_setmode(M_BREATH);
    } else {
      random_timer = millis() + random_range(30000, 50000);  // Off time is 30-50 seconds
      coyote_controller->chan_a()->put_setmode(M_NONE);
      coyote_controller->chan_b()->put_setmode(M_NONE);
    }
  }
  if (millis() > random_display_refresh + 500) {  // update the display every .5 seconds with countdown
    random_display_refresh = millis();
    need_display_timer_update = true;
  }
}

void coyote_change_handler(coyote_type_of_change t) {
  last_change = t;
  if ( t == C_CONNECTED ) {
    update_display(true);
    comms_stop_scan();
    return;
  }
  update_display(false);
}

void handle_buttons() {
  auto b = get_button();
  if (b == 1) {
    select_state++;
    if (select_state >= STATE_LAST) {
      select_state = STATE_MODE;
    }
    update_display(false);
    return;
  }
  if (select_state == STATE_MODE) {
    if (b == 3) {
      mode_now++;
      if (mode_now >= modes.size()) {
        mode_now = 0;
      }
      ESP_LOGD("main", "Set mode %d\n", mode_now);
      if (mode_now == 1) {
        coyote_controller->chan_a()->put_setmode(M_BREATH);
        coyote_controller->chan_b()->put_setmode(M_BREATH);
        update_display(false);
      } else if ( mode_now == 2 ) {
        coyote_controller->chan_a()->put_setmode(M_WAVES);
        coyote_controller->chan_b()->put_setmode(M_WAVES);
        update_display(false);
      } else {
        coyote_controller->chan_a()->put_setmode(M_NONE);
        coyote_controller->chan_b()->put_setmode(M_NONE);
        update_display(false);
      }
    }
  } else if (select_state == STATE_A) {
    if (b == 2) {
      coyote_controller->chan_a()->put_power_diff(-1);
    } else if (b == 3) {
      coyote_controller->chan_a()->put_power_diff(1);
    }
  } else if (select_state == STATE_B) {
    if (b == 2) {
      coyote_controller->chan_b()->put_power_diff(-1);
    } else if (b == 3) {
      coyote_controller->chan_b()->put_power_diff(1);
    }
  }
}

// Leave empty - we are using FreeRTOS tasks instead of the Arduino main loop
void loop() {
}

void main_loop() {
  M5.update();

  handle_buttons();
  handle_random_mode();

  //comms_uart_colorpicker();
  update_display_if_needed();
  vTaskDelay(pdMS_TO_TICKS(10));
}

void TaskMain(void *pvParameters) {
  vTaskDelay(200);
  while (true)
    main_loop();
}

// On the ESP32, we scan in a separate task - scanning is a blocking
// operation. All the communication with the Coyote also happens
// in this task.
void TaskScan(void *pvParameters) {
  while (true) {
    scan_loop();
  }
}

void app_main() {
  auto cfg = M5.config();
  M5.begin(cfg);

  lcd.init();
  lcd.setRotation(1);
  lcd.setBrightness(128);
  lcd.setColorDepth(16);
  lcd.clearDisplay();
  lcd.setTextSize(2);

  vTaskDelay(pdMS_TO_TICKS(100));
  coyote_controller = std::unique_ptr<Coyote>(new Coyote());
  coyote_controller->set_callback(coyote_change_handler);

  xTaskCreate(TaskMain, "Main", 10000, nullptr, 1, nullptr);
  xTaskCreatePinnedToCore(TaskScan, "Scan", 10000, nullptr, 2, nullptr, 0);
  need_display_update = true;
}
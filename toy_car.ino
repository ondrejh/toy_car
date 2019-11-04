#include <PCM.h>

#include "tick.h"
#include "tock.h"
#include "horn.h"

#define BLINK_PERIOD 350

#define RIGHT 8
#define LIGHTS 9
#define LEFT 10

#define ON LOW
#define OFF HIGH

#define BTN_LIGHT 17
#define BTN_RIGHT 16
#define BTN_LEFT  15
#define BTN_ALARM 14
#define BTN_HORN  12

#define BTN_SAMPLING_MS 2
#define BTN_STABLE_CNT 8

#define PLAYING (TCCR2B&_BV(CS10))

typedef struct {
  bool btn;
  uint8_t cnt;
  uint8_t input;
} button_t;

#define BTNF_LIGHT 0
#define BTNF_RIGHT 1
#define BTNF_LEFT  2
#define BTNF_ALARM 3
#define BTNF_HORN  4
button_t buttons[5];

void btn_init(button_t *button, uint8_t btn_input) {
  button->input = btn_input;
  button->btn = false;
  button->cnt = 0;
}

uint8_t btn_filter(button_t *button) {
  if (button->btn == digitalRead(button->input)) {
    button->cnt ++;
    if (button->cnt >= BTN_STABLE_CNT) {
      button->btn = !button->btn;
      if (button->btn)
        return 1;
    }
  }
  else
    button->cnt = 0;
  return 0;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(LIGHTS, OUTPUT);
  digitalWrite(LIGHTS, OFF);
  pinMode(LEFT, OUTPUT);
  digitalWrite(LEFT, OFF);
  pinMode(RIGHT, OUTPUT);
  digitalWrite(RIGHT, OFF);

  pinMode(BTN_LIGHT, INPUT_PULLUP);
  btn_init(&buttons[BTNF_LIGHT], BTN_LIGHT);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  btn_init(&buttons[BTNF_RIGHT], BTN_RIGHT);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  btn_init(&buttons[BTNF_LEFT],  BTN_LEFT);
  pinMode(BTN_ALARM, INPUT_PULLUP);
  btn_init(&buttons[BTNF_ALARM], BTN_ALARM);
  pinMode(BTN_HORN, INPUT_PULLUP);
  btn_init(&buttons[BTNF_HORN], BTN_HORN);
}

void loop() {
  uint32_t now = millis();

  uint8_t blinker_status = 0;
  uint8_t next_blinker_status = 0;

  // read and filter buttons and connect actions
  static uint32_t btn_time;
  if ((now - btn_time) >= BTN_SAMPLING_MS) {
    btn_time += BTN_SAMPLING_MS;

    if (btn_filter(&buttons[BTNF_LIGHT])) { // LIGHTS ON - OFF
      static bool lights_on = false;
      lights_on = !lights_on;
      digitalWrite(LIGHTS, lights_on);
    }
    if (btn_filter(&buttons[BTNF_RIGHT])) { // BLINKER RIGHT
      if (blinker_status == 0x1)
        next_blinker_status = 0x0;
      else if ((blinker_status == 0x0) || (blinker_status == 0x2))
        next_blinker_status = 0x1;
    }
    if (btn_filter(&buttons[BTNF_LEFT])) { // BLINKER LEFT
      if (blinker_status == 0x2)
        next_blinker_status = 0x0;
      else if ((blinker_status == 0x0) || (blinker_status == 0x1))
        next_blinker_status = 0x2;
    }
    if (btn_filter(&buttons[BTNF_ALARM])) { // BLINKER ALARM
      if (blinker_status == 0x3)
        next_blinker_status = 0x0;
      else
        next_blinker_status = 0x3;
    }
    if (btn_filter(&buttons[BTNF_HORN])) // HORN
      startPlayback(horn_data, horn_length);
  }

  // notify active playback (debug)
  digitalWrite(LED_BUILTIN, PLAYING);

  // blink my blinker acording to blinker_status
  static uint32_t blink_t = 0;
  static bool blinker_on = false;
  if ((blinker_status == 0) && (next_blinker_status != 0)) {
    blinker_status = next_blinker_status;
    blinker_on = true;
    digitalWrite(RIGHT, blinker_status & 1);
    digitalWrite(LEFT, blinker_status & 2);
    if (!PLAYING) startPlayback(tock_data, tock_length);
    blink_t = now;
  }
  else {
    if ((now - blink_t) > BLINK_PERIOD) {
      blink_t += BLINK_PERIOD;
      if (blinker_on) {
        blinker_on = false;
        digitalWrite(LEFT, 0);
        digitalWrite(RIGHT, 0);
        if (!PLAYING) startPlayback(tick_data, tick_length);
      }
      else {
        if (blinker_status != next_blinker_status)
          blinker_status = next_blinker_status;
        if (blinker_status != 0) {
          blinker_on = true;
          digitalWrite(RIGHT, blinker_status & 1);
          digitalWrite(LEFT, blinker_status & 2);
          if (!PLAYING) startPlayback(tock_data, tock_length);
        }
      }
    }
  }
}

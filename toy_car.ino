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
#define BTN_ALARM 15
#define BTN_LEFT 14

#define BTN_SAMPLING_MS 2
#define BTN_STABLE_CNT 8

#define PLAYING (TCCR2B&_BV(CS10))

typedef struct {
  bool btn;
  uint8_t cnt;
  uint8_t input;
} button_t;

button_t buttons[4];

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
  btn_init(&buttons[0], BTN_LIGHT);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  btn_init(&buttons[1], BTN_RIGHT);
  pinMode(BTN_ALARM, INPUT_PULLUP);
  buttons[2].input = BTN_ALARM;
  pinMode(BTN_LEFT, INPUT_PULLUP);
  buttons[3].input = BTN_LEFT;
}

void loop() {
  uint32_t now = millis();
  
  static bool btn_light = false;
  static uint32_t btn_time = 0;
  static uint8_t btn_light_cnt = 0;
  
  if ((now - btn_time) >= BTN_SAMPLING_MS) {
    btn_time += BTN_SAMPLING_MS;

    if (btn_filter(&buttons[0]))
      digitalWrite(LIGHTS, !digitalRead(LIGHTS));
    if (btn_filter(&buttons[1]))
      startPlayback(horn_data, horn_length);

  
  digitalWrite(LED_BUILTIN, PLAYING);
    
    /*uint8_t raw_in = PINC & 0xF;
    static uint8_t filter_in = 0xF;
    uint8_t mask = 0x1;
    static uint8_t filter_cnt[4];
    for (int i=0; i<4; i++) {
      if ((filter_in & mask) != (raw_in & mask)) {
        filter_cnt[i] ++;
        if (filter_cnt[i] >= BTN_STABLE_CNT)
      }
        
      if (filter_in & (1<<i)
    }
    if (btn_light == digitalRead(BTN_LIGHT)) {
      btn_light_cnt ++;
      if (btn_light_cnt >= BTN_STABLE_CNT) {
        btn_light = !btn_light;
        if (btn_light == false)
          digitalWrite(LIGHTS, !digitalRead(LIGHTS));
      }
    }
    else
      btn_light_cnt = 0;*/
  }

  static uint32_t blink_t = 0;
  if ((now - blink_t) > BLINK_PERIOD) {
    blink_t += BLINK_PERIOD;
    digitalWrite(LEFT, !digitalRead(LEFT));
    digitalWrite(RIGHT, !digitalRead(RIGHT));
    if (!PLAYING) {
      if (!digitalRead(LEFT))
        startPlayback(tick_data, tick_length);
      else
        startPlayback(tock_data, tock_length);
    }
  }
}

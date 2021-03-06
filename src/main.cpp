//#define __DEBUG__

#include <Arduino.h>
#include "debouncer.hpp"

typedef void (* FunctionPointer) (bool);

const int ssr1_pin_input = PD2; // switch
const int ssr1_pin_output = PD4;
const int ssr1_led_pin_output = PD6;
debounce_t ssr1_debounce;

const int ssr2_pin_input = PD3; // switch
const int ssr2_pin_output = PD5;
const int ssr2_led_pin_output = PD7;
debounce_t ssr2_debounce;

#ifdef __DEBUG__
const unsigned long auto_switch_after_ms = .3*60*1000l;
const unsigned long blink_after_ms = .1*60*1000l;
#else
const unsigned long auto_switch_after_ms = 6*60*60*1000l;
const unsigned long blink_after_ms = 5.5*60*60*1000l;
#endif

void display_current_ssr_status();
void set_led_status();
void ssr1_change_state(bool toggle);
void ssr1_check_button_state();
void ssr2_change_state(bool toggle);
void ssr2_check_button_state();
void auto_switch_off(debounce_t* ssr_debounce, int output_pin, FunctionPointer);

// the setup function runs once when you press reset or power the board
void setup() {
    //Start serial connection
#ifdef __DEBUG__
    Serial.begin(9600);
#endif

    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(ssr1_pin_input, INPUT);
    pinMode(ssr1_pin_output, OUTPUT);
    pinMode(ssr1_led_pin_output, OUTPUT);

    pinMode(ssr2_pin_input, INPUT);
    pinMode(ssr2_pin_output, OUTPUT);
    pinMode(ssr2_led_pin_output, OUTPUT);

    bool initial_state = LOW;
    digitalWrite(ssr1_pin_output, initial_state);
    digitalWrite(ssr1_led_pin_output, initial_state);
    digitalWrite(ssr2_pin_output, initial_state);
    digitalWrite(ssr2_led_pin_output, initial_state);
}

// the loop function runs over and over again forever
void loop() {
    ssr1_check_button_state();
    ssr2_check_button_state();

#ifdef __DEBUG__
    if (millis() % 5000 == 1) {
        display_current_ssr_status();
    }
#endif
}

void display_current_ssr_status() {
    Serial.print("SSR1 is ");
    Serial.print(ssr1_debounce.toggle);
    Serial.print(" SSR2 is ");
    Serial.println(ssr2_debounce.toggle);
}

void set_led_status() {
#ifdef __DEBUG__
    display_current_ssr_status();
#endif

    bool led_status = ssr1_debounce.toggle | ssr2_debounce.toggle;
    digitalWrite(LED_BUILTIN, led_status  ? HIGH : LOW);

#ifdef __DEBUG__
    Serial.print("LED_BUILTIN should be ");
      Serial.println(led_status);
#endif
}

void ssr1_change_state(bool toggle) {
#ifdef __DEBUG__
    Serial.print("Solid State Relay 1 should be ");
    Serial.println(toggle);
#endif
    digitalWrite(ssr1_pin_output, toggle ? HIGH : LOW);
    digitalWrite(ssr1_led_pin_output, toggle ? HIGH : LOW);
    set_led_status();
}
void ssr1_check_button_state() {
    int reading = digitalRead(ssr1_pin_input);
    debouncer(&ssr1_debounce, reading, ssr1_change_state);

    auto_switch_off(&ssr1_debounce, ssr1_led_pin_output, ssr1_change_state);
}

void ssr2_change_state(bool toggle) {
#ifdef __DEBUG__
    Serial.print("Solid State Relay 2 should be ");
    Serial.println(toggle);
#endif
    digitalWrite(ssr2_pin_output, toggle ? HIGH : LOW);
    digitalWrite(ssr2_led_pin_output, toggle ? HIGH : LOW);
    set_led_status();
}
void ssr2_check_button_state() {
    int reading = digitalRead(ssr2_pin_input);
    debouncer(&ssr2_debounce, reading, ssr2_change_state);

    auto_switch_off(&ssr2_debounce, ssr2_led_pin_output, ssr2_change_state);
}

void auto_switch_off(debounce_t* ssr_debounce, int output_pin, FunctionPointer ssr_func) {
    if (!ssr_debounce->toggle) {
        return;
    }
    if ((millis() - ssr_debounce->last_debounce_time) > blink_after_ms) {
        ssr_debounce->close_to_timeout = true;
        digitalWrite(output_pin, (millis() / 250) % 2);
    }
    if ((millis() - ssr_debounce->last_debounce_time) > auto_switch_after_ms) {
        ssr_debounce->close_to_timeout = false;
        ssr_debounce->last_debounce_time = millis();
        ssr_debounce->toggle = !ssr_debounce->toggle;
        ssr_func(ssr_debounce->toggle);
    }
}

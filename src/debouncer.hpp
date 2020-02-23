#include <Arduino.h>

/**
 * #include "debouncer.hpp"
 * 
 * const int pushPin = D2;
 * debounce_t push_debounce;
 * 
 * void setup() {
 *     pinMode(LED_BUILTIN, OUTPUT);
 *     pinMode(pushPin, INPUT);
 *     // if needed you can update the debounce delay 
 *     push_debounce.debounce_delay_ms = 60;
 * }
 * void push_action(bool toggle) {
 *     digitalWrite(LED_BUILTIN, toggle ? HIGH : LOW);
 * }
 * 
 * void push_button_read() {
 *     int reading = digitalRead(pushPin);
 *     debouncer(&push_debounce, reading, push_action);
 * }
 * 
 * void loop() {
 *     push_button_read();
 * }
 */
typedef void (* FunctionPointer) (bool);

struct debounce_t {
    int state{}; // the current reading from the input pin
    int last_state = LOW; // the previous reading from the input pin
    unsigned long last_debounce_time = 0; // the last time the output pin was toggled
    unsigned long debounce_delay_ms = 50;
    bool toggle = false;
};

void debouncer(debounce_t *debounce_struct, int reading, FunctionPointer func) {
    // debounce mechanisms
    if (reading != debounce_struct->last_state) {
        debounce_struct->last_debounce_time = millis();
    }
    if ((millis() - debounce_struct->last_debounce_time) > debounce_struct->debounce_delay_ms) {
        if (reading != debounce_struct->state) {
            debounce_struct->state = reading;

            if (debounce_struct->state == HIGH) {
                debounce_struct->toggle = !debounce_struct->toggle;
                #ifdef __DEBUG__
                Serial.print("debouncer run function : toggle ");
                Serial.println(debounce_struct->toggle);
                #endif
                func(debounce_struct->toggle);// function action
            }
        }
    }
    // save the reading. Next time through the loop, it'll be the lastButtonState:
    debounce_struct->last_state = reading;
}
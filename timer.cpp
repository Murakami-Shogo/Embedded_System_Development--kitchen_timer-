#include <Adafruit_LEDBackpack.h>
#include "EnpitShield.h"

Adafruit_7segment display = Adafruit_7segment();

enum State { NO_TIME_SET, TIME_SET, TIMER_STOPPED, COUNT_UP, COUNT_DOWN, ALARM };
State state = NO_TIME_SET;
State prevState = NO_TIME_SET;

const int buttonPins[] = {PIN_SW0, PIN_SW1,20,21,22,23,10,11};
bool buttonStates[8] = {false, false, false, false, false, false, false, false};

int minutes = 0;
int seconds = 0;

unsigned long prevMillis = 0;

void setup() {
    display.begin(0x70);
    display.setBrightness(15);

    for (int i = 0; i < 8; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
    }

    pinMode(PIN_LED0, OUTPUT); // LED
    pinMode(PIN_BUZZER, OUTPUT); // Buzzer

    for (int i = 0; i < 8; i++) {
        buttonStates[i] = digitalRead(buttonPins[i]) == LOW;
    }
}

void loop() {
    checkButtons();
    updateState();
    displayTime();

    if (state == COUNT_UP || state == COUNT_DOWN) {
        unsigned long currentMillis = millis();
        if (currentMillis - prevMillis >= 1000) {
            prevMillis = currentMillis;
            if (state == COUNT_UP) {
                incrementTime();
            } else if (state == COUNT_DOWN) {
                decrementTime();
            }
        }
    }

    if (state == ALARM) {
        digitalWrite(PIN_LED0, millis() % 1000 < 500 ? HIGH : LOW); // LED blink
        digitalWrite(PIN_BUZZER, HIGH); // Buzzer on
    } else {
        digitalWrite(PIN_LED0, LOW);
        digitalWrite(PIN_BUZZER, LOW); // Buzzer off
    }
}

void checkButtons() {
    for (int i = 0; i < 8; i++) {
        bool currentState = digitalRead(buttonPins[i]) == LOW;
        if (currentState != buttonStates[i]) {
            buttonStates[i] = currentState;
            if (currentState) {
                handleButtonPress(i);
            }
        }
    }
}

void handleButtonPress(int button) {
    switch (button) {
        case 0: // start/stop
            if (state == NO_TIME_SET) {
                state = COUNT_UP;
            } else if (state == TIME_SET) {
                state = COUNT_DOWN;
            } else if (state == COUNT_UP || state == COUNT_DOWN) {
                prevState = state;
                state = TIMER_STOPPED;
            } else if (state == TIMER_STOPPED) {
                state = prevState;
            }
            break;
        case 1: // reset
            state = NO_TIME_SET;
            minutes = 0;
            seconds = 0;
            break;
        case 2: // 1秒
            if (state == NO_TIME_SET || state == TIME_SET) {
                seconds += 1;
                normalizeTime();
                state = TIME_SET;
            }
            break;
        case 3: // 10秒
            if (state == NO_TIME_SET || state == TIME_SET) {
                seconds += 10;
                normalizeTime();
                state = TIME_SET;
            }
            break;
        case 4: // 1分
            if (state == NO_TIME_SET || state == TIME_SET) {
                minutes += 1;
                normalizeTime();
                state = TIME_SET;
            }
            break;
        case 5: // 10分
            if (state == NO_TIME_SET || state == TIME_SET) {
                minutes += 10;
                normalizeTime();
                state = TIME_SET;
            }
            break;
        case 6: // カップヌードル
            if (state == NO_TIME_SET || state == TIME_SET) {
                minutes = 3;
                seconds = 0;
                state = TIME_SET;
            }
            break;
        case 7: // どんべい
            if (state == NO_TIME_SET || state == TIME_SET) {
                minutes = 5;
                seconds = 0;
                state = TIME_SET;
            }
            break;
    }
}

void updateState() {
    if (state == COUNT_DOWN && minutes == 0 && seconds == 0) {
        state = ALARM;
    }
}

void displayTime() {
    int displayValue = minutes * 100 + seconds;
    display.print(displayValue, DEC);
    display.drawColon(true);
    display.writeDisplay();
}

void incrementTime() {
    seconds++;
    normalizeTime();
}

void decrementTime() {
    if (seconds == 0 && minutes == 0) {
        state = ALARM;
    } else {
        if (seconds == 0) {
            if (minutes > 0) {
                minutes--;
                seconds = 59;
            }
        } else {
            seconds--;
        }
    }
}

void normalizeTime() {
    if (seconds >= 60) {
        minutes += seconds / 60;
        seconds %= 60;
    }
    if (minutes >= 100) {
        minutes = 99;
        seconds = 59;
    }
}

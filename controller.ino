// LED
#include <FastLED.h>
#define LED_PIN     5
#define NUM_LEDS    300
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];


// COLOR
int hue = 100;
int lerp_start_hue = 100;
int to_hue = 100;
int saturation = 255;
float brightness = 20;
int max_brightness = 100;
// Rainbow
uint8_t speed = 6;
uint8_t speed_max = 100;
uint8_t dHue = 1;


// INPUT
const int pot1 = A0;
const int pot2 = A1;
int pot1_val;
int pot2_val;
int prev_pot1_val;
int prev_pot2_val;

int pot1_val_max = 1023; // 661;
int pot2_val_max = 1023; // 394;


// OUTPUT
const int button = 2;
int mode = 0;
const int modes = 4;
int current_state;
int last_state = HIGH;

const int LED1 = 10;
const int LED2 = 11;
const int LED3 = 12;
const int LED4 = 13;

// LERP
bool do_lerp = true;
const int lerp_length = 50;
int cur_lerp = 1;

void LERP() {
    hue = abs(lerp_start_hue - to_hue) / lerp_length * cur_lerp;
    cur_lerp++;
    set_color();
}

void set_color() {
    for (int i = 0; i < NUM_LEDS; i++) { leds[i] = CHSV(hue, saturation, brightness); }
}

void update_brightness() {
    prev_pot1_val = pot1_val;

    pot1_val = analogRead(pot1);

    if (prev_pot1_val != pot1_val) {
        brightness = map(pot1_val, 0, pot1_val_max, 0, 255);
        FastLED.setBrightness(min(brightness, max_brightness));
    }


    if (pot1_val > pot1_val_max) {
        pot1_val_max = pot1_val;
    }
}

void update_hue() {
    prev_pot2_val = pot2_val;

    pot2_val = analogRead(pot2);

    if (prev_pot2_val != pot2_val) {
        hue = map(pot2_val, 0, pot1_val_max, 0, 255);
        set_color();
    }

    if (pot2_val > pot2_val_max) {
        pot2_val_max = pot2_val;
    }
}

void update_speed() {
    speed = map(analogRead(pot2), 0, pot2_val_max, 0, speed_max);
}

void rainbow() {
    // hue = beat8(speed, 255);
    hue = (hue + 1) % 255;

    set_color();
}

void sin_rainbow() {
    hue = beat8(speed, 255);

    for(int i = 0; i < NUM_LEDS; i++) {
        int cur_hue = hue + (256 * i) / NUM_LEDS;
        leds[i] = CHSV(cur_hue, saturation, brightness);
    }
}

void change_mode() {
    mode = (mode + 1) % modes;

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(LED4, LOW);

    switch (mode) {
    case 0:
        digitalWrite(LED1, HIGH);

        lerp_start_hue = hue;
        hue = 0;
        set_color();
        break;
    
    case 1:
        digitalWrite(LED2, HIGH);
        break;

    case 2:
        digitalWrite(LED3, HIGH);
        break;
    
    case 3:
        digitalWrite(LED4, HIGH);
        break;
    }
}

void setup() {
    delay(2000);

    pinMode(button, INPUT_PULLUP);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);

    FastLED.addLeds<WS2812, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 10000); // 5V and 10A

    set_color();

    FastLED.show();
}

void loop() {
    // BUTTON
    current_state = digitalRead(button);
    if (last_state == LOW && current_state == HIGH) { change_mode(); }
    last_state = current_state;
    
    
    // POTENTIOMETERS
    update_brightness();

    switch (mode) {
    case 0:
        break;
    
    case 1:
        update_hue();
        break;

    case 2:
        update_speed();
        rainbow();
        break;
    
    case 3:
        update_speed();
        sin_rainbow();
        break;
    }


    FastLED.show();
    delay(1);
}
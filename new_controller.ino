// STD
#include <string>
#include <vector>
#include <memory>

// LED
#include <FastLED.h>
#define LED_PIN     5
#define NUM_LEDS    300
#define COLOR_ORDER GRB

// IO PINS
constexpr int PIN_LED1 =   10;
constexpr int PIN_LED2 =   11;
constexpr int PIN_LED3 =   12;
constexpr int PIN_LED4 =   13;
constexpr int PIN_BUTTON =  2;
constexpr int PIN_POT1 =   A0;
constexpr int PIN_POT2 =   A1;

// VALUES
constexpr int LERP_LENGTH =         50;
constexpr int START_BRIGHTNESS =    20;
constexpr int MAX_BRIGHTNESS =     100;
constexpr int MAX_VOLTAGE =          5; // V
constexpr int MAX_AMPERE =       10000; // A


// -- BUTTON --
class BUTTON
{
private:
    const int pin;
    int curr_state;
    int prev_state;
    bool ret_val;

public:
    BUTTON(int pin);
    bool pressed();
};

BUTTON::BUTTON(int pin) 
    :   pin { pin }, 
        curr_state { HIGH }, 
        prev_state { HIGH }, 
        ret_val { false }
{
    pinMode(pin, INPUT_PULLUP);
}

bool BUTTON::pressed()
{
    curr_state = digitalRead(pin);

    ret_val = prev_state == LOW && curr_state == HIGH;

    prev_state = curr_state;

    return ret_val;
}


// -- POTENTIOMETER --
class POTENTIOMETER
{
private:
    const int pin;
    int curr_val;
    int prev_val;
    const int max;

public:
    POTENTIOMETER(int pin);
    bool changed();
    int value() const;
};

POTENTIOMETER::POTENTIOMETER(int pin)
    :   pin { pin }, 
        curr_val { 0 }, 
        prev_val { 0 }, 
        max { 1023 } {}

bool POTENTIOMETER::changed()
{
    prev_val = curr_val;
    curr_val = analogRead(pin);

    return prev_val != curr_val;
}

int POTENTIOMETER::value() const
{
    return map(curr_val, 0, max, 0, 255);
}


class BASE_MODE
{
public:
    BASE_MODE(std::string name, bool update_h);
    virtual void update(CRGB (&leds)[], int hue, int saturation, int brightness) = 0;
    std::string get_name() const;
    bool update_hue() const { return update_h; };

protected:
    const std::string name;
    const bool update_h;
};

BASE_MODE::BASE_MODE(std::string name, bool update_h)
    :   name { name },
        update_h { update_h} {}

std::string BASE_MODE::get_name() const 
{
    return name;
}


// -- RED light strip --
class RED : public BASE_MODE
{
public:
    RED(std::string name);
    void update(CRGB (&leds)[], int hue, int saturation, int brightness) override;
};

RED::RED(std::string name)
    :   BASE_MODE(name, false) {}

void RED::update(CRGB (&leds)[], int hue, int saturation, int brightness)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(0, saturation, brightness);  // Hue = 0 -> Red
    }
}


// -- RAINBOW light strip --
class RAINBOW : public BASE_MODE
{
public:
    RAINBOW(std::string name);
    void update(CRGB (&leds)[], int hue, int saturation, int brightness) override;
};

RAINBOW::RAINBOW(std::string name)
    :   BASE_MODE(name, false) {}

void RAINBOW::update(CRGB (&leds)[], int hue, int saturation, int brightness)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(255 / NUM_LEDS * i, saturation, brightness);  // Hue = 0 -> Red
    }
}


// -- COLOR light strip --
class HUE : public BASE_MODE
{
public:
    HUE(std::string name);
    void update(CRGB (&leds)[], int hue, int saturation, int brightness) override;
};

HUE::HUE(std::string name)
    :   BASE_MODE(name, true) {}

void RAINBOW::update(CRGB (&leds)[], int hue, int saturation, int brightness)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue, saturation, brightness);
    }
}


// -- MODE --
class MODE
{
private:
    int mode;
    std::vector<std::unique_ptr<BASE_MODE>> modes;

public:
    MODE();
    void operator++();
    void update(CRGB (&leds)[], int hue, int saturation, int brightness);
    std::string get_name() const;
    bool update_hue() const;
};

MODE::MODE()
    :   mode { 0 },
        modes { std::vector<std::unique_ptr<BASE_MODE>>(2) }
{
    modes[0] = std::make_unique<RED>("RED");
    modes[1] = std::make_unique<RAINBOW>("Rainbow");
}

void MODE::operator++()
{
    mode = (mode + 1) % modes.size();
}

void MODE::update(CRGB (&leds)[], int hue, int saturation, int brightness)
{
    modes[mode] -> update(leds, hue, saturation, brightness);
}

std::string MODE::get_name() const
{
    return modes[mode] -> get_name();
}

bool MODE::update_hue() const
{
    return modes[mode] -> update_hue();
}


// -- CONTROLLER --
class CONTROLLER
{
private:
    // Hardware
    BUTTON button;
    POTENTIOMETER pot_hue;
    POTENTIOMETER pot_brightness;

    // LED strip
    CRGB leds[NUM_LEDS];
    CRGB temp_leds[NUM_LEDS];

    // Mode
    MODE mode;

    // Linear interpolation
    bool do_lerp;
    const double lerp_length;
    int cur_lerp;

    void change_mode();
    void update_hue();

public:
    CONTROLLER(int button_pin, int pot_hue_pin, int pot_brightness_pin, int lerp_length);

    void update();
};

CONTROLLER::CONTROLLER(int button_pin, int pot_hue_pin, int pot_brightness_pin, int lerp_length)
    :   button { BUTTON(button_pin) },
        pot_hue { POTENTIOMETER(pot_hue_pin) },
        pot_brightness { POTENTIOMETER(pot_brightness_pin) },
        mode { MODE() },
        do_lerp { false },
        lerp_length { lerp_length },
        cur_lerp { 1 } 
{
    FastLED.addLeds<WS2812, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTAGE, MAX_AMPERE);

    // Fade in
    for (int i = 0; i < START_BRIGHTNESS; i++) {
        mode.update(leds, 0, 255, i);
        FastLED.show();
        delay(50);
    }
}

void CONTROLLER::update()
{
    // CHECK BUTTON
    if (button.pressed()) change_mode();

    // CHECK POTENTIOMETER HUE
    if (mode.update_hue())
    {
        if (pot_hue.changed()) update_hue();
    }

    // CHECK POTENTIOMETER BRIGHTNESS
    if (pot_brightness.changed())
    {
        FastLED.setBrightness(min(pot_hue.value(), MAX_BRIGHTNESS));
    }

}

void CONTROLLER::update_hue()
{
    if (do_lerp)
    {
        mode.update(temp_leds, pot_hue.value(), 255, pot_brightness.value());

        for (int i = 0; i < NUM_LEDS; i++) {
            leds[i] = (uint8_t)leds[i] + (uint8_t)(abs((uint8_t)leds[i] - (uint8_t)temp_leds[i]) / lerp_length * cur_lerp);
        }

        cur_lerp++;
        if (cur_lerp == lerp_length)
        {
            do_lerp = false;
            cur_lerp = 1;
        }

    }
    else
    {
        mode.update(leds, pot_hue.value(), 255, pot_brightness.value());
    }
}
// KOMMENTAR:
// Kanskje kun ha pot_brightness.velue på FastLED.setBright og ikke i hsv

void CONTROLLER::change_mode()
{
    ++mode;
    do_lerp = true;
    cur_lerp = 1;
}


// SETUP AND LOOP
CONTROLLER controller;

void setup()
{
    delay(2000); // For hardware safety

    CONTROLLER controller(PIN_BUTTON, PIN_A0, PIN_A1, LERP_LENGTH);
}

void loop()
{
    controller.update();
    FastLED.show();

    delay(1);
}
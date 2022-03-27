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
    bool update();
    int value() const;
};

POTENTIOMETER::POTENTIOMETER(int pin)
    :   pin { pin }, 
        curr_val { 0 }, 
        prev_val { 0 }, 
        max { 1023 } {}

bool POTENTIOMETER::update()
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
    BASE_MODE(std::string name);
    virtual void update(CRGB& leds, int saturation, int brightness) = 0;
    std::string get_name() const;

protected:
    const std::string name;
};

BASE_MODE::BASE_MODE(std::string name)
    :   name { name } {} 

std::string BASE_MODE::get_name() const 
{
    return name;
}


// -- RED light strip --
class RED : public BASE_MODE
{
public:
    RED(std::string name);
    void update(CRGB& leds, int saturation, int brightness) override;
};

RED::RED(std::string name)
    :   BASE_MODE(name) {}

void RED::update(CRGB& leds, int saturation, int brightness)
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
    void update(CRGB& leds, int saturation, int brightness) override;
};

RAINBOW::RAINBOW(std::string name)
    :   BASE_MODE(name) {}

void RAINBOW::update(CRGB& leds, int saturation, int brightness)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(100, saturation, brightness);  // Hue = 0 -> Red
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
    void update(CRGB& leds, int saturation, int brightness);
    std::string get_name() const;
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

void MODE::update(CRGB& leds, int saturation, int brightness)
{
    modes[mode] -> update(leds, saturation, brightness);
}

std::string MODE::get_name() const
{
    return modes[mode] -> get_name();
}


// -- CONTROLLER --
class CONTROLLER
{
private:
    // Hardware
    BUTTON button;
    POTENTIOMETER pot_brightness;
    POTENTIOMETER pot_hue;

    // LED strip
    CRGB leds[NUM_LEDS];
    CRGB temp_leds[NUM_LEDS];

    // Mode
    enum class mode;
    mode operator++(mode& m);

    // Linear interpolation
    bool do_lerp;
    const int lerp_length;
    int cur_lerp;

    void change_mode();

public:
    CONTROLLER();

    void update();
};

void CONTROLLER::update()
{

}

void setup()
{

}

void loop()
{

}
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
    : pin { pin }, curr_state { HIGH }, prev_state { HIGH }, ret_val { false } {}

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
    : pin { pin }, curr_val { 0 }, prev_val { 0 }, max { 1023 } {}

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


class MODE
{
public:
    MODE();

protected:
    virtual void update() = 0;
};

class RAINBOW : public MODE
{

};

// -- CONTROLLER --
class CONTROLLER
{
private:
    BUTTON button;
    POTENTIOMETER pot_brightness;
    POTENTIOMETER pot_hue;
    CRGB leds[NUM_LEDS];

public:
    CONTROLLER();

    void update();
    void change_mode();
};

void setup()
{

}

void loop()
{

}
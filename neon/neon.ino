#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <Arduino.h>
#include <LittleVector.h>



// TODO: move classes to external .h and .cpp files, this one is getting crowded...

class Colour {
    public:
    int r, g, b;

    Colour() {
        this->r = 0;
        this->g = 0;
        this->b = 0;
    }

    Colour(int r, int g, int b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }

    Colour calculateComplent() {
        /*calculate the current colour's complentary colour. Mostly for testing purposes*/
        return Colour(255-this->r, 255-this->g, 255-this->b);
    }
};

class Section {
    public:
    // colour stuff
    Colour colour;
    Colour complement;

    LittleVector<Colour> up;
    LittleVector<Colour> down;
    LittleVector<Colour> updown;

    // LED stuff
    Adafruit_NeoPixel strip;
    int stripLength;

    // animation helpers
    int modeSelection = 0;
    int animationStep = 0;
    int animationMaxStep = 100;

    Section(Colour colour, Colour complement, int stripLength, Adafruit_NeoPixel strip) {
        this->colour = colour;
        this->complement = complement;
        this->stripLength = stripLength;
        this->strip = strip;
        this->up.reserve(stripLength);
        this->down.reserve(stripLength);
        this->updown.reserve(stripLength);
    }

    void initialise() {
        Serial.println("initialising...");

        // calculate colour gradients based on https://bsouthga.dev/posts/color-gradients-with-python 
        // That's weird python to start with. Now it's translated to C++ by someone who doesn't know C++. 

        // calculate up gradient, every second colour is added to updown
        up.push_back(colour);
        updown.push_back(colour);
        int r, g, b;
        for (int led = 1; led < stripLength; led++) {
            r = (int) (colour.r + ((float) led / (stripLength -1)) * (complement.r - colour.r));
            g = (int) (colour.g + ((float) led / (stripLength -1)) * (complement.g - colour.g));
            b = (int) (colour.b + ((float) led / (stripLength -1)) * (complement.b - colour.b));
            up.push_back(Colour(r, g, b));
            if (led % 2 == 0) {
                updown.push_back(Colour(r, g, b));
            }
        }

        // calculate down gradient, every second colour is added to updown
        down.push_back(complement);
        for (int led = 1; led < stripLength; led++) {
            r = (int) (complement.r + ((float) led / (stripLength -1)) * (colour.r - complement.r));
            g = (int) (complement.g + ((float) led / (stripLength -1)) * (colour.g - complement.g));
            b = (int) (complement.b + ((float) led / (stripLength -1)) * (colour.b - complement.b));
            down.push_back(Colour(r, g, b));
            if (led % 2 == 0) {
                updown.push_back(Colour(r, g, b));
            }
        }
        updown.push_back(colour);


        char strBuf[50];
        Serial.println("vector up:");
        for (int i = 0; i < up.size(); i++) {
            sprintf(strBuf, "r: %d g: %d b: %d", up[i].r, up[i].g, up[i].b);
            Serial.println(strBuf);
        }

        Serial.println("vector down:");
        for (int i = 0; i < down.size(); i++) {
            sprintf(strBuf, "r: %d g: %d b: %d", down[i].r, down[i].g, down[i].b);
            Serial.println(strBuf);
        }

        Serial.println("vector updown:");
        for (int i = 0; i < updown.size(); i++) {
            sprintf(strBuf, "r: %d g: %d b: %d", updown[i].r, updown[i].g, updown[i].b);
            Serial.println(strBuf);
        }
                
        // initialise strip
        strip.begin();
        strip.setBrightness(32); // TODO: debug setting --> change/ remove
        strip.clear();
        strip.show();

        animate();
    }

    void animate() {
        // I want my f-strings back. Fuck C++, all my homies hate C++
        char strBuf[50];
        sprintf(strBuf, "Animation step: %d/%d", animationStep, animationMaxStep);
        Serial.println(strBuf);

        switch (modeSelection) {
        case 0:
            doAnimateSolid();
            break;
        case 1:
            doAnimateAlternating();
            break;
        case 2:
            doAnimateGradientConstant();
            break;
        case 3:
            doAnimateGradientBreathing();
            break;
        case 4: 
            doAnimateGradientSlide();
            break;
        default:
            Serial.println("\tdefault case");
            // if we don't have a handler for this mode, reset to base
            // I don't know how to create a list of function references in C++ and I don't want
            // to constantly update a modulo value while adding new stuff
            modeSelection = 0;
            animate();
            break;
        }

        animationStep = (animationStep + 1) % animationMaxStep;

    }

    void doAnimateSolid() {
        Serial.println("\tsolid");
        strip.clear();

        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, colour.r, colour.g, colour.b);
        }
        strip.show();
    }

    void doAnimateAlternating() {
        Serial.println("\talternating");
        strip.clear();

        // two lines in python (: 
        Colour a, b;
        if (animationStep % 2 == 0) {
            a = colour;
            b = complement;
        } else {
            a = complement;
            b = colour;
        }

        for (int i = 0; i < stripLength; i+=2) {
            strip.setPixelColor(i, a.r, a.g, a.b);
            strip.setPixelColor(i+1, b.r, b.g, b.b);
        }
        strip.show();
    }

    void doAnimateGradientConstant() {
        Serial.println("\tgradient constant");  
        strip.clear();

        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, up[i].r, up[i].g, up[i].b);
        }
        strip.show();
    }

    void doAnimateGradientBreathing() {
        Serial.println("\t gradient breathing");
        strip.clear();

        // can't use precalculated gradients, we want more detail for breathing
        int localAnimationMax = animationMaxStep / 2;
        int r, g, b;
        if (animationStep < localAnimationMax) {
            r = (int) (colour.r + ((float) animationStep / (localAnimationMax -1)) * (complement.r - colour.r));
            g = (int) (colour.g + ((float) animationStep / (localAnimationMax -1)) * (complement.g - colour.g));
            b = (int) (colour.b + ((float) animationStep / (localAnimationMax -1)) * (complement.b - colour.b));
        } else {
            r = (int) (complement.r + ((float) (animationStep - localAnimationMax) / (localAnimationMax -1)) * (colour.r - complement.r));
            g = (int) (complement.g + ((float) (animationStep - localAnimationMax) / (localAnimationMax -1)) * (colour.g - complement.g));
            b = (int) (complement.b + ((float) (animationStep - localAnimationMax) / (localAnimationMax -1)) * (colour.b - complement.b));
        }

        Colour current = Colour(r, g, b);

        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, current.r, current.g, current.b);
        }

        strip.show();
    }

    void doAnimateGradientSlide() {
        Serial.println("\t gradient slide");
        strip.clear();

        float animationPercent = (float) animationStep / animationMaxStep;
        int offset = (int) (animationPercent * stripLength);

        for (int i = 0; i < stripLength; i++) {
            int colourIndex = (i + offset) % stripLength;
            char strBuf2[100];
            int r, g, b;
            r = updown[colourIndex].r;
            g = updown[colourIndex].g;
            b = updown[colourIndex].b;
            strip.setPixelColor(i, r, g, b);
        }

        strip.show();
    }

    void nextAnimation() {
        modeSelection = modeSelection + 1;
    }
};

// https://colorcodes.io/neon-color-codes/
Colour NEON_BLUE = Colour(77, 77, 255);
Colour NEON_PURPLE = Colour(199, 36, 177);
Colour NEON_RED = Colour(210, 39, 48);
Colour NEON_GREEN = Colour(68, 214, 44);
Colour NEON_ORANGE = Colour(255, 173, 0);

Colour DEBUG_ON = Colour(255, 255, 255);
Colour DEBUG_OFF = Colour(0, 0, 0);

// setup for strips and sections
int krummePin = A4;
int krummeLength = 60;
Adafruit_NeoPixel krummeStrip = Adafruit_NeoPixel(krummeLength, krummePin, NEO_GRB + NEO_KHZ800);
Section krumme = Section(NEON_BLUE, NEON_PURPLE, 60, krummeStrip);

int gemeindePin = A5;
int gemeindeLength = 20;
Adafruit_NeoPixel gemeindeStrip = Adafruit_NeoPixel(gemeindeLength, gemeindePin, NEO_GRB + NEO_KHZ800);
Section gemeinde = Section(NEON_RED, NEON_GREEN, 20, gemeindeStrip);

int borderPin = A6;
int borderLength = 20;
Adafruit_NeoPixel borderStrip = Adafruit_NeoPixel(borderLength, borderPin, NEO_GRB + NEO_KHZ800);
Section border = Section(DEBUG_ON, DEBUG_OFF, 20, borderStrip);



// setup for buttons and dials
const byte buttonPin = 0;
const byte potPin = A3;
int animationDelay = 100;
const int animationDelayMax = 500; // slowest: 1000/val steps per second. fastest: implicit 1 ms delay --> 1000 steps per second


void setup() {
    Serial.begin(9600);
    delay(5000);
    Serial.println("setup start");

    krumme.initialise();
    gemeinde.initialise();
    border.initialise();

    // set up button
    // TODO: use this instead of interrupt to avoid hardware bounce
    // https://github.com/thomasfredericks/Bounce2
    pinMode(buttonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(buttonPin), button, RISING); // trigger interrupt when button is released

    // set up potentiometer
    pinMode(potPin, INPUT);
    Serial.println("setup end");
}

void loop() {
    krumme.animate();
    gemeinde.animate();
    border.animate();
    
    int potValue = analogRead(potPin); /// 10 bit
    // normalize from 0-1023 to 1-1000
    float percent = (float) potValue / 1023;
    animationDelay = percent * animationDelayMax;
    
    delay(animationDelay);
}

void button() {
    krumme.nextAnimation();
    gemeinde.nextAnimation();
    border.nextAnimation();
}
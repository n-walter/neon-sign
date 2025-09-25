#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <Arduino.h>

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
    Colour colour;
    Colour complement;
    Adafruit_NeoPixel strip;
    int stripLength;
    int modeSelection = 0;
    int animationStep = 0;
    int animationMaxStep = 100;

    Section(Colour colour, Colour complement, int stripLength, Adafruit_NeoPixel strip) {
        this->colour = colour;
        this->complement = complement;
        this->stripLength = stripLength;
        this->strip = strip;
    }

    void initialise() {
        Serial.println("initialising...");
        
        // initialise strip
        strip.begin();
        strip.setBrightness(32);
        strip.clear();
        strip.show();

        animate();
    }

    /*

    TODO:
    Animation plan: add tick counter (e.g. 100 states ("frames")?)
    animation functions use tick counter to calculate values
    i.e. pulse will calculate brightness based on tick counter
    i.e. gradient will shift pixels in row based on tick counter

    */

    void animate() {
        Serial.println("animating...");
        Serial.println(modeSelection);
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

        // I want my f-strings back. Fuck C++, all my homies hate C++
        char strBuf[50];
        sprintf(strBuf, "Animation step: %d/%d", animationStep, animationMaxStep);
        Serial.println(strBuf);
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
        /* https://bsouthga.dev/posts/color-gradients-with-python 
        
        That's weird python to start with. Now it's translated to C++ by someone who doesn't know C++. 
        */
        Serial.println("\tgradient constant");  
        Colour gradient[stripLength];
        gradient[0] = colour;
        for (int step = 1; step < stripLength; step++) {
            int r,g,b;
            r = (int) (colour.r + ((float) step / (stripLength -1)) * (complement.r - colour.r));
            g = (int) (colour.g + ((float) step / (stripLength -1)) * (complement.g - colour.g));
            b = (int) (colour.b + ((float) step / (stripLength -1)) * (complement.b - colour.b));
            gradient[step] = Colour(r, g, b);
        }
        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, gradient[i].r, gradient[i].g, gradient[i].b);
        }
        strip.show();
    }

    void doAnimateGradientBreathing() {
        Serial.println("\t gradient breathing");
        int r = (int) (colour.r + ((float) animationStep / (animationMaxStep -1)) * (complement.r - colour.r));
        int g = (int) (colour.g + ((float) animationStep / (animationMaxStep -1)) * (complement.g - colour.g));
        int b = (int) (colour.b + ((float) animationStep / (animationMaxStep -1)) * (complement.b - colour.b));
        Colour current = Colour(r, g, b);

        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, current.r, current.g, current.b);
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
Section border = Section(NEON_ORANGE, NEON_ORANGE, 20, borderStrip);

// setup for buttons and dials
const byte buttonPin = 0;


void setup() {
    Serial.begin(9600);
    delay(5000);
    Serial.println("setup start");

    krumme.initialise();
    gemeinde.initialise();
    border.initialise();

    // set up button
    pinMode(buttonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(buttonPin), button, RISING); // trigger interrupt when button is released

    Serial.println("setup end");
}

void loop() {
    krumme.animate();
    gemeinde.animate();
    border.animate();

    delay(100); // 1 animation per second for debugging, should easily handle 10, probably 100 later
}

void button() {
    krumme.nextAnimation();
    gemeinde.nextAnimation();
    border.nextAnimation();
}
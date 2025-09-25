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
    Adafruit_NeoPixel strip;
    int stripLength;
    int modeSelection = 0;

    Section(Colour colour, Adafruit_NeoPixel strip, int stripLength) {
        this->colour = colour;
        this->strip = strip;
        this->stripLength = stripLength;
    }

    void initialise() {
        Serial.println("initialising...");
        // initialise strip
        strip.begin();
        strip.setBrightness(32);
        strip.clear();
        strip.show();

        // 
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
            Serial.println("\tcase 0");
            doAnimateSolid();
            break;
        case 1:
            Serial.println("\tcase 1");
            doAnimateAlternating();
            break;
        case 2:
            Serial.println("\tcase 3");
            doAnimateGradient();
            break;
        default:
            Serial.println("\tdefault");
            // if we don't have a handler for this mode, reset to base
            // I don't know how to create a list of function references in C++ and I don't want
            // to constantly update a modulo value while adding new stuff
            modeSelection = 0;
            animate();
            break;
        }
    }

    void doAnimateSolid() {
        Serial.println("\t\tsolid");
        strip.clear();
        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, colour.r, colour.g, colour.b);
        }
        strip.show();
    }

    void doAnimateAlternating() {
        Serial.println("\t\talternating");
        strip.clear();
        for (int i = 0; i < stripLength; i++) {
            if (i % 2 == 0) { // equivalent of python truthy or falsy values in C++? cast to bool?
                strip.setPixelColor(i, colour.r, colour.g, colour.b);
            } else {
                Colour complement = colour.calculateComplent();
                strip.setPixelColor(i, complement.r, complement.g, complement.b);
            }
        }
        strip.show();
    }

    void doAnimateGradient() {
        /* https://bsouthga.dev/posts/color-gradients-with-python 
        
        That's weird python to start with. Now it's translated to C++ who doesn't know C++. 
        */
        Serial.println("\t\tgradient");  
        Colour complement = colour.calculateComplent();
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

    void nextAnimation() {
        modeSelection = modeSelection + 1;
    }
};

int stripPin1 = A4;
int stripPin2 = A5;
int stripPin3 = A6;
Colour c1, c2, c3;
int NUM_PIXELS = 20;

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_PIXELS, stripPin1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM_PIXELS, stripPin2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(NUM_PIXELS, stripPin3, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strips[3] = {strip1, strip2, strip3};

// https://colorcodes.io/neon-color-codes/
Colour NEON_RED = Colour(210, 39, 48);
Colour NEON_BLUE = Colour(77, 77, 255);
Colour NEON_ORANGE = Colour(255, 173, 0);

Section krumme = Section(NEON_RED, strip1, 20);
Section gemeinde = Section(NEON_BLUE, strip2, 20);
Section border = Section(NEON_ORANGE, strip3, 20);

const byte buttonPin = 0;
int buttonState = 0;



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

    delay(1000); // 1 animation per second for debugging, should easily handle 10, probably 100 later
}

void button() {
    krumme.nextAnimation();
    gemeinde.nextAnimation();
    border.nextAnimation();
}
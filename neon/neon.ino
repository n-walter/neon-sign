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

        // TODO: use vectors for gradient steps; calculate gradient (as well as reverse and breathing) here instead of during animation
        
        // initialise strip
        strip.begin();
        strip.setBrightness(32);
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

        /* https://bsouthga.dev/posts/color-gradients-with-python 
        
        That's weird python to start with. Now it's translated to C++ by someone who doesn't know C++. 
        */
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
        strip.clear();

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

        Colour gradient[stripLength];
        gradient[0] = colour;

        // only calculate half of gradient, second half is reverse
        for (int step = 1; step < stripLength/2; step++) {
            int r,g,b;
            r = (int) (colour.r + ((float) step / (stripLength -1)) * (complement.r - colour.r));
            g = (int) (colour.g + ((float) step / (stripLength -1)) * (complement.g - colour.g));
            b = (int) (colour.b + ((float) step / (stripLength -1)) * (complement.b - colour.b));
            gradient[step] = Colour(r, g, b);
        }
        for (int step = stripLength / 2; step < stripLength; step++) {
            gradient[step] = gradient[stripLength - step]; // wrong: starting again from front to back instead of back to front
        }
        
        float animationPercent = (float) animationStep / animationMaxStep;
        int offset = (int) (animationPercent * stripLength);

        char strBuf[100];
        sprintf(strBuf, "offset: %d", offset);
        Serial.println(strBuf);

        for (int i = 0; i < stripLength; i++) {
            int colourIndex = (i + offset) % stripLength;
            char strBuf2[100];
            int r, g, b;
            r = gradient[colourIndex].r;
            g = gradient[colourIndex].g;
            b = gradient[colourIndex].b;
            sprintf(strBuf2, "index: %d colourIndex: %d r: %d g: %d b: %d", i, colourIndex, r, g, b);
            Serial.println(strBuf2);
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
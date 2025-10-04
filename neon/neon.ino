#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <Arduino.h>
#include <LittleVector.h>



// TODO: move classes to external .h and .cpp files, this one is getting crowded...

class ColourPair {
    public:
    Colour colour;
    Colour complement;

    LittleVector<Colour> up;
    LittleVector<Colour> down;
    LittleVector<Colour> updown;

    ColourPair(Colour colour, Colour complement) {
        this->colour= colour;
        this->complement = complement;
        this->up.reserve(stripLength);
        this->down.reserve(stripLength);
        this->updown.reserve(stripLength);
    }

    void initialise(int stripLength) {
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
    }
}

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
    ColourPair pairA;
    ColourPair pairB;
    ColourPair pairC;

    // LED stuff
    Adafruit_NeoPixel strip;
    int stripLength;

    // animation helpers
    int modeSelection = 0;
    int animationStep = 0;
    int animationMaxStep = 100;

    Section(ColourPair pairA, ColourPair pairB, ColourPair pairC, int stripLength, Adafruit_NeoPixel strip) {
        this->pairA = pairA;
        this->pairB = pairB;
        this->pairC = pairC;
        
        this->stripLength = stripLength;
        this->strip = strip;
    }

    void initialise() {
        Serial.println("initialising strip...");

                
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
            doAnimateSolid(true);
            break;
        case 1:
            doAnimateSolid(false);
            break;
        case 2:
            doAnimateAlternating(pairA, 5);
            break;
        case 3:
            doAnimateAlternating(pairB, 5);
            break;
        case 4:
            doAnimateAlternating(pairC, 5);
            break;
        case 5: 
            doAnimateGradientSlide();
            break;
        case 6:
            doAnimateRandomALl();
            break;
        case 7:
            doAnimateRandomSingle();
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

    void doAnimateSolid(bool reverse) {
        Serial.println("\tsolid");
        strip.clear();

        for (int i = 0; i < stripLength; i++) {
            if (reverse) {
                strip.setPixelColor(i, pairA.colour.r, pairA.colour.g, pairA.colour.b);
            } else {
                strip.setPixelColor(i, pairA.complement.r, pairA.complement.g, pairA.complement.b);
            }
        }
        strip.show();
    }



    void doAnimateAlternating(ColourPair pair; int blockSize = 1) {
        Serial.println("\talternating");
        strip.clear();

        // two lines in python (: 
        Colour a, b;
        if (animationStep % 2 == 0) {
            a = pair.colour;
            b = pair.complement;
        } else {
            a = pair.complement;
            b = pair.colour;
        }

        for (int i = 0; i < stripLength; i += (2 * blockSize)) {
            strip.setPixelColor(i, a.r, a.g, a.b);
            strip.setPixelColor(i+blockSize, b.r, b.g, b.b);
        }
        strip.show();
    }

    /*
    unused
    void doAnimateGradientConstant() {
        Serial.println("\tgradient constant");  
        strip.clear();

        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, up[i].r, up[i].g, up[i].b);
        }
        strip.show();
    }
    */

    /* unused
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
    */

    void doAnimateGradientSlide(ColourPair pair) {
        Serial.println("\t gradient slide");
        strip.clear();

        float animationPercent = (float) animationStep / animationMaxStep;
        int offset = (int) (animationPercent * stripLength);

        for (int i = 0; i < stripLength; i++) {
            int colourIndex = (i + offset) % stripLength;
            char strBuf2[100];
            int r, g, b;
            r = pair.updown[colourIndex].r;
            g = pair.updown[colourIndex].g;
            b = pair.updown[colourIndex].b;
            strip.setPixelColor(i, r, g, b);
        }

        strip.show();
    }

    void doAnimateRandomALl() {
        Serial.println("\ all random");
        strip.clear();
        int r, g, b;
        for (int i = 0; i < stripLength; i++) {
            r = rand() % 255;
            g = rand() % 255;
            b = rand() % 255;

            strip.setPixelColor(i, r, g, b);
        }
        strip.show();
    }

    void doAnimateRandomSingle() {
        Serial.println("\ single random");
        strip.clear();
        int r, g, b;
        r = rand() % 255;
        g = rand() % 255;
        b = rand() % 255;
        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, r, g, b);
        }
        strip.show();
    }

    void nextAnimation() {
        modeSelection = modeSelection + 1;
    }
};

// https://colorcodes.io/neon-color-codes/
Colour NEON_RED = Colour(210, 39, 48);
Colour NEON_WHITE = Colour(255, 255, 255);
ColourPair PAIR_1 = ColourPair(NEON_WHITE, NEON_RED);
ColourPair PAIR_1_Reverse = ColourPair(NEON_RED, NEON_WHITE);

Colour NEON_BLUE = Colour(77, 77, 255);
Colour NEON_PURPLE = Colour(199, 36, 177);
ColourPair PAIR_2 = ColourPair(NEON_BLUE, NEON_PURPLE);

Colour NEON_GREEN = Colour(68, 214, 44);
Colour NEON_ORANGE = Colour(255, 173, 0)
ColourPair PAIR_3 = ColourPair(NEON_GREEN, NEON_ORANGE);


// setup for strips and sections
int krummePin = A4;
int krummeLength = 60;
Adafruit_NeoPixel krummeStrip = Adafruit_NeoPixel(krummeLength, krummePin, NEO_GRB + NEO_KHZ800);
Section krumme = Section(NEON_BLUE, NEON_YELLOW, 60, krummeStrip);

int gemeindePin = A5;
int gemeindeLength = 20;
Adafruit_NeoPixel gemeindeStrip = Adafruit_NeoPixel(gemeindeLength, gemeindePin, NEO_GRB + NEO_KHZ800);
Section gemeinde = Section(NEON_GREEN, NEON_ORANGE, 20, gemeindeStrip);

int borderPin = A6;
int borderLength = 20;
Adafruit_NeoPixel borderStrip = Adafruit_NeoPixel(borderLength, borderPin, NEO_GRB + NEO_KHZ800);
Section border = Section(DEBUG_ON, DEBUG_OFF, 20, borderStrip);



// setup for buttons and dials
const byte animationButtonPin = 0;
const byte borderOffPin = 1;
const byte animationSpeedPotPin = A3;
int animationDelay = 100;
const int animationDelayMax = 500; // slowest: 1000/val steps per second. fastest: implicit 1 ms delay --> 1000 steps per second




void setup() {
    Serial.begin(9600);
    delay(5000); // TODO: debug setting --> change/ remove
    Serial.println("setup start");

    krumme.initialise();
    gemeinde.initialise();
    border.initialise();

    // set up buttons
    // TODO: use this instead of interrupt to avoid hardware bounce
    // https://github.com/thomasfredericks/Bounce2
    pinMode(animationButtonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(animationButtonPin), nextAnimationButton, RISING); // trigger interrupt when button is released
    
    pinMode(borderOffPin, INPUT_PULLUP);


    // set up potentiometer
    pinMode(animationSpeedPotPin, INPUT);
    Serial.println("setup end");
}

void loop() {
    krumme.animate();
    gemeinde.animate();
    border.animate();
    
    int potValue = analogRead(animationSpeedPotPin); /// 10 bit
    // normalize from 0-1023 to 1-1000
    float percent = (float) potValue / 1023;
    animationDelay = percent * animationDelayMax;
    
    delay(animationDelay);
}

void nextAnimationButton() {
    krumme.nextAnimation();
    gemeinde.nextAnimation();
    border.nextAnimation();
}

void borderOffButton() {
    // TODO: border.strip.setBrightness(0);
}


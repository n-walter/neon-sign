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
};

class ColourPair {
    public:
    Colour colour;
    Colour complement;

    LittleVector<Colour> up;
    LittleVector<Colour> down;
    LittleVector<Colour> updown;

    ColourPair() {
        this->colour = Colour(0, 0, 0);
        this->complement = Colour(255, 255, 255);
    }

    ColourPair(Colour colour, Colour complement) {
        this->colour = colour;
        this->complement = complement;
    }

    void initialise(int stripLength) {
        this->up.reserve(stripLength);
        this->down.reserve(stripLength);
        this->updown.reserve(stripLength);

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
};



class Section {
    public:
    // colour stuff
    ColourPair pair_a, pair_b, pair_c;

    // LED stuff
    Adafruit_NeoPixel strip;
    int stripLength;

    // animation helpers
    int modeSelection = 0;
    int animationStep = 0;
    int animationMaxStep = 100;

    Section(Colour a1, Colour a2, Colour b1, Colour b2, Colour c1, Colour c2, int stripLength, Adafruit_NeoPixel strip) {
        this->pair_a = ColourPair(a1, a2);
        this->pair_b = ColourPair(b1, b2);
        this->pair_c = ColourPair(c1, c2);
        
        this->stripLength = stripLength;
        this->strip = strip;
    }

    void initialise() {
        Serial.println("initialising...");

        pair_a.initialise(stripLength);
        pair_b.initialise(stripLength);
        pair_c.initialise(stripLength);

        // initialise strip
        strip.begin();
        strip.setBrightness(200); // TODO: debug setting --> change/ remove
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
            doAnimateSolid(pair_a, false);
            break;
        case 1:
            doAnimateSolid(pair_a, true);
            break;
        case 2:
            doAnimateGradientSlide(pair_a);
            break;
        case 3:
            doAnimateAlternating(pair_a);
            break;
        case 4:
            doAnimateGradientSlide(pair_b);
            break;
        case 5:
            doAnimateAlternating(pair_b);
            break;
        case 6:
            doAnimateGradientSlide(pair_c);
            break;
        case 7:
            doAnimateAlternating(pair_c);
            break;
        // case 8:
        //     doAnimateRandomALl();
        //     break;
        // case 9:
        //     doAnimateRandomSingle();
        //     break;
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

    void doAnimateSolid(ColourPair active_pair, bool reverse) {
        strip.clear();

        Colour local;
        if (reverse) {
            Serial.println("\tsolid reverse");
            local = active_pair.complement;
        } else {
            Serial.println("\tsolid normal");
            local = active_pair.colour;
        }

        for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor(i, local.r, local.g, local.b);
        }
        strip.show();
    }

    void doAnimateAlternating(ColourPair active_pair) {
        // TODO: anything but blocksize = 1 does not work, debug later
        Serial.println("\talternating");
        strip.clear();

        int blocksize = 5;

        if (! (animationStep % 10 == 0)) {
            return;
        }
        
        int localAnimationStep = animationStep / 10;

        // two lines in python (: 
        Colour a, b;
        if (localAnimationStep % 2 == 0) {
            a = active_pair.colour;
            b = active_pair.complement;
        } else {
            a = active_pair.complement;
            b = active_pair.colour;
        }
        for (int i = 0; i < stripLength; i+=(blocksize * 2)) {
            for (int j = 0; j < blocksize; j++) {
                strip.setPixelColor(i + j, a.r, a.g, a.b);
                strip.setPixelColor(i + j + blocksize, b.r, b.g, b.b);
            }
        }
        
        strip.show();
    }

    void doAnimateGradientSlide(ColourPair active_pair) {
        Serial.println("\t gradient slide");
        strip.clear();

        float animationPercent = (float) animationStep / animationMaxStep;
        int offset = (int) (animationPercent * stripLength);

        for (int i = 0; i < stripLength; i++) {
            int colourIndex = (i + offset) % stripLength;
            char strBuf2[100];
            int r, g, b;
            r = active_pair.updown[colourIndex].r;
            g = active_pair.updown[colourIndex].g;
            b = active_pair.updown[colourIndex].b;
            strip.setPixelColor(i, r, g, b);
        }

        strip.show();
    }

    void doAnimateRandomALl() {
        Serial.println("\t all random");
        // if (! (animationStep % 10 == 0)) {
        //     return;
        // }
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
        Serial.println("\t single random");
        // if (! (animationStep % 10 == 0)) {
        //     return;
        // }
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
Colour NEON_RED = Colour(255, 0, 0);
Colour NEON_WHITE = Colour(255, 255, 255);

Colour NEON_BLUE = Colour(77, 77, 255);
Colour NEON_PURPLE = Colour(199, 36, 177);

Colour NEON_GREEN = Colour(68, 214, 44);
Colour NEON_ORANGE = Colour(255, 173, 0);

// setup for strips and sections
int krummePin = A0;
int krummeLength = 26;
Adafruit_NeoPixel krummeStrip = Adafruit_NeoPixel(krummeLength, krummePin, NEO_GRB + NEO_KHZ800);
Section krumme = Section(NEON_RED, NEON_WHITE, NEON_BLUE, NEON_PURPLE, NEON_GREEN, NEON_ORANGE, krummeLength, krummeStrip);

int gemeindePin = A3;
int gemeindeLength = 17;
Adafruit_NeoPixel gemeindeStrip = Adafruit_NeoPixel(gemeindeLength, gemeindePin, NEO_GRB + NEO_KHZ800);
Section gemeinde = Section(NEON_WHITE, NEON_RED, NEON_PURPLE, NEON_BLUE, NEON_ORANGE, NEON_GREEN, gemeindeLength, gemeindeStrip);

int borderPin = A6;
int borderLength = 109;
Adafruit_NeoPixel borderStrip = Adafruit_NeoPixel(borderLength, borderPin, NEO_GRB + NEO_KHZ800);
Section border = Section(NEON_RED, NEON_WHITE, NEON_BLUE, NEON_PURPLE, NEON_GREEN, NEON_ORANGE, borderLength, borderStrip);

// setup for buttons and dials
const byte innerButtonPin = 1;
const byte borderButtonPin = 5;
const int animationDelay = 20;


void setup() {
    Serial.begin(9600);
    Serial.println("setup start");

    krumme.initialise();
    gemeinde.initialise();
    border.initialise();

    // set up buttons
    // TODO: use this instead of interrupt to avoid hardware bounce
    // https://github.com/thomasfredericks/Bounce2
    pinMode(innerButtonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(innerButtonPin), innerButtonInterrupt, RISING); // trigger interrupt when button is released
    
    pinMode(borderButtonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(borderButtonPin), borderButtonInterrupt, RISING);

    Serial.println("setup end");
}

void loop() {
    krumme.animate();
    gemeinde.animate();
    border.animate();
    
    
    delay(animationDelay);
}

void innerButtonInterrupt() {
    krumme.nextAnimation();
    gemeinde.nextAnimation();

    delay(25);
}

void borderButtonInterrupt() {
    border.nextAnimation();

    delay(25);
}

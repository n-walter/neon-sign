#include <Adafruit_NeoPixel.h>
#include <stdlib.h>
#include <Arduino.h>

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

int stripPin1 = A4;
int stripPin2 = A5;
int stripPin3 = A6;
Colour c1, c2, c3;
int NUM_PIXELS = 20;

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(NUM_PIXELS, stripPin1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUM_PIXELS, stripPin2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(NUM_PIXELS, stripPin3, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel strips[3] = {strip1, strip2, strip3};

void setup() {
    Serial.begin(9600);
    delay(5000);
    Serial.println("setup start");

    // reset pixel strips. for each doesnt work for some reason
    for (int i = 0; i < 3; i++) {
        Serial.println("strip setup");
        strips[i].begin();
        strips[i].clear();
        strips[i].show();
    }
    Serial.println("setup end");
}

void loop() {
    Serial.println("loop");
    solidColours();
    delay(1000);
}



void solidColours() {
    for (int i = 0; i < 3; i++) {
        int r = rand() % 255;
        int g = rand() % 255;
        int b = rand() % 255;
        strips[i].clear();
        for (int j = 0; j < NUM_PIXELS; j++) {
            strips[i].setPixelColor(j, r, g, b);
        }    
        strips[i].show();
    }
}
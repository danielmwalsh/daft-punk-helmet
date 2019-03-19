//Daft Punk Helmet Code
//Daniel Walsh
//2018

//Teensy 3.2 with Audio Shield
//WS2812b Leds
//Uses WS2812Serial and FastLED libraries
//https://github.com/FastLED/FastLED/wiki/Pixel-reference#chsv

#include <Audio.h>
#include <Bounce.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>

const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;

//Create audio components
AudioInputI2S            audioInput;
AudioSynthWaveform       synthwave; //Synthesized waveform for testing
AudioAnalyzeFFT256       fft;
AudioOutputI2S           audioOutput;

//Connect to aux input or synthesized wave
AudioConnection          patchCord1(audioInput, 1, fft, 0);
//AudioConnection          patchCord1(synthwave, 0, fft, 0);
AudioControlSGTL5000     audioShield;

//PINS
#define LED_PIN 1
#define POT A2

//LEDS
#define NUM_LEDS 8
#define NUM_SEGMENTS 8
CRGB leds[NUM_LEDS * NUM_SEGMENTS * 2];
int stripL[NUM_LEDS][NUM_SEGMENTS];
int stripR[NUM_LEDS][NUM_SEGMENTS];

// https://htmlcolorcodes.com/
uint32_t colors[] = {0x4B004B,   //purple
                     0x0000FF,   //dark blue
                     0x2DAFFF,   //light blue
                     0x004000,   //dark green
                     0x00FF00,   //light green
                     0xFFFF00,   //yellow
                     0xFF6400,   //orange
                     0xDD0000    //red
                    };

//SWITCHES
//https://www.pjrc.com/teensy/td_libs_Bounce.html
Bounce switch1 = Bounce(5, 5); //Switch for cycling LED modes
Bounce switch2 = Bounce(8, 5); //Switch for playing audio

void setup() {

  //Setup Pins
  pinMode(5, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);

  AudioMemory(10);

  // Enable the audio shield and set the output volume.
  audioShield.enable();
  audioShield.inputSelect(myInput);
  audioShield.volume(0.5);

  //synthwave.begin(1, 1000, WAVEFORM_SINE); // Create synthetic wave for testing purposes

  //Setup Serial for the WS2812b
  Serial.begin(57600);
  delay(100);

  //Setup LEDs
  LEDS.addLeds<WS2812SERIAL, LED_PIN, BRG>(leds, NUM_LEDS * NUM_SEGMENTS * 2);
  LEDS.setBrightness(64);
  FastLED.show();
  //LEDS.setMaxRefreshRate(120);

  //Setup LED matrix
  Serial.println("LED Array: ");
  int i, j, k = 0;
  for (j = 0; j < NUM_SEGMENTS; j++) {
    if (j % 2 == 0) {
      for (i = 0; i < NUM_LEDS; i++) {
        stripR[i][j] = i + k;
        Serial.print(i + k);
        Serial.print(" ");
      }
    }
    else {
      for (i = 0; i < NUM_LEDS; i++) {
        stripR[i][j] = k + NUM_LEDS - 1 - i;
        Serial.print(k + NUM_LEDS - 1 - i);
        Serial.print(" ");
      }
    }
    k += i;
    Serial.println();
  }
  for (j = 0; j < NUM_SEGMENTS; j++) {
    if (j % 2 == 0) {
      for (i = 0; i < NUM_LEDS; i++) {
        stripL[i][j] = i + k;
        Serial.print(i + k);
        Serial.print(" ");
      }
    }
    else {
      for (i = 0; i < NUM_LEDS; i++) {
        stripL[i][j] = k + NUM_LEDS - 1 - i;
        Serial.print(k + NUM_LEDS - 1 - i);
        Serial.print(" ");
      }
    }
    k += i;
    Serial.println();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {
  animateLEDs();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void audioVis() {

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  //Improvements:
  // - Don't start unless audio is playing
  // - Smoother pulses
  // - Light intensity peak
  // - Don't address unlit segments
  // - Compensate for variable line-in voltage (volume)
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////

  int intensity, scale, binrange = 16;
  if (fft.available()) {
    scale = map(analogRead(POT), 1, 1023, 0, 40); // Input from the potentiometer is used as a scale factor for the intensity
    //Serial.println();
    //Serial.print("scale: "); Serial.println(scale);
    for (int s = 0; s < NUM_SEGMENTS; s++) {
      intensity = scale * fft.read(binrange * s / NUM_SEGMENTS, binrange * (s + 1) / NUM_SEGMENTS);
      intensity = intensity < 0.0 ? 0.0 : intensity;
      intensity = intensity > 8.0 ? 8.0 : intensity;
      //Serial.print("intensity "); Serial.print(s); Serial.print(" = "); Serial.println(intensity);
      for (int i = 0 ; i < NUM_LEDS ; i++) {
        if (i < intensity) {
          //leds[stripL[i][s]] = CRGB(255, 0, 0);
          //leds[stripR[i][s]] = CRGB(255, 0, 0);
          leds[stripL[i][NUM_SEGMENTS - 1 - s]] = CRGB(255, 0, 0);
          leds[stripR[i][NUM_SEGMENTS - 1 - s]] = CRGB(255, 0, 0);
        } else {
          leds[stripL[i][s]] = CRGB(0, 0, 0);
          leds[stripR[i][s]] = CRGB(0, 0, 0);
        }
      }
    }
    FastLED.show();
  }
}

void allOff() { //Turns off all LEDS
  for (int i = 0; i < NUM_LEDS * NUM_SEGMENTS * 2; i++ ) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void allOn() { //Turns on all LEDS (the same color)
  for (int i = 0; i < NUM_LEDS * NUM_SEGMENTS * 2; i++ ) {
    leds[i] = CRGB(255, 0, 0);
  }
  FastLED.show();
}

void allRainbow() { //Turns on all LEDS (each segment is a different color)
  for (int j = 0; j < NUM_SEGMENTS; j++) {
    for (int i = 0; i < NUM_LEDS; i++ ) {
      leds[stripL[i][j]] = colors[j];
      leds[stripR[i][j]] = colors[j];
    }
  }
  FastLED.show();
}

void flash(int x, int t, uint32_t color) {
  for (int i = 0; i < x; i++) {
    // Turn all LEDS on
    for (int i = 0; i < NUM_LEDS * NUM_SEGMENTS * 2; i++) {
      leds[i] = color;
    }
    FastLED.show();
    delay(t);
    // Turn all LEDS off
    for (int i = 0; i < NUM_LEDS * NUM_SEGMENTS * 2; i++) {
      leds[i] = CRGB{0, 0, 0};
    }
    FastLED.show();
    delay(t);
  }
}

void stackUp() {
  for (int i = 0; i < NUM_LEDS * NUM_SEGMENTS * 2; i++ ) {
    leds[i] = CRGB(0, 0, 0);
  }
  int stackHeight, activeSegment;
  for (stackHeight = NUM_SEGMENTS; stackHeight >= 0; stackHeight--) {
    for (activeSegment = 0; activeSegment < stackHeight; activeSegment++) {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[stripL[i][activeSegment]] = colors[activeSegment];
        leds[stripR[i][activeSegment]] = colors[activeSegment];
      }
      if (activeSegment - 1 >= 0) {
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[stripL[i][activeSegment - 1]] = CRGB{0, 0, 0};
          leds[stripR[i][activeSegment - 1]] = CRGB{0, 0, 0};
        }
      }
      FastLED.show();
      delay(100);
    }
  }
}

void dropDown() {
  for (int j = 0; j < NUM_SEGMENTS; j++) {
    for (int i = 0; i < NUM_LEDS; i++ ) {
      leds[stripL[i][j]] = colors[j];
      leds[stripR[i][j]] = colors[j];
    }
  }
  int stackHeight, activeSegment;
  for (stackHeight = NUM_SEGMENTS - 1; stackHeight >= -1; stackHeight--) { // Iterate until every segment drops from the stack
    for (activeSegment = stackHeight + 1; activeSegment <= NUM_SEGMENTS; activeSegment++) { // Move the active segment away from the stack
      if (activeSegment < NUM_SEGMENTS) { // Only turn on segment if it is less than NUM_SEGMENTS
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[stripL[i][activeSegment]] = colors[activeSegment]; //Turn on the active pixel
          leds[stripR[i][activeSegment]] = colors[activeSegment]; //Turn on the active pixel
        }
      }
      if (activeSegment - 1 > stackHeight) { // Turn off segments above the active pixel and below the stack
        for (int i = 0; i < NUM_LEDS; i++) {
          leds[stripL[i][activeSegment - 1]] = CRGB{0, 0, 0};
          leds[stripR[i][activeSegment - 1]] = CRGB{0, 0, 0};
        }
      }
      FastLED.show();
      delay(100);
    }
  }
}

void wink(int x, int t, uint32_t color) {
  for (int i = 0; i < x; i++) {
    for (int k = 0; k < NUM_SEGMENTS / 2; k++) {
      for (int j = 0; j < NUM_SEGMENTS; j++) {
        if (j == k || j == (NUM_SEGMENTS - 1) - k) {
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[stripL[i][j]] = color;
            leds[stripR[i][j]] = color;
          }
        } else {
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[stripL[i][j]] = CRGB{0, 0, 0};
            leds[stripR[i][j]] = CRGB{0, 0, 0};
          }
        }
      }
      FastLED.show();
      delay(t);
    }
    for (int k = (NUM_SEGMENTS / 2) - 1; k >= 0 ; k--) {
      for (int j = 0; j < NUM_SEGMENTS; j++) {
        if (j == k || j == (NUM_SEGMENTS - 1) - k) {
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[stripL[i][j]] = color;
            leds[stripR[i][j]] = color;
          }
        } else {
          for (int i = 0; i < NUM_LEDS; i++) {
            leds[stripL[i][j]] = CRGB{0, 0, 0};
            leds[stripR[i][j]] = CRGB{0, 0, 0};
          }
        }
      }
      FastLED.show();
      delay(t);
    }
  }
}


void horizontalWipe(int x, int t, uint32_t color) {
  for (int i = 0; i < x; i++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      for (int j = 0; j < NUM_SEGMENTS; j++ ) {
        leds[stripL[i][j]] = color;
        leds[stripR[i][j]] = color;
      }
      FastLED.show();
      delay(t);
    }
    for (int i = NUM_LEDS - 1; i >= 0; i--) {
      for (int j = 0; j < NUM_SEGMENTS; j++ ) {
        leds[stripL[i][j]] = CRGB{0, 0, 0};
        leds[stripR[i][j]] = CRGB{0, 0, 0};
      }
      FastLED.show();
      delay(t);
    }
  }
}

void verticalWipe(int x, int t, uint32_t color) {
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < NUM_SEGMENTS; j++) {
      for (int i = 0; i < NUM_LEDS; i++ ) {
        leds[stripL[i][j]] = color;
        leds[stripR[i][j]] = color;
      }
      FastLED.show();
      delay(t);
    }
    delay(100); // Pause before reversing
    for (int j = NUM_SEGMENTS - 1; j >= 0; j--) {
      for (int i = 0; i < NUM_LEDS; i++ ) {
        leds[stripL[i][j]] = CRGB{0, 0, 0};
        leds[stripR[i][j]] = CRGB{0, 0, 0};
      }
      FastLED.show();
      delay(t);
    }
  }
}

void trickleDown(int x, int t, uint32_t color) {
  for (int i = 0; i < x; i++) {
    for (int j = -1; j < NUM_SEGMENTS + 1; j++) {
      if (j - 1 >= 0) {
        for (int i = 0; i < NUM_LEDS; i++ ) {
          leds[stripL[i][j - 1]] = CRGB(0, 0, 0);
          leds[stripR[i][j - 1]] = CRGB(0, 0, 0);
        }
      }
      FastLED.show();
      delay(t);
      if (j + 1 < NUM_SEGMENTS) {
        for (int i = 0; i < NUM_LEDS; i++ ) {
          leds[stripL[i][j + 1]] = color;
          leds[stripR[i][j + 1]] = color;
        }
      }
      FastLED.show();
      delay(t);
    }
  }
}

void checkers(int x, int t, uint32_t color) {
  for (int i = 0; i < x; i++) { // Repeat x number of times
    for (int j = 0; j < NUM_SEGMENTS; j++) {
      if (j < NUM_SEGMENTS / 2) {
        for (int i = 0; i < NUM_LEDS; i++ ) {
          leds[stripL[i][j]] = color;
          leds[stripR[i][j]] = CRGB(0, 0, 0);
        }
      } else {
        for (int i = 0; i < NUM_LEDS; i++ ) {
          leds[stripL[i][j]] = CRGB(0, 0, 0);
          leds[stripR[i][j]] = color;
        }
      }
    }
    FastLED.show();
    delay(t);
    for (int j = 0; j < NUM_SEGMENTS; j++) {
      if (j < NUM_SEGMENTS / 2) {
        for (int i = 0; i < NUM_LEDS; i++ ) {
          leds[stripL[i][j]] = CRGB(0, 0, 0);
          leds[stripR[i][j]] = color;
        }
      } else {
        for (int i = 0; i < NUM_LEDS; i++ ) {
          leds[stripL[i][j]] = color;
          leds[stripR[i][j]] = CRGB(0, 0, 0);
        }
      }
    }
    FastLED.show();
    delay(t);
  }
}

void animateLEDs() {
  stackUp();
  dropDown();
  wink(5, 50, CRGB::Green);
  horizontalWipe(5, 50, CRGB::Blue);
  verticalWipe(5, 50, CRGB::Blue);
  flash(5, 150, CRGB::Red);
  trickleDown(5, 100, CRGB::Blue);
  checkers(5, 200, CRGB::Yellow);
}

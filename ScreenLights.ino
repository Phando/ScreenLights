#include <FastLED.h>
#include <Button2.h>
#include <ESPRotary.h>

// Screen Lights:
// A simple program to control a strand of WS2811 LEDS with an ESP8266 micro controller.
//
// The LED control is based on the FastLED "100-lines-of-code" demo reel example.
// https://github.com/FastLED/FastLED
//
// The Encoder functionality is based on the ESPRotary "SimpleCounterWithButton" example.
// https://github.com/LennartHennigs/ESPRotary
// https://github.com/LennartHennigs/Button2
//
// Using the encoder you can change the patters and the brightness. 
//
// -Joe Andolina, Nov 2018


FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define ROTARY_PIN1 13
#define ROTARY_PIN2 12
#define BUTTON_PIN  14
#define DATA_PIN    4
#define LED_TYPE    WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS    36
CRGB leds[NUM_LEDS];

#define FRAMES_PER_SECOND  120

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { white, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t brightness = 150;
uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2);
Button2 b = Button2(BUTTON_PIN);

void setup() {
  Serial.begin(115200);
  delay(2000); // 2 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  
  // set master brightness control
  FastLED.setBrightness(brightness);

  r.setLeftRotationHandler(handleSpin);
  r.setRightRotationHandler(handleSpin);
  b.setClickHandler(handleClick);
  Serial.println("Ready!");
}

//LOW CHANGE RISING FALLING HIGH
  
void loop()
{
  // Exercise the rotarty and button libraries
  r.loop();
  b.loop();
  
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void handleSpin(ESPRotary& r) {
  if( r.getDirection() == 1 ) {
    brightness = min(255, brightness-2);
  } else {
    brightness = max(1, brightness+2);
  }
  FastLED.setBrightness(brightness);
  Serial.println(brightness);
}

void handleClick(Button2& btn) {
  nextPattern();
  Serial.println(gCurrentPatternNumber);
}

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void white() 
{
  // FastLED's built-in rainbow generator
  fill_solid( leds, NUM_LEDS, CRGB::White);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void off() {
  fadeToBlackBy( leds, NUM_LEDS, 20);
}

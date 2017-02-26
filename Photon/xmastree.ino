#include "FastLED/FastLED.h"
FASTLED_USING_NAMESPACE;

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN D5
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 124
CRGB leds[NUM_LEDS];

#define BRIGHTNESS 100
#define FRAMES_PER_SECOND 120
#define MAX_ARGS 64

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))


int lightsOn(String input);
int lightsOff(String input);
int toggleLights(String args);
int setBright(String brightness);
int twitterMention(String args);
void addGlitter(NSFastLED::fract8 chanceOfGlitter);

// list of patterns to cycle through.  each is defined as a separate function
// below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm};

uint8_t gCurrentPatternNumber = 0;  // Index number of which pattern is current
uint8_t gHue = 0;  // rotating "base color" used by many of the patterns


void setup() {
  // Publishing particle functions and vars.
  Particle.function("lightsOn", lightsOn);
  Particle.function("lightsOff", lightsOff);
  Particle.function("toggleLights", toggleLights);
  Particle.function("brightness", setBright);
  Particle.function("tweet", tweet);
  Particle.function("twitMention", twitterMention);
  delay(3000);  // 3 second delay for recovery

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
}


bool mentioned = false;
bool lightState = false;
bool currentState = false;
int lightBrightness = 100;


String onMessages[] = {"Lighting it up for Christmas!", 
                       "Let's get LIT!",
                       "HO HO HO!, It's Christmas Time!",
                       "Christmas Tree, OH Christmas ME!",
                       "My lights are brightly shining!"};

String offMessages[] = {"No more xmas.. FOR NOW!?",
                        "I'm going to sleep. ZZzz...ZZzzz..", 
                        "O-Holy-Night!",
                        "Going away. I'll be home for Christmas"};


void loop() {
  // main loop
  if (currentState != lightState) {
    lightState = currentState;
  }

  if (lightState == true) {
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / FRAMES_PER_SECOND);

    // do some periodic updates
    // slowly cycle the "base color" through the rainbow
    EVERY_N_MILLISECONDS(20) {
        gHue++;
        if (mentioned) {
            // Blink the tree lights 5 times if mentioned on twitter
            for (int i=0; i<5; i++) {
                FastLED.clear();
                FastLED.show();
                delay(250);
                rainbow();
                FastLED.show();
                delay(250);
            }
            mentioned = false;
        }
    }  
    
    // change patterns periodically
    EVERY_N_SECONDS(10) { 
        nextPattern(); 
    }  
  }
}

// Particle functions. 
// Per the docs, hese functions HAVE to return an int 
// and accept a String as a parameter. 
int tweet(String msg) { 
    Particle.publish("sendTweet", msg, 60, PRIVATE); 
    return 0;
}

int setBright(String brightness) {
  lightBrightness = brightness.toInt();
  FastLED.setBrightness(lightBrightness);
  return 0;
}

int toggleLights(String args) {
  int index = args.toInt();
  int value;
  char szArgs[MAX_ARGS];

  args.toCharArray(szArgs, MAX_ARGS);

  sscanf(szArgs, "%d=%d", &index, &value);
  
  if (value == 1) {
    lightsOn("");
  } else if (value == 0) {
    lightsOff("");
  }
  return 1;
}

int lightsOn(String input) {
  currentState = true;
  FastLED.show();

  int size = sizeof(onMessages) / sizeof(onMessages[0]);
  String msg = onMessages[random(0, size)];
  tweet(msg);

  return 0;
}

int lightsOff(String input) {
  currentState = false;
  FastLED.clear();
  FastLED.show();

  int size = sizeof(offMessages) / sizeof(offMessages[0]);
  String msg = offMessages[random(0, size)];
  tweet(msg);

  return 0;
}

int twitterMention(String args) {
    mentioned = true;
    String message = args + ", Thanks for the mention!";
    tweet(message);
    return 0;
}

// Light and pattern functions
void nextPattern() {
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
}

void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void addGlitter(NSFastLED::fract8 chanceOfGlitter) {
  if (random8() < chanceOfGlitter) {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

void rainbowWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void confetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) {  // 9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}


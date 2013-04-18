// Headband Program for fox ears with florapixels written by Bluebie 2013-4-14
#include <Adafruit_NeoPixel.h>
#include "utilities.h"

// ----------- available modes:
#define MODE_RANDOM_WALKER    0
#define MODE_WHITE            1
#define MODE_BOTH_LIGHT       2
#define MODE_EDGE_LIGHT       3
#define MODE_INNER_LIGHT      4
#define MODE_OPPOSITES        5
#define MODE_TWO_HUE_STROBE   6
#define MODE_REGULAR_STROBE   7
#define MODE_TOGGLE_STROBE    8
#define MODE_HEARTBEAT        9
#define MODE_WAVE            10
#define MODE_FOREST_WALK     11
#define MODE_OCEANIC         12
#define MODE_RAINBOW_CYCLE   13
#define MODE_SPIN_UP         14
// --------------------- total:
#define MODES                15
// -------------------- timing:
#define STROBE_INTERVAL      30 /* milliseconds between transition */
#define TOGGLE_INTERVAL      50
#define HUMAN_REACTION_TIME  20 /* takes 20ms to react from eye to finger */
#define COLOR_SELECT_WAIT    20 /* milliseconds between hue increments when holding button */
#define RANDOM_WALKER_WAIT   50
#define RAINBOW_CYCLE_DELAY  16

Adafruit_NeoPixel headband = Adafruit_NeoPixel(6, 3, NEO_RGB + NEO_KHZ400);
byte mode = MODE_HEARTBEAT;
byte hue = 85; // currently selected hue - default is red
RGBPixel hue_cache;

void setup() {
  digitalWrite(4, LOW);
  pinMode(4, OUTPUT);
  digitalWrite(3, LOW);
  pinMode(3, OUTPUT);
  
  pinMode(5, INPUT);
  digitalWrite(5, HIGH);
  
  headband.begin();
  
  hue_cache = color_wheel(hue);
}

void on_button() {
  unsigned long start_time = millis();
  
  for (int i = 255; i >= 0; i--) {
    set_all(RGB(i,0,0));
    headband.show();
  }
  
  while (digitalRead(5) == LOW) {
    if (millis() - start_time > 500) { // long hold
      hue += 1;
      hue_cache = color_wheel(hue);
      set_all(hue_cache);
      headband.show();
      delay(COLOR_SELECT_WAIT);
    }
  }
  
  set_all(BLACK);
  
  if (millis() - start_time < 500) {
    // it was a short press, so increment mode by one
    mode = (mode + 1) % MODES;
  } else {
    // it was a long hold
    // because human reaction times are slow, for colour selection should reverse hue back
    // by about 20ms (FINGER_REACTION_TIME) to give user the colour they intended
    hue -= HUMAN_REACTION_TIME / COLOR_SELECT_WAIT;
  }
  
  //srand(rand() + micros()); // reseed random number generator with user input
  delay(100);
}

void loop() {
  // detect when button is pressed down
  if (digitalRead(5) == LOW) on_button();
  
  // execute selected mode
  if (mode == MODE_HEARTBEAT) heartbeat(hue_cache);
  else if (mode == MODE_WHITE) set_all(RGB(255,255,255));
  else if (mode == MODE_BOTH_LIGHT) set_all(hue_cache);
  else if (mode == MODE_EDGE_LIGHT) set_edges(hue_cache);
  else if (mode == MODE_INNER_LIGHT) set_inner_ears(hue_cache);
  else if (mode == MODE_OPPOSITES) opposites(hue);
  else if (mode == MODE_TWO_HUE_STROBE) two_color_strobe(hue_cache, color_wheel(hue + 127));
  else if (mode == MODE_REGULAR_STROBE) strobe(hue_cache);
  else if (mode == MODE_TOGGLE_STROBE) toggle_strobe(hue_cache);
  else if (mode == MODE_WAVE) wave(hue_cache);
  else if (mode == MODE_RANDOM_WALKER) random_walker();
  else if (mode == MODE_FOREST_WALK) forest_walk();
  else if (mode == MODE_OCEANIC) oceanic();
  else if (mode == MODE_RAINBOW_CYCLE) rainbow_cycle();
  else if (mode == MODE_SPIN_UP) spin_up();
  
  // update headband
  headband.show();
}


// Bluebie's natural resting heartbeat duration is about about 1.1 seconds
#define HEARTBEAT_DURATION 1200 /* duration of entire animation loop */
#define HEARTBEAT_MICROS_INTERVAL ((HEARTBEAT_DURATION * 768UL) / 1000UL)
inline void heartbeat(RGBPixel color) {
  static unsigned int animation_frame; // a number between 0 and 767, looping
  static unsigned int last_micros;
  unsigned int current_micros = micros();
  
  // step forward heart's internal clock with system clock
  if (current_micros > last_micros + HEARTBEAT_MICROS_INTERVAL || current_micros < last_micros) {
    animation_frame++;
    animation_frame %= 768;
    last_micros = current_micros;
  }
  
  // run animation based on heart's internal clock
  if (animation_frame < 512) {
    set_inner_ears(multiply_colors(GRAY(255 - (animation_frame % 256)), color));
  } else {
    set_inner_ears(BLACK);
  }
}

void two_color_strobe(RGBPixel color_1, RGBPixel color_2) {
  set_all(get_two_color_strobe(color_1, color_2));
}

// set inner and outer to opposite colors
inline void opposites(byte hue) {
  set_inner_ears(color_wheel(hue));
  set_edges(color_wheel(hue + 127));
}

// strobe a single color
inline void strobe(RGBPixel color) {
  two_color_strobe(color, color);
}

// toggle light between edge and inner rapidly
inline void toggle_strobe(RGBPixel color) {
  boolean toggler = (millis() / TOGGLE_INTERVAL) % 2;
  set_inner_ears(toggler ? BLACK : color);
  set_edges(toggler ? color : BLACK);
}

// send a gentle wave of light downwards
#define WAVES_TOTAL_DURATION 1500
#define WAVE_DURATION 350
#define WAVE_OFFSET 210
inline void wave(RGBPixel color) {
  unsigned long start_time = millis();
  start_time -= start_time % WAVES_TOTAL_DURATION; // round down to last start
  
  wave_light(1, color, start_time, WAVE_DURATION);
  wave_light(2, color, start_time + WAVE_OFFSET, WAVE_DURATION);
  wave_light(0, color, start_time + (WAVE_OFFSET * 2), WAVE_DURATION);
  
  wave_light(4, color, start_time, WAVE_DURATION);
  wave_light(3, color, start_time + WAVE_OFFSET, WAVE_DURATION);
  wave_light(5, color, start_time + (WAVE_OFFSET * 2), WAVE_DURATION);
  
  headband.show();
}

// triangle wave pulse one pixel at a start time for a duration
void wave_light(byte pixel_id, RGBPixel color, unsigned long start_time, unsigned long duration) {
  long relative_time = millis() - start_time;
  relative_time *= 256;
  relative_time /= duration;
  
  byte intensity = 0;
  if (relative_time >= 0 && relative_time <= 511) {
    intensity = lookup_sine(relative_time / 2);
  }
  
  RGBPixel gray_color = GRAY(intensity); // to multiply with to control brightness
  headband.setPixelColor(pixel_id, multiply_colors(color, gray_color));
}

// set random hues to random lights 
void random_walker() {
  static unsigned int prev_time;
  unsigned int current_time = millis();
  if (prev_time + RANDOM_WALKER_WAIT < current_time || current_time < prev_time) {
    prev_time = current_time;
    
    byte target_pixel = rand() % headband.numPixels();
    RGBPixel color = color_wheel(rand());
    headband.setPixelColor(target_pixel, color);
  }
}

// calculate forest walk colors
inline RGBPixel forest_walk_equasion(unsigned int step) {
  unsigned int green = perlin(step) + 70;
  
  byte lightness = green / 4;
  lightness += green > 255 ? green - 255 : 0;
  green -= green > 255 ? green - 255 : 0;
  signed char color_wiggler = (lookup_sine((step % 1024) / 4) / 8) - 16;
  
  return RGB(
    constrain(lightness - color_wiggler, 0, 255),
    green,
    constrain(lightness + color_wiggler, 0, 255)
  );
}

// perlin random colours inspired by light falling through a forest canopy
void forest_walk() {
  for (byte i = 0; i < headband.numPixels(); i++) {
    RGBPixel color = forest_walk_equasion((millis() / 3) + (i * 20));
    headband.setPixelColor(i, color);
  }
}

// waving colors inspired by the ocean
inline void oceanic() {
  unsigned int step = millis() / 5;
  byte lightness = (lookup_sine((step % 512) / 2) + lookup_sine((step % 768) / 3)) / 4;
  byte blue = 192 + (lookup_sine((step % 256)) / 4);
  signed char color_wiggler = (lookup_sine((step % 1024) / 4) / 8) - 16;
  byte left = constrain(lightness - color_wiggler, 0, 255);
  byte right = constrain(lightness + color_wiggler, 0, 255);
  headband.setPixelColor(2, left, right, blue);
  headband.setPixelColor(3, right, left, blue);
}

// rotate through a rainbow, with opposite hue colours on edges
inline void rainbow_cycle() {
  byte rotation = (millis() / RAINBOW_CYCLE_DELAY);
  opposites(rotation);
}


inline void spin_up() {
  byte angle = millis();
  for (byte idx = 0; idx < 3; idx++) {
    RGBPixel color = color_wheel(-(angle + (idx * 64)));
    headband.setPixelColor(idx, color);
    headband.setPixelColor(5 - idx, color);
  }
}


// strobes between color_1, black, color_2, and black in that order
inline RGBPixel get_two_color_strobe(RGBPixel color_1, RGBPixel color_2) {
  byte idx = (millis() / STROBE_INTERVAL) % 4;
  if (idx != 0) color_1 = color_2;
  if (idx & 1) color_1 = BLACK;
  return color_1;
}

// set edge lights to a color - doesn't affect inner lights
inline void set_edges(RGBPixel color) {
  headband.setPixelColor(0, color);
  headband.setPixelColor(1, color);
  headband.setPixelColor(4, color);
  headband.setPixelColor(5, color);
}

// set inner lights to a color - doesn't affect edge lights
inline void set_inner_ears(RGBPixel color) {
  headband.setPixelColor(2, color);
  headband.setPixelColor(3, color);
}

// set every light to a color
inline void set_all(RGBPixel color) {
  set_inner_ears(color);
  set_edges(color);
}

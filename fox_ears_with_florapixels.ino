#include "Adafruit_FloraPixel.h"

// Headband Program for fox ears with florapixels written by Bluebie 2013-3-3
// TODO: Figure out why heartbeat bugs out after a while

#define MODE_HEARTBEAT      0
#define MODE_COLORSWEEP     1
#define MODE_HAIRMATCH      2
#define MODE_RAINBOWCYCLE   3
#define MODE_TRON_CLUE      4
#define MODE_TRON_HERO      5
#define MODE_TRON_HERO_DIM  6
#define MODE_TRON_GREEN     7
#define MODE_TRON_PURPLE    8
#define MODE_COLOR_CRAWL    9
#define MODE_PURE_BLUE     10
#define MODE_PURE_GREEN    11
#define MODE_PURE_RED      12
#define MODE_STROBE_RED    13
#define MODE_STROBE_GREEN  14
#define MODE_STROBE_BLUE   15
#define MODE_STROBE_PURPLE 16
#define MODE_PULSE_BLUE    17
#define MODE_PULSE_GREEN   18
#define MODE_PULSE_RED     19
#define MODES              20
byte mode = MODE_HEARTBEAT;

#define RGB(r,g,b) ((RGBPixel){(byte) (b), (byte) (r), (byte) (g)})
//#define RGB(r,g,b) RAW_RGB((((r)*(r)) / 256), (((g)*(g)) / 256), (((b)*(b)) / 256))
#define GRAY(intensity) RGB(intensity, intensity, intensity)
//#define GAMMA(rgb) ((RGBPixel) {(rgb.red * rgb.red) / 256, (rgb.green * rgb.green) / 256, (rgb.blue * rgb.blue) / 256})
//#define GAMMA(rgb) multiply_colors(rgb, rgb)
// fake gamma passthrough - took up too much progmem and complexity!
#define GAMMA(rgb) (rgb)

// Set the first variable to the NUMBER of pixels. 25 = 25 pixels in a row
Adafruit_FloraPixel headband = Adafruit_FloraPixel(6);

void setup() {
  digitalWrite(4, LOW);
  pinMode(4, OUTPUT);
  digitalWrite(3, LOW);
  pinMode(3, OUTPUT);
  
  pinMode(5, INPUT);
  digitalWrite(5, HIGH);
  
  headband.begin();
  
  // Update the headband, to start they are all 'off'
  headband.show();
}

void loop() {
  // Some example procedures showing how to display to the pixels
  if (digitalRead(5) == LOW) {
    mode = (mode + 1) % MODES;
    
    for (int i = 255; i >= 0; i--) {
      set_all_pixels(GAMMA(RGB(i,0,0)));
      headband.show();
    }
    
    while (digitalRead(5) == LOW) {;}
    delay(100);
  }
  
  if (mode == MODE_RAINBOWCYCLE)       iterate_rainbowcycle();
  else if (mode == MODE_COLORSWEEP)    iterate_colorsweep();
  else if (mode == MODE_HAIRMATCH)     iterate_hairmatch();
  // the tron ones are all pretty rubbish except purple
  else if (mode == MODE_TRON_CLUE)     iterate_tron_style(RGB(255,30,15)); // ???
  //else if (mode == MODE_TRON_CLUE_DIM) iterate_tron_style(RGB(228,87,39)); // crap
  else if (mode == MODE_TRON_HERO)     iterate_tron_style(RGB(255,255,255)); // awesome
  else if (mode == MODE_TRON_HERO_DIM) iterate_tron_style(RGB(60,255,255)); // cool!
  else if (mode == MODE_TRON_GREEN)    iterate_tron_style(RGB(40,255,30)); // pretty cool!
  else if (mode == MODE_TRON_PURPLE)   iterate_tron_style(RGB(255,60,255)); // awesome
  else if (mode == MODE_HEARTBEAT)     iterate_heartbeat();
  else if (mode == MODE_PURE_BLUE)     iterate_color(RGB(0,0,255));
  else if (mode == MODE_PURE_GREEN)    iterate_color(RGB(0,255,0));
  else if (mode == MODE_PURE_RED)      iterate_color(RGB(255,0,0));
  else if (mode == MODE_STROBE_RED)    iterate_strobe(RGB(255,0,0), 60);
  else if (mode == MODE_STROBE_GREEN)  iterate_strobe(RGB(0,255,0), 60);
  else if (mode == MODE_STROBE_BLUE)   iterate_strobe(RGB(0,0,255), 60);
  else if (mode == MODE_STROBE_PURPLE) iterate_strobe(RGB(180,0,255), 60);
  else if (mode == MODE_COLOR_CRAWL)   iterate_color_crawl();
  else if (mode == MODE_PULSE_BLUE)    iterate_pulse(RGB(0,0,255));
  else if (mode == MODE_PULSE_GREEN)   iterate_pulse(RGB(0,255,0));
  else if (mode == MODE_PULSE_RED)     iterate_pulse(RGB(255,0,0));
  
}

void set_all_pixels(RGBPixel c) {
  for (byte i = 0; i < headband.numPixels(); i++) {
    headband.setPixelColor(i, GAMMA(c));
  }
}

#define RAINBOWCYCLE_DELAY 16
void iterate_rainbowcycle() {
  byte rotation = (millis() / RAINBOWCYCLE_DELAY);
  
  set_edges(Wheel(rotation));
  set_inner_ears(Wheel(rotation + /*85*/ 127));
  
  headband.show();   // write all the pixels out
}

// cycle through a rainbow across the headset
#define COLORSWEEP_SPEED 75
#define COLORSWEEP_DURATION 500
void iterate_colorsweep() {
  RGBPixel colors[6] = {
    GAMMA(RGB(255, 0, 0)),
    GAMMA(RGB(255, 255, 0)),
    GAMMA(RGB(0, 255, 0)),
    GAMMA(RGB(0, 255, 255)),
    GAMMA(RGB(0, 0, 255)),
    GAMMA(RGB(255, 0, 255))
  };
  
  // calculate current color in sequence
  byte color_id = (millis() / COLORSWEEP_DURATION) % 6;
  RGBPixel color = colors[color_id];
  byte pixel_id = (millis() % COLORSWEEP_DURATION) / COLORSWEEP_SPEED;
  
  if (pixel_id < headband.numPixels()) {
    if (color_id & 1) { // alternate diretion for each color
      headband.setPixelColor(pixel_id, color);
    } else {
      headband.setPixelColor(headband.numPixels() - (pixel_id + 1), color);
    }
  }
  headband.show();
}

// alternate between two colours which match my hairdye
#define HAIRMATCH_SPEED 50
#define HAIRMATCH_COLORS 4
void iterate_hairmatch() {
  RGBPixel colors[HAIRMATCH_COLORS] = {
    RGB(0,0,0), GAMMA(RGB(65, 2, 254)),
    RGB(0,0,0), GAMMA(RGB(57, 233, 0))
  };
  
  RGBPixel color = colors[(millis() / HAIRMATCH_SPEED) % HAIRMATCH_COLORS];
  set_all_pixels(color);
  headband.show();
}

void iterate_tron_style(RGBPixel color) {
  set_edges(color);
  set_inner_ears(RGB(0,0,0));
  headband.show();
}

#define HEARTBEAT_DURATION 700 /* duration of entire animation loop */
#define HEARTBEAT_MICROS_INTERVAL (1000000L / HEARTBEAT_DURATION)
void iterate_heartbeat() {
  static int animation_frame; // a number between 0 and 767, looping
  static unsigned long last_micros;
  unsigned long current_micros = micros();
  
  // step forward heart's internal clock with system clock
  if (current_micros > last_micros + HEARTBEAT_MICROS_INTERVAL || current_micros < last_micros) {
    animation_frame++;
    animation_frame %= 768;
    last_micros = current_micros;
  }
  
  // run animation based on heart's internal clock
  if (animation_frame < 512) {
    set_inner_ears(GAMMA(RGB(255 - (animation_frame % 256), 0, 0)));
  } else {
    set_inner_ears(RGB(0,0,0));
  }
  
  headband.show();
}

void iterate_color(RGBPixel color) {
  set_edges(color);
  set_inner_ears(color);
  headband.show();
}

// strobe just one color
void iterate_strobe(RGBPixel color, int frequency) {
  boolean lit = (millis() % frequency) >= (frequency / 2);
  if (lit) {
    set_edges(GAMMA(color));
    set_inner_ears(GAMMA(color));
  } else {
    set_edges(RGB(0,0,0));
    set_inner_ears(RGB(0,0,0));
  }
  
  headband.show();
}

#define COLOR_CRAWL_DURATION 50
void iterate_color_crawl() {
  static unsigned long prev_time;
  unsigned long current_time = millis();
  if (prev_time + COLOR_CRAWL_DURATION < current_time || current_time < prev_time) {
    prev_time = current_time;
    
    byte target_pixel = rand() % headband.numPixels();
    RGBPixel color = Wheel(rand());
    headband.setPixelColor(target_pixel, color);
  }
  
  headband.show();
}

#define PULSES_DURATION 1500
#define PULSE_DURATION 350
#define PULSE_OFFSET 225
void iterate_pulse(RGBPixel color) {
  unsigned long start_time = millis();
  start_time -= start_time % PULSES_DURATION; // round down to last start
  
  pulse(1, color, start_time, PULSE_DURATION);
  pulse(2, color, start_time + PULSE_OFFSET, PULSE_DURATION);
  pulse(0, color, start_time + (PULSE_OFFSET * 2), PULSE_DURATION);
  
  pulse(4, color, start_time, PULSE_DURATION);
  pulse(3, color, start_time + PULSE_OFFSET, PULSE_DURATION);
  pulse(5, color, start_time + (PULSE_OFFSET * 2), PULSE_DURATION);
  
  headband.show();
}

// triangle wave pulse one pixel at a start time for a duration
void pulse(byte pixel_id, RGBPixel color, unsigned long start_time, unsigned long duration) {
  //int time = millis() - start_time;
  long time = millis() - start_time;
  time *= 256;
  time /= duration;
  
  byte intensity = 0;
  if (time > 0 && time < 256) {
    intensity = time;
  } else if (time >= 256 && time < 512) {
    intensity = 255 - (time - 256);
  }
  
  RGBPixel gray_color = GRAY(intensity); // to multiply with to control brightness
  headband.setPixelColor(pixel_id, multiply_colors(color, gray_color));
}

void set_edges(RGBPixel color) {
  headband.setPixelColor(0, color);
  headband.setPixelColor(1, color);
  headband.setPixelColor(4, color);
  headband.setPixelColor(5, color);
}

void set_inner_ears(RGBPixel color) {
  headband.setPixelColor(2, color);
  headband.setPixelColor(3, color);
}

// turn an incrementing number in to a triangle wave
//byte triangle(byte input) {
//  if (input < 128) {
//    return input * 2;
//  } else {
//    return 255 - ((input - 128) * 2);
//  }
//}

// multiply two colours together
RGBPixel multiply_colors(RGBPixel left, RGBPixel right) {
  left.red   = (left.red   * right.red)   / 256;
  left.green = (left.green * right.green) / 256;
  left.blue  = (left.blue  * right.blue)  / 256;
  return left;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
RGBPixel Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return RGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return RGB(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return RGB(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

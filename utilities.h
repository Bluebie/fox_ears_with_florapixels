#include <avr/pgmspace.h>

typedef uint32_t RGBPixel;
typedef struct _RGBPixelStruct {
  byte green;
  byte red;
  byte blue;
} RGBPixelStruct;

typedef union _RGBPixelUnion {
  uint32_t raw;
  RGBPixelStruct pixel;
} RGBPixelUnion;

RGBPixel RGB(byte r, byte g, byte b) {
  RGBPixelUnion result;
  result.pixel.red = r;
  result.pixel.green = g;
  result.pixel.blue = b;
  return result.raw;
}

//#define RGB(r,g,b) ((RGBPixel){(byte) (b), (byte) (r), (byte) (g)})
//#define RGB(r,g,b) ((uint32_t) (((uint32_t)g << 16) | ((uint32_t)r <<  8) | b))
#define GRAY(intensity) RGB(intensity, intensity, intensity)
#define BLACK RGB(0,0,0)
#define WHITE RGB(255,255,255)

#define mul8bit(a,b) ((((byte) a) * ((byte) b)) / 256)

// turn an incrementing number in to a triangle wave
//byte triangle(byte input) {
//  if (input < 128) {
//    return input * 2;
//  } else {
//    return 255 - ((input - 128) * 2);
//  }
//}

static byte sineWaveTable[256] PROGMEM = {
0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 16, 17, 19, 21, 23, 24, 26, 28, 30, 32, 35, 37, 39, 41, 44, 46, 48, 51, 54, 56, 59, 61, 64, 67, 70, 73, 75, 78, 81, 84, 87, 90, 93, 96, 99, 102, 106, 109, 112, 115, 118, 121, 124, 127, 128, 131, 134, 137, 140, 143, 146, 149, 153, 156, 159, 162, 165, 168, 171, 174, 177, 180, 182, 185, 188, 191, 194, 196, 199, 201, 204, 207, 209, 211, 214, 216, 218, 220, 223, 225, 227, 229, 231, 232, 234, 236, 238, 239, 241, 242, 243, 245, 246, 247, 248, 249, 250, 251, 252, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 247, 245, 244, 243, 241, 240, 238, 237, 235, 233, 232, 230, 228, 226, 224, 222, 219, 217, 215, 213, 210, 208, 205, 203, 200, 198, 195, 192, 189, 187, 184, 181, 178, 175, 172, 169, 166, 163, 160, 157, 154, 151, 148, 145, 142, 139, 135, 132, 129, 126, 123, 120, 116, 113, 110, 107, 104, 101, 98, 95, 92, 89, 86, 83, 80, 77, 74, 71, 68, 66, 63, 60, 57, 55, 52, 50, 47, 45, 42, 40, 38, 36, 33, 31, 29, 27, 25, 23, 22, 20, 18, 17, 15, 14, 12, 11, 10, 8, 7, 6, 5, 4, 4, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0
};

inline byte lookup_sine(byte pos) {
  return pgm_read_byte(&(sineWaveTable[pos]));
}

//inline byte lookup_sine(byte idx) {
//  return EEPROM.read(idx % 256);
//}

// multiply two colours together
inline RGBPixel multiply_colors(RGBPixel left_op, RGBPixel right_op) {
  RGBPixelUnion left;
  RGBPixelUnion right;
  left.raw = left_op;
  right.raw = right_op;
  
  left.pixel.red   = (left.pixel.red   * right.pixel.red)   / 256;
  left.pixel.green = (left.pixel.green * right.pixel.green) / 256;
  left.pixel.blue  = (left.pixel.blue  * right.pixel.blue)  / 256;
  return left.raw;
}

// which colourwheel algorithm should we use?
// sine_wheel uses a compramise between triangles and sine waves to give
// a really smooth wheel while still representing many colours and not focusing
// too heavily on reds, greens, and blues
// while triangle_wheel uses pure triangle waveforms to represent every colour
// equally, but can look jarring in the way it speeds up to reds, greens, and
// blues then abruptly without momentum backs off.
//#define color_wheel_mechanism(position) color_wheel_waveform(position)
#define color_wheel_mechanism(position) color_wheel_triangle(position)
//#define color_wheel_mechanism(position) color_wheel_compramise(position)

// a simple sine-wave response curve /~\_
inline byte color_wheel_sine(signed int position) {
  if (position >= 0 && position <= 255) {
    return lookup_sine(position);
  } else {
    return 0;
  }
}

// a harsh triangle-wave response curve /^\_
inline byte color_wheel_triangle(signed int position) {
  if (position >= 0 && position <= 127) {
    return position * 2;
  } else if (position >= 128 && position <= 255) {
    return 255 - ((position - 128) * 2);
  } else {
    return 0;
  }
}

// the average of sine and triangle waveforms
inline byte color_wheel_compramise(signed int position) {
  return (color_wheel_triangle(position) / 2)
       + (color_wheel_sine(position) / 2);
}

// takes in a number from 0 to 255, and returns a color representing that
// position on the hue wheel
inline RGBPixel color_wheel(byte position) {
  RGBPixelUnion result;
  result.raw = 0; // initialize to black
  
  // expand range by one third of 256 = 0-380
  signed int x = ((unsigned int) position) * 191 / 128;
  
  // lookup each of the values in our sine wave table
  result.pixel.red   = color_wheel_mechanism(x);
  result.pixel.green = color_wheel_mechanism(x - 127);
  result.pixel.blue  = color_wheel_mechanism((x + 127) % 380);
  
  // next, apply "gamma correction" to fit human vision so all brightnesses
  // look roughly equivilent around the wheel
//  result.pixel.red   = (result.pixel.red   * result.pixel.red)   / 256;
//  result.pixel.green = (result.pixel.green * result.pixel.green) / 256;
//  result.pixel.blue  = (result.pixel.blue  * result.pixel.blue)  / 256;
  return result.raw;
}

/* Space Plumber's random */
inline unsigned int sp_random(unsigned int seed) {
  seed = (seed * 58321) + 11113;
  return seed >> 8;
}

inline byte perlin(unsigned int num) {
  byte output = 0;
  for (byte idx = 0; idx < 8; idx++) output |= (sp_random(num / (1 << idx)) % 2) << idx;
  return output;
}


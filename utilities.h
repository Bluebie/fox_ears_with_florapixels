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

// turn an incrementing number in to a triangle wave
//byte triangle(byte input) {
//  if (input < 128) {
//    return input * 2;
//  } else {
//    return 255 - ((input - 128) * 2);
//  }
//}

inline byte lookup_sine(byte idx) {
  return EEPROM.read(idx);
}

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

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
inline RGBPixel Wheel(byte WheelPos) {
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

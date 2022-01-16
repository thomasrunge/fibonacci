#include <OneButton.h>
#include <avr/pgmspace.h>
#include "FastLED.h"
#include "scrolltext.h"


/*
 * Thomas Runge (tom@truwo.de), 2015/2016/2017
 * 
 * This is heavily based on work by Jason Coon
 * (http://evilgeniuslabs.org/)
 * found here: https://github.com/jasoncoon/fibonacci
 * 
 * and the great work of the FastLED team (http://fastled.io/)
 * 
 * idea of a Fibonacci display and the incrementalDrift
 * algorithm by Jim Bumgardner (http://jbum.com/)
 * 
 * noise-noise algo by Stefan Petrick:
 *   https://plus.google.com/communities/109127054924227823508
 */

#define SECONDS_PER_PATTERN 60

// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
#define SECONDS_PER_PALETTE 10


#define DATA_PIN    6
#define COLOR_ORDER GRB
#define LED_TYPE    WS2812
#define NUM_LEDS    100
#define BUTTON1_PIN A2
#define BUTTON2_PIN A3

#define NUM_VIRTUAL_LEDS NUM_LEDS+1
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
CRGB leds[NUM_VIRTUAL_LEDS];

// Params for width and height
const uint8_t kMatrixWidth = 10;
const uint8_t kMatrixHeight = 10;

Scrolltext scroller;
const char *scrolltext = "FastLED rulez!";

OneButton button1(BUTTON1_PIN, true);
OneButton button2(BUTTON2_PIN, true);

uint8_t incrementalDrift();
uint8_t colorWaves();
uint8_t noise();
uint8_t radialPaletteShift();
uint8_t spiral1();
uint8_t spiral2();
uint8_t spiral3();
uint8_t spiralPath();
uint8_t noise_noise();


typedef struct {
  uint8_t (*fnc)();
  const char *name;
  bool scroller;
} Pattern;
typedef Pattern PatternList[];

static const char pat01_name[] PROGMEM = "INCREMENTAL DRIFT";
static const char pat02_name[] PROGMEM = "COLOR WAVES";
static const char pat03_name[] PROGMEM = "NOISE";
static const char pat04_name[] PROGMEM = "RADIAL PALETTE SHIFT";
static const char pat05_name[] PROGMEM = "SPIRAL 1";
static const char pat06_name[] PROGMEM = "SPIRAL 2";
static const char pat07_name[] PROGMEM = "SPIRAL 3";
static const char pat08_name[] PROGMEM = "SPIRAL PATH";
static const char pat09_name[] PROGMEM = "NOISE NOISE";

const PatternList patternlist = {
  { incrementalDrift,   pat01_name, false },
  { colorWaves,         pat02_name, true  },
  { noise,              pat03_name, true  },
  { radialPaletteShift, pat04_name, true  },
  { spiral1,            pat05_name, true  },
  { spiral2,            pat06_name, true  },
  { spiral3,            pat07_name, true  },
  { spiralPath,         pat08_name, false },
  { noise_noise,        pat09_name, false }
};


uint8_t brightness = 128;
uint8_t currentpattern = 0;
boolean automaticmode;
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGBPalette16 IceColors_p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);

// List of palettes to cycle through.
CRGBPalette16 palettes[] = {
  RainbowColors_p,
  RainbowStripeColors_p,
  CloudColors_p,
  OceanColors_p,
  ForestColors_p,
  HeatColors_p,
  LavaColors_p,
  PartyColors_p,
  IceColors_p
};
const uint8_t paletteCount = ARRAY_SIZE(palettes);

extern const TProgmemRGBGradientPalettePtr gradientPalettes[];
extern const uint8_t gradientPaletteCount;

CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette = palettes[0];


void setup() {
  randomSeed(analogRead(0));
  random16_set_seed(1579);
  random16_add_entropy(analogRead(0));

  //Serial.begin(57600);

  button1.attachClick(cb_click1);
  button2.attachClick(cb_click2);
  button1.attachDoubleClick(cb_dclick1);
  button2.attachDoubleClick(cb_dclick2);
  button1.attachPress(cb_press1);
  button2.attachPress(cb_press2);
  automaticmode = true;

  scroller.setup(12, kMatrixWidth, kMatrixHeight);
  scroller.text(scrolltext, true);
  scroller.ptext(patternlist[currentpattern].name);

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  FastLED.setDither(false);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void loop() {
  Pattern pattern = patternlist[currentpattern];
  uint8_t dly = pattern.fnc();

  if (pattern.scroller) {
    scroller.draw(leds, setPixelXY, CRGB::White);
  }

  FastLED.delay(dly); 

  button1.tick();
  button2.tick();

  EVERY_N_MILLISECONDS(20) {
    gHue++;
  }

  // blend the current palette to the next
  EVERY_N_MILLISECONDS(40) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, 16);
  }

  // slowly change to a new palette
  EVERY_N_SECONDS(SECONDS_PER_PALETTE) {
    uint8_t paletteIndex = random8(paletteCount+gradientPaletteCount);
    if (paletteIndex < paletteCount) {
      targetPalette = palettes[paletteIndex];
    } else {
      targetPalette = gradientPalettes[paletteIndex-paletteCount];
    }
  }

  // move to next pattern
  if (automaticmode) {
    EVERY_N_SECONDS(SECONDS_PER_PATTERN) {
      currentpattern = addmod8(currentpattern, 1, ARRAY_SIZE(patternlist));
      scroller.ptext(patternlist[currentpattern].name);
    }
  }
}

void cb_click1() {
  if (automaticmode) {
    automaticmode = false;    
  } else {
    currentpattern = addmod8(currentpattern, 1, ARRAY_SIZE(patternlist));
    scroller.ptext(patternlist[currentpattern].name);
  }
}

void cb_click2() {
  if (automaticmode) {
    automaticmode = false;
  } else {
    currentpattern = addmod8(currentpattern+ARRAY_SIZE(patternlist), -1, ARRAY_SIZE(patternlist));
    scroller.ptext(patternlist[currentpattern].name);
  }
}

void cb_dclick1() {
  automaticmode = true;
}

void cb_dclick2() {
  automaticmode = true;
}

void cb_press1() {
  brightness = qadd8(brightness, 10);
  FastLED.setBrightness(brightness);
  FastLED.delay(100);
}

void cb_press2() {
  brightness = qsub8(brightness, 10);
  FastLED.setBrightness(brightness);
  FastLED.delay(100);
}

uint8_t incrementalDrift() {
  uint8_t stepwidth = 256*(20-1)/NUM_LEDS;
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    uint8_t bri = beatsin88(256+(NUM_LEDS-i-1)*stepwidth, 0, 255);
    leds[i] = ColorFromPalette(currentPalette, 2.5*i + gHue, bri, LINEARBLEND);
  }

  return 8;
}

uint8_t radialPaletteShift() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(currentPalette, 2.5*i + gHue, 255, LINEARBLEND);
  }
  return 8;
}


const uint8_t spiral1Count = 13;
const uint8_t spiral1Length = 7;
static const uint8_t spiral1Arms[spiral1Count][spiral1Length] PROGMEM = {
  { 0, 14, 27, 40, 53, 66, 84 },
  { 1, 15, 28, 41, 54, 67, 76 },
  { 2, 16, 29, 42, 55, 68, 85 },
  { 3, 17, 30, 43, 56, 77, 86 },
  { 4, 18, 31, 44, 57, 69, 78 },
  { 5, 19, 32, 45, 58, 70, 87 },
  { 6, 20, 33, 46, 59, 79, 94 },
  { 7, 21, 34, 47, 60, 71, 88 },
  { 8, 22, 35, 48, 61, 80, 89 },
  { 9, 23, 36, 49, 62, 72, 81 },
  { 10, 24, 37, 50, 63, 73, 82 },
  { 11, 25, 38, 51, 64, 74, 83 },
  { 12, 13, 26, 39, 52, 65, 75 }
};

const uint8_t spiral2Count = 21;
const uint8_t spiral2Length = 4;
static const uint8_t spiral2Arms[spiral2Count][spiral2Length] PROGMEM = {
  { 0, 26, 51, 73 },
  { 1, 14, 39, 64 },
  { 15, 27, 52, 74 },
  { 2, 28, 40, 65 },
  { 16, 41, 53, 75 },
  { 3, 29, 54, 66 },
  { 4, 17, 42, 67 },
  { 18, 30, 55, 76 },
  { 5, 31, 43, 68 },
  { 19, 44, 56, 85 },
  { 6, 32, 57, 77 },
  { 7, 20, 45, 69 },
  { 21, 33, 58, 78 },
  { 8, 34, 46, 70 },
  { 9, 22, 47, 59 },
  { 23, 35, 60, 79 },
  { 10, 36, 48, 71 },
  { 24, 49, 61, 88 },
  { 11, 37, 62, 80 },
  { 12, 25, 50, 72 },
  { 13, 38, 63, 81 }
};

const uint8_t spiral3Count = 8;
const uint8_t spiral3Length = 11;
static const uint8_t spiral3Arms[spiral3Count][spiral3Length] PROGMEM = {
  { 89, 81, 73, 64, 52, 40, 41, 29, 17, 18,  5 },
  { 90, 82, 74, 65, 53, 54, 42, 30, 31, 19,  6 },
  { 83, 75, 66, 67, 55, 43, 44, 32, 20, 21,  8 },
  { 84, 76, 68, 56, 57, 45, 33, 34, 22, 23, 10 },
  { 92, 85, 77, 69, 58, 46, 47, 35, 36, 24, 11 },
  { 86, 78, 70, 59, 60, 48, 49, 37, 25, 13,  0 },
  { 87, 79, 71, 61, 62, 50, 38, 26, 14, 15,  2 },
  { 94, 88, 80, 72, 63, 51, 39, 27, 28, 16,  3 }
};

template <size_t armCount, size_t armLength>
void fillSpiral(const uint8_t (&spiral)[armCount][armLength], bool reverse) {
  fill_solid(leds, NUM_LEDS, ColorFromPalette(currentPalette, 0, 255, LINEARBLEND));

  uint8_t offset = 255 / armCount;

  for(uint8_t i = 0; i < armCount; i++) {
      CRGB color;

      if(reverse) {
          color = ColorFromPalette(currentPalette, gHue+offset, 255, LINEARBLEND);
      } else {
          color = ColorFromPalette(currentPalette, 255-gHue-offset, 255, LINEARBLEND);
      }

      for(uint8_t j = 0; j < armLength; j++) {
          uint8_t offset = pgm_read_byte(&spiral[i][j]);
          leds[offset] = color;
      }

      if(reverse) {
          offset += 255 / armCount;
      } else {
          offset -= 255 / armCount;
      }
  }
}

uint8_t spiral1() {
  fillSpiral(spiral1Arms, false);
  
  return 0;
}

uint8_t spiral2() {
  fillSpiral(spiral2Arms, true);
  
  return 0;
}

uint8_t spiral3() {
  fillSpiral(spiral3Arms, true);
  
  return 0;
}

static const uint8_t spiralPath1Arms[] PROGMEM =  {
    11, 25, 38, 51, 64, 74, 83, 65, 40, 28, 2,
    16, 29, 42, 55, 68, 85, 56, 44, 19, 32, 45,
    58, 70, 87, 59, 47, 22, 9, 23, 36, 49, 62,
    72, 81, 63, 38, 13, 26, 39, 52, 65, 75, 91,
    66, 54, 29, 3, 17, 30, 43, 56, 77, 86, 69,
    45, 20, 7, 21, 34, 47, 60, 71, 88, 61, 49,
    24, 37, 50, 63, 73, 82, 64, 39, 14, 1, 15,
    28, 41, 54, 67, 76, 55, 30, 18, 31, 44, 57,
    69, 78, 93, 70, 46, 34, 8, 22, 35, 48, 61,
    80, 89, 90, 73, 51, 26, 0, 14, 27, 40, 53,
    66, 84, 97, 98, 77, 57, 32, 6, 20, 33, 46,
    59, 79, 94, 95, 80, 62, 37
};

const uint8_t spiralPath1Length = ARRAY_SIZE(spiralPath1Arms);

uint8_t spiralPath() {
  static uint8_t currentIndex = 0;

  fadeToBlackBy(leds, NUM_LEDS, 10);
  uint8_t offset = pgm_read_byte(&spiralPath1Arms[currentIndex]);
  leds[offset] = ColorFromPalette(currentPalette, gHue, 255, LINEARBLEND);
  currentIndex = addmod8(currentIndex, 1, spiralPath1Length);

  return 50;
}

static const uint8_t xyMap[kMatrixHeight][kMatrixWidth][2] PROGMEM = {
    { { 100, 100 }, { 100, 100 }, { 100, 100 }, { 21, 100 }, {  7,  20 }, {   6, 100 }, { 32, 100 }, {  19, 100 }, { 100, 100 }, { 100, 100 } },
    { { 100, 100 }, {   8, 100 }, {  34, 100 }, { 33,  46 }, { 58, 100 }, {  45, 100 }, { 57, 100 }, {  44, 100 }, {   5,  31 }, { 100, 100 } },
    { {   9, 100 }, {  22, 100 }, {  47, 100 }, { 59, 100 }, { 70, 100 }, {  69,  78 }, { 77, 100 }, {  56, 100 }, {  43, 100 }, {  18, 100 } },
    { { 100, 100 }, {  35, 100 }, {  60, 100 }, { 79, 100 }, { 87,  93 }, {  86, 100 }, { 85, 100 }, {  68, 100 }, { 100, 100 }, {  30,   4 } },
    { {  23, 100 }, {  48, 100 }, {  71, 100 }, { 94, 100 }, { 99, 100 }, {  98, 100 }, { 92, 100 }, {  76, 100 }, {  55, 100 }, {  17,  42 } },
    { {  10,  36 }, {  61, 100 }, {  80, 100 }, { 88, 100 }, { 95, 100 }, {  96,  97 }, { 91, 100 }, {  84, 100 }, {  67, 100 }, {  29,   3 } },
    { {  24, 100 }, {  49,  62 }, {  80, 100 }, { 89, 100 }, { 90, 100 }, {  83, 100 }, { 75, 100 }, {  66, 100 }, {  54, 100 }, {  29, 100 } },
    { { 100, 100 }, {  37, 100 }, {  50,  72 }, { 81, 100 }, { 73, 100 }, {  74,  82 }, { 65, 100 }, {  53, 100 }, {  41, 100 }, {  16, 100 } },
    { { 100, 100 }, {  11, 100 }, {  25, 100 }, { 63, 100 }, { 51, 100 }, {  64, 100 }, { 52, 100 }, {  40,  27 }, {  28, 100 }, {   2, 100 } },
    { { 100, 100 }, { 100, 100 }, {  12, 100 }, { 13,  38 }, { 26, 100 }, {   0,  39 }, { 14, 100 }, {   1,  27 }, {  15, 100 }, { 100, 100 } }
};

void setPixelXY(uint8_t x, uint8_t y, CRGB color) {
    if((x >= kMatrixWidth) || (y >= kMatrixHeight)) {
        return;
    }
    
    for(uint8_t z = 0; z < 2; z++) {
        uint8_t offset = pgm_read_byte(&xyMap[y][x][z]);
        leds[offset] = color;
    }
}

static const uint8_t coordsX[NUM_LEDS] PROGMEM = {
    135, 180, 236, 255, 253, 210, 143, 98, 37, 7,
    0, 32, 64, 93, 160, 200, 240, 243, 234, 182,
    116, 75, 28, 6, 15, 58, 120, 179, 214, 237,
    226, 210, 155, 95, 59, 27, 14, 36, 85, 142,
    191, 219, 227, 206, 186, 132, 81, 50, 32, 27,
    60, 109, 159, 195, 217, 212, 183, 162, 113, 74,
    50, 46, 46, 83, 128, 169, 191, 207, 193, 141,
    101, 57, 67, 104, 142, 169, 190, 161, 126, 76,
    64, 89, 122, 146, 176, 170, 142, 99, 73, 87,
    112, 155, 165, 119, 90, 100, 130, 137, 142, 116
};

static const uint8_t coordsY[NUM_LEDS] PROGMEM = {
    255, 250, 201, 132, 86, 28, 3, 0, 37, 73,
    146, 209, 242, 246, 242, 229, 174, 106, 64, 21,
    12, 18, 65, 102, 171, 220, 239, 224, 204, 145,
    86, 50, 22, 28, 39, 92, 128, 187, 224, 228,
    202, 179, 122, 72, 43, 30, 47, 63, 115, 151,
    198, 220, 212, 178, 154, 104, 66, 43, 44, 69,
    87, 135, 167, 201, 208, 191, 156, 133, 94, 52,
    62, 109, 175, 195, 191, 169, 120, 70, 68, 91,
    148, 174, 180, 166, 138, 93, 84, 85, 125, 151,
    160, 144, 113, 94, 112, 132, 148, 127, 107, 116
};

uint8_t noise() {
  static uint16_t noiseX = random16();
  static uint16_t noiseY = random16();
  static uint16_t noiseZ = random16();

  for(uint8_t i = 0; i < NUM_LEDS; i++) {
      uint8_t x = pgm_read_byte(&coordsX[i]);
      uint8_t y = pgm_read_byte(&coordsY[i]);

      uint8_t data = inoise8(x+x+noiseX, y+y+noiseY, noiseZ);

      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));

      leds[i] = ColorFromPalette(currentPalette, data, 255, LINEARBLEND);
  }

  noiseX += 0; // experiment with these values
  noiseY += 0;
  noiseZ += 1;

  return 0;
}

uint8_t noise_noise() {
  static const uint8_t CentreX =  (kMatrixWidth / 2) - 1;
  static const uint8_t CentreY = (kMatrixHeight / 2) - 1;

  static uint32_t x, y, z, scale_x, scale_y;
  static uint8_t noise[kMatrixWidth][kMatrixHeight];


  x = x + (2 * noise[0][0]) - 255;
  y = y + (2 * noise[kMatrixWidth-1][0]) - 255;
  z += 1 + ((noise[0][kMatrixHeight-1]) / 4);
  scale_x = 8000 + (noise[0][CentreY] * 24);
  scale_y = 8000 + (noise[kMatrixWidth-1][CentreY] * 24);

  for (uint8_t i = 0; i < kMatrixWidth; i++) {
    uint32_t ioffset = scale_x * (i - CentreX);
    for (uint8_t j = 0; j < kMatrixHeight; j++) {
      uint32_t joffset = scale_y * (j - CentreY);
      uint16_t data = inoise16(x + ioffset, y + joffset, z);
      if (data < 11000) data = 11000;
      if (data > 51000) data = 51000;
      data = data - 11000;
      data = data / 161;
      noise[i][j] = data;
    }
  }

  for (uint8_t y = 0; y < kMatrixHeight; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {
      CRGB overlay = CHSV(noise[y][x], 255, noise[x][y]);
      setPixelXY(x, y, ColorFromPalette((CRGBPalette16)gradientPalettes[0], noise[kMatrixWidth-1][kMatrixHeight-1] + noise[x][y]) + overlay);
    }
  }

  // cheap correction with gamma 2.0
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].r = dim8_video(leds[i].r);
    leds[i].g = dim8_video(leds[i].g);
    leds[i].b = dim8_video(leds[i].b);
  }

  return 0;
}

uint8_t colorWaves() {
  colorwaves(leds, NUM_LEDS, currentPalette);
  return 20;
}


// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void colorwaves(CRGB* ledarray, uint16_t numleds, CRGBPalette16& palette) {
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16; //gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88( 400, 5,9);
  uint16_t brightnesstheta16 = sPseudotime;

  for(uint16_t i = 0 ; i < numleds; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if(h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8(index);
    index = scale8(index, 240);

    CRGB newcolor = ColorFromPalette(palette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (numleds-1) - pixelnumber;

    nblend(ledarray[pixelnumber], newcolor, 128);
  }
}

// Gradient Color Palette definitions for different cpt-city color palettes.

DEFINE_GRADIENT_PALETTE( pit ) {
  0,     3,   3,   3,
  64,   13,   13, 255,  //blue
  128,   3,   3,   3,
  192, 255, 130,   3 ,  //orange
  255,   3,   3,   3
};

// Gradient palette "ib_jul01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ing/xmas/tn/ib_jul01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( ib_jul01_gp ) {
    0, 194,  1,  1,
   94,   1, 29, 18,
  132,  57,131, 28,
  255, 113,  1,  1};

// Gradient palette "es_vintage_57_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/vintage/tn/es_vintage_57.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_vintage_57_gp ) {
    0,   2,  1,  1,
   53,  18,  1,  0,
  104,  69, 29,  1,
  153, 167,135, 10,
  255,  46, 56,  4};

// Gradient palette "es_vintage_01_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/vintage/tn/es_vintage_01.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_vintage_01_gp ) {
    0,   4,  1,  1,
   51,  16,  0,  1,
   76,  97,104,  3,
  101, 255,131, 19,
  127,  67,  9,  4,
  153,  16,  0,  1,
  229,   4,  1,  1,
  255,   4,  1,  1};

// Gradient palette "es_rivendell_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/rivendell/tn/es_rivendell_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_rivendell_15_gp ) {
    0,   1, 14,  5,
  101,  16, 36, 14,
  165,  56, 68, 30,
  242, 150,156, 99,
  255, 150,156, 99};

// Gradient palette "rgi_15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/rgi/tn/rgi_15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE( rgi_15_gp ) {
    0,   4,  1, 31,
   31,  55,  1, 16,
   63, 197,  3,  7,
   95,  59,  2, 17,
  127,   6,  2, 34,
  159,  39,  6, 33,
  191, 112, 13, 32,
  223,  56,  9, 35,
  255,  22,  6, 38};

// Gradient palette "retro2_16_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/retro2/tn/retro2_16.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 8 bytes of program space.

DEFINE_GRADIENT_PALETTE( retro2_16_gp ) {
    0, 188,135,  1,
  255,  46,  7,  1};

// Gradient palette "Analogous_1_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/red/tn/Analogous_1.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( Analogous_1_gp ) {
    0,   3,  0,255,
   63,  23,  0,255,
  127,  67,  0,255,
  191, 142,  0, 45,
  255, 255,  0,  0};

// Gradient palette "es_pinksplash_08_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/pink_splash/tn/es_pinksplash_08.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_pinksplash_08_gp ) {
    0, 126, 11,255,
  127, 197,  1, 22,
  175, 210,157,172,
  221, 157,  3,112,
  255, 157,  3,112};

// Gradient palette "es_pinksplash_07_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/pink_splash/tn/es_pinksplash_07.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_pinksplash_07_gp ) {
    0, 229,  1,  1,
   61, 242,  4, 63,
  101, 255, 12,255,
  127, 249, 81,252,
  153, 255, 11,235,
  193, 244,  5, 68,
  255, 232,  1,  5};

// Gradient palette "Coral_reef_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/other/tn/Coral_reef.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( Coral_reef_gp ) {
    0,  40,199,197,
   50,  10,152,155,
   96,   1,111,120,
   96,  43,127,162,
  139,  10, 73,111,
  255,   1, 34, 71};

// Gradient palette "es_ocean_breeze_068_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_068.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_ocean_breeze_068_gp ) {
    0, 100,156,153,
   51,   1, 99,137,
  101,   1, 68, 84,
  104,  35,142,168,
  178,   0, 63,117,
  255,   1, 10, 10};

// Gradient palette "es_ocean_breeze_036_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/ocean_breeze/tn/es_ocean_breeze_036.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_ocean_breeze_036_gp ) {
    0,   1,  6,  7,
   89,   1, 99,111,
  153, 144,209,255,
  255,   0, 73, 82};

// Gradient palette "departure_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/mjf/tn/departure.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 88 bytes of program space.

DEFINE_GRADIENT_PALETTE( departure_gp ) {
    0,   8,  3,  0,
   42,  23,  7,  0,
   63,  75, 38,  6,
   84, 169, 99, 38,
  106, 213,169,119,
  116, 255,255,255,
  138, 135,255,138,
  148,  22,255, 24,
  170,   0,255,  0,
  191,   0,136,  0,
  212,   0, 55,  0,
  255,   0, 55,  0};

// Gradient palette "es_landscape_64_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_64.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 36 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_landscape_64_gp ) {
    0,   0,  0,  0,
   37,   2, 25,  1,
   76,  15,115,  5,
  127,  79,213,  1,
  128, 126,211, 47,
  130, 188,209,247,
  153, 144,182,205,
  204,  59,117,250,
  255,   1, 37,192};

// Gradient palette "es_landscape_33_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/landscape/tn/es_landscape_33.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_landscape_33_gp ) {
    0,   1,  5,  0,
   19,  32, 23,  1,
   38, 161, 55,  1,
   63, 229,144,  1,
   66,  39,142, 74,
  255,   1,  4,  1};

// Gradient palette "rainbowsherbet_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ma/icecream/tn/rainbowsherbet.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( rainbowsherbet_gp ) {
    0, 255, 33,  4,
   43, 255, 68, 25,
   86, 255,  7, 25,
  127, 255, 82,103,
  170, 255,255,242,
  209,  42,255, 22,
  255,  87,255, 65};

// Gradient palette "gr65_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr65_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( gr65_hult_gp ) {
    0, 247,176,247,
   48, 255,136,255,
   89, 220, 29,226,
  160,   7, 82,178,
  216,   1,124,109,
  255,   1,124,109};

// Gradient palette "gr64_hult_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/hult/tn/gr64_hult.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 32 bytes of program space.

DEFINE_GRADIENT_PALETTE( gr64_hult_gp ) {
    0,   1,124,109,
   66,   1, 93, 79,
  104,  52, 65,  1,
  130, 115,127,  1,
  150,  52, 65,  1,
  201,   1, 86, 72,
  239,   0, 55, 45,
  255,   0, 55, 45};

// Gradient palette "GMT_drywet_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/gmt/tn/GMT_drywet.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( GMT_drywet_gp ) {
    0,  47, 30,  2,
   42, 213,147, 24,
   84, 103,219, 52,
  127,   3,219,207,
  170,   1, 48,214,
  212,   1,  1,111,
  255,   1,  7, 33};

// Gradient palette "ib15_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ing/general/tn/ib15.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 24 bytes of program space.

DEFINE_GRADIENT_PALETTE( ib15_gp ) {
    0, 113, 91,147,
   72, 157, 88, 78,
   89, 208, 85, 33,
  107, 255, 29, 11,
  141, 137, 31, 39,
  255,  59, 33, 89};

// Gradient palette "Fuschia_7_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/ds/fuschia/tn/Fuschia-7.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( Fuschia_7_gp ) {
    0,  43,  3,153,
   63, 100,  4,103,
  127, 188,  5, 66,
  191, 161, 11,115,
  255, 135, 20,182};

// Gradient palette "es_emerald_dragon_08_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/emerald_dragon/tn/es_emerald_dragon_08.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 16 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_emerald_dragon_08_gp ) {
    0,  97,255,  1,
  101,  47,133,  1,
  178,  13, 43,  1,
  255,   2, 10,  1};

// Gradient palette "lava_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/lava.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

DEFINE_GRADIENT_PALETTE( lava_gp ) {
    0,   0,  0,  0,
   46,  18,  0,  0,
   96, 113,  0,  0,
  108, 142,  3,  1,
  119, 175, 17,  1,
  146, 213, 44,  2,
  174, 255, 82,  4,
  188, 255,115,  4,
  202, 255,156,  4,
  218, 255,203,  4,
  234, 255,255,  4,
  244, 255,255, 71,
  255, 255,255,255};

// Gradient palette "fire_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/neota/elem/tn/fire.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( fire_gp ) {
    0,   1,  1,  0,
   76,  32,  5,  0,
  146, 192, 24,  0,
  197, 220,105,  5,
  240, 252,255, 31,
  250, 252,255,111,
  255, 255,255,255};

// Gradient palette "Colorfull_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Colorfull.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

DEFINE_GRADIENT_PALETTE( Colorfull_gp ) {
    0,  10, 85,  5,
   25,  29,109, 18,
   60,  59,138, 42,
   93,  83, 99, 52,
  106, 110, 66, 64,
  109, 123, 49, 65,
  113, 139, 35, 66,
  116, 192,117, 98,
  124, 255,255,137,
  168, 100,180,155,
  255,  22,121,174};

// Gradient palette "Magenta_Evening_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Magenta_Evening.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( Magenta_Evening_gp ) {
    0,  71, 27, 39,
   31, 130, 11, 51,
   63, 213,  2, 64,
   70, 232,  1, 66,
   76, 252,  1, 69,
  108, 123,  2, 51,
  255,  46,  9, 35};

// Gradient palette "Pink_Purple_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Pink_Purple.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 44 bytes of program space.

DEFINE_GRADIENT_PALETTE( Pink_Purple_gp ) {
    0,  19,  2, 39,
   25,  26,  4, 45,
   51,  33,  6, 52,
   76,  68, 62,125,
  102, 118,187,240,
  109, 163,215,247,
  114, 217,244,255,
  122, 159,149,221,
  149, 113, 78,188,
  183, 128, 57,155,
  255, 146, 40,123};

// Gradient palette "Sunset_Real_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/atmospheric/tn/Sunset_Real.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( Sunset_Real_gp ) {
    0, 120,  0,  0,
   22, 179, 22,  0,
   51, 255,104,  0,
   85, 167, 22, 18,
  135, 100,  0,103,
  198,  16,  0,130,
  255,   0,  0,160};

// Gradient palette "es_autumn_19_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/es/autumn/tn/es_autumn_19.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 52 bytes of program space.

DEFINE_GRADIENT_PALETTE( es_autumn_19_gp ) {
    0,  26,  1,  1,
   51,  67,  4,  1,
   84, 118, 14,  1,
  104, 137,152, 52,
  112, 113, 65,  1,
  122, 133,149, 59,
  124, 137,152, 52,
  135, 113, 65,  1,
  142, 139,154, 46,
  163, 113, 13,  1,
  204,  55,  3,  1,
  249,  17,  1,  1,
  255,  17,  1,  1};

// Gradient palette "BlacK_Blue_Magenta_White_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Blue_Magenta_White.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( BlacK_Blue_Magenta_White_gp ) {
    0,   0,  0,  0,
   42,   0,  0, 45,
   84,   0,  0,255,
  127,  42,  0,255,
  170, 255,  0,255,
  212, 255, 55,255,
  255, 255,255,255};

// Gradient palette "BlacK_Magenta_Red_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Magenta_Red.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( BlacK_Magenta_Red_gp ) {
    0,   0,  0,  0,
   63,  42,  0, 45,
  127, 255,  0,255,
  191, 255,  0, 45,
  255, 255,  0,  0};

// Gradient palette "BlacK_Red_Magenta_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Red_Magenta_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 28 bytes of program space.

DEFINE_GRADIENT_PALETTE( BlacK_Red_Magenta_Yellow_gp ) {
    0,   0,  0,  0,
   42,  42,  0,  0,
   84, 255,  0,  0,
  127, 255,  0, 45,
  170, 255,  0,255,
  212, 255, 55, 45,
  255, 255,255,  0};

// Gradient palette "Blue_Cyan_Yellow_gp", originally from
// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/Blue_Cyan_Yellow.png.index.html
// converted for FastLED with gammas (2.6, 2.2, 2.5)
// Size: 20 bytes of program space.

DEFINE_GRADIENT_PALETTE( Blue_Cyan_Yellow_gp ) {
    0,   0,  0,255,
   63,   0, 55,255,
  127,   0,255,255,
  191,  42,255, 45,
  255, 255,255,  0};

// Single array of defined cpt-city color palettes.
// This will let us programmatically choose one based on
// a number, rather than having to activate each explicitly 
// by name every time.
// Since it is const, this array could also be moved 
// into PROGMEM to save SRAM, but for simplicity of illustration
// we'll keep it in a regular SRAM array.
//
// This list of color palettes acts as a "playlist"; you can
// add or delete, or re-arrange as you wish.
const TProgmemRGBGradientPalettePtr gradientPalettes[] = {
  pit,
  Sunset_Real_gp,
  es_rivendell_15_gp,
  es_ocean_breeze_036_gp,
  rgi_15_gp,
  retro2_16_gp,
  Analogous_1_gp,
  es_pinksplash_08_gp,
  Coral_reef_gp,
  es_ocean_breeze_068_gp,
  es_pinksplash_07_gp,
  es_vintage_01_gp,
  departure_gp,
  es_landscape_64_gp,
  es_landscape_33_gp,
  rainbowsherbet_gp,
  gr65_hult_gp,
  gr64_hult_gp,
  GMT_drywet_gp,
  ib_jul01_gp,
  es_vintage_57_gp,
  ib15_gp,
  Fuschia_7_gp,
  es_emerald_dragon_08_gp,
  lava_gp,
  fire_gp,
  Colorfull_gp,
  Magenta_Evening_gp,
  Pink_Purple_gp,
  es_autumn_19_gp,
  BlacK_Blue_Magenta_White_gp,
  BlacK_Magenta_Red_gp,
  BlacK_Red_Magenta_Yellow_gp,
  Blue_Cyan_Yellow_gp
};

const uint8_t gradientPaletteCount = ARRAY_SIZE(gradientPalettes);


#include <limits.h>
#include "Arduino.h"
#include "FastLED.h"
#include "scrolltext.h"
#include "Font5x7.h"


#define FONTWIDTH  5
#define FONTHEIGHT 7

Scrolltext::Scrolltext() {
}

void Scrolltext::setup(int8_t stepsPerSecond, uint8_t matrix_width, uint8_t matrix_height) {
  this->matrix_width = matrix_width;
  this->matrix_height = matrix_height;
  this->steps_every_ms = 1000./stepsPerSecond;
  this->running = 0;
  this->next_step = 0;
  this->font = Font5x7;
}

void Scrolltext::ptext(const char *text, bool is_default) {
  static char namebuf[21]; // max strlen of patnames+1
  strcpy_P(namebuf, text);
  this->text(namebuf, is_default);
}

void Scrolltext::text(const char *text, bool is_default) {
  if (is_default == true) {
    this->defaulttext = text;
  }
  this->thetext = text;
  this->textlen = strlen(thetext);

  curr_x = matrix_width;
  curr_y = random8(matrix_height-FONTHEIGHT);
  dosin = random8(3);
  if (!is_default) {
    running = millis() + 1000;
  } else {
    running = millis() + 1000 * (uint32_t)random8(30, 60);
  }
}

// Put a character on the display using glcd fonts.
void Scrolltext::drawChar(CRGB* leds, void (*setPixelXY)(uint8_t x, uint8_t y, CRGB color),
            int8_t x, int8_t y, const char c, CRGB color) {
  if((x >= matrix_width)       ||
     (y >= matrix_height)      ||
     ((x + FONTWIDTH) < 0)     ||
     ((y + FONTHEIGHT) < 0))
  {
    return;
  }

  for (int8_t i = 0; i < FONTWIDTH; i++) {
    uint8_t line = pgm_read_byte(font+((c-0x20)*FONTWIDTH)+i);
    for (int j = 0; j < FONTHEIGHT; j++) {
      if (line & 0x1) {
        setPixelXY(x+i, y+j, color);
      }
      line >>= 1;
    }
  }
}

void Scrolltext::draw(CRGB* leds, void (*setPixelXY)(uint8_t x, uint8_t y, CRGB color), CRGB color) {
  uint32_t now = millis();
  if (millis() < running) {
    return;
  }

  if (now > next_step) {
    curr_x--;
    next_step = now+steps_every_ms;
  }

  if (curr_x < -(FONTWIDTH+1)*textlen) {
    // text passed completely, next start in a few seconds
    text(this->defaulttext, true);
    return;
  }

  for (int8_t i = 0; i < textlen; i++) {
    if (dosin) {
      curr_y = beatsin8(35, 0, 6, 0, (textlen-i-1)<<5)-2;
    }
    drawChar(leds, setPixelXY, curr_x+(FONTWIDTH+1)*i, curr_y, thetext[i], color);
  }
}



#ifndef _scrolltext_h_
#define _scrolltext_h_

class Scrolltext {
  public:
    Scrolltext();
    void setup(int8_t stepsPerSecond, uint8_t matrix_width, uint8_t matrix_height);
    void text(const char *text, bool is_default=false);
    void ptext(const char *text, bool is_default=false);
    void draw(CRGB* leds, void (*setPixelXY)(uint8_t x, uint8_t y, CRGB color), CRGB color);
  private:
    void drawChar(CRGB* leds, void (*setPixelXY)(uint8_t x, uint8_t y, CRGB color), int8_t x, int8_t y, const char c, CRGB color);
    void init();
    const char *thetext;
    const char *defaulttext;
    const unsigned char *font;
    uint8_t matrix_width, matrix_height;
    uint8_t textlen;
    float steps_every_ms;
    boolean dosin;
    uint32_t running, next_step;
    int16_t curr_x, curr_y;
};

#endif // _scrolltext_h_


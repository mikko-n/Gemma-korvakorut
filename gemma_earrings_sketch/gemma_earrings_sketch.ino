/* *********************************************************
 * Earring sketch for Adafruit Gemma & 16-led NeoPixel Ring
 * Button debounce adapted from Trinket Tap tempo sketch at: 
 *    http://learn.adafruit.com/tap-tempo-trinket/code
 *
 * Sine wave pulse idea from:
 *    https://www.sparkfun.com/tutorials/329
 * ********************************************************* */
#include <Adafruit_NeoPixel.h>

#define BTN 0
#define RING 1
#define RING_LED_COUNT 15

// special mode definitions
#define MODE_CHASE_1 11
#define MODE_CHASE_2 12
#define MODE_CHASE_3 13
#define MODE_RAINBOW 14
#define MODE_FLASHLIGHT 15

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(RING_LED_COUNT, RING);

// color definitions
uint32_t colors[8] = {0xFF2EEF, 0xB92EFF, 0x0B00E6, 0x00BAE6, 0x00E695, 0xDAFF05, 0xFFA405, 0xFFFFFF}; // move to PROGMEM if things get tight
uint8_t current_color = 0;

uint8_t button; // value of last digitalread() = button state
uint8_t mode = 0; // currently visible program
uint8_t offset = 0; // for spinner program
uint8_t patterns = 2; // number of light patterns
boolean preview = false; // is the code in preview mode
unsigned long previewStart; // preview button press timecode

const uint8_t brightness = 10; // sane brightness for earring use = 5-10
float sine = 1.570; // pi*0.5 to start at max point of brightness sin function (= preset brightness)

void setup() {  
  pixels.begin();
  pixels.setBrightness(brightness); // set ring brightness to something sane
  button = digitalRead(BTN);        // read initial state
}

/*
 * Get NeoPixel ring overall brightness for current mode
 * RETURNS default brightness or value based to sin function for pulse patterns
 */
uint8_t getBrightness() {
  
  uint8_t out;
  float range = brightness / 2; // sine waveform, pulse range is 1/2 of preset brightness value

  switch (mode % patterns) {
    case 0:
      // sine wave pulse
      sine += 0.01;          // adjust here if pulse rate is too quick or too slow
      if (sine > 7.808)       // = loop full wave length (2*pi) starting from pi*0.5 = 1.570
        sine = 1.570;     // pi*0.5 to start at max point of brightness sin function (= preset brightness)
        
      // NeoPixels accept brightness values between 0-255 so max value for range is 127.5
      // sin() returns from -1 to +1, multiply result with range + offset + 2 
      // to get brightness value which does not to completely shut off leds
      out = (uint8_t)(sin(sine) * range + range+2);
      if (out > brightness) {
        out=brightness;
      }
      return out;
      break;

    default:
      // static color
      return brightness;
      break;
  }
}

/* 
 * Get correct color for each mode & pattern
 * RETURNS predefined color from color array
 */
uint32_t getColor() {  
  switch (mode) {
    case MODE_RAINBOW:   // random color for rainbow spark effect
      return colors[random(8)];
      break;
    case MODE_FLASHLIGHT:  // emergency flashlight mode
      return colors[7];
      break;
    case MODE_CHASE_1:  // spinner chase 1
      return colors[0];
      break;
    case MODE_CHASE_2:  // spinner chase 2
      return colors[3];
      break;
    case MODE_CHASE_3:  // spinner chase 3
      return colors[2];
      break;
    default:   // no specific color for pgm, change color when pattern changes
      if ((mode % patterns) == 0) {
        current_color++;
      }  
      if (current_color > 8) {
        current_color = 0;
      }
      break;
  }
  
  return colors[current_color];
}

/*
 * Method to handle button press for preview mode and program state changes
 */
static void readBtn() {
  uint8_t b, i;
  unsigned long last;
  long d;
 
    last = micros();
    while ((b = digitalRead(BTN)) != button) { // button state changed
      if ((micros() - last) > 25000L) {        // delay for state change > 25mS
        button = b;
        if (button == HIGH) {                  // button pressed
          previewStart = micros();             // set/reset preview mode start time
          preview = true;                      // set preview mode
                   
          // mode advance & limit
          mode++;                                
          if (mode > RING_LED_COUNT-1)
            mode = 0;
          
          for (i=0;i<RING_LED_COUNT;i++) pixels.setPixelColor(i, 0, 0, 0); // clear ring          
          pixels.setPixelColor(mode, getColor());  // paint preview of the selected color         
        }
      }
    }
    // else, button is unchanged for 3 sec, exit preview mode
    d = (last - previewStart) - 3000000L; // function start minus 3 sec
    if (d > 0) {                   // no button in last 3 sec
      preview = false;             //  = preview mode off
      previewStart = 0L;
    }    
}

// main program
void loop() {
  uint8_t i;
   
  readBtn(); // handle button & program changes
  pixels.setBrightness(getBrightness()); // set ring overall brightness

  // paint selected program only if not in preview mode
  if (preview == false) {
    switch(mode) {
      case MODE_RAINBOW:      // rainbow sparks
        i = random(16);
        pixels.setPixelColor(i, getColor());
        pixels.show();
        delay(40);
        pixels.setPixelColor(i, 0);
        break;
      case MODE_FLASHLIGHT:     // emergency flashlight
        pixels.setBrightness(70);
        break;
      case MODE_CHASE_1: // Spinny wheels (8 LEDs on at a time)
      case MODE_CHASE_2:
      case MODE_CHASE_3:
        pixels.setBrightness(brightness);
        for(i=0; i<16; i++) {
          uint32_t c = 0;
          if(((offset + i) & 7) < 2) c = getColor(); // 4 pixels on...
          pixels.setPixelColor( i, c);          
        }
        pixels.show();
        offset++;
        delay(60);
        break;
      default:
        for (i=0;i<RING_LED_COUNT;i++) {    
          pixels.setPixelColor(i, getColor());    
        }
        break;
    }
  }
  
  pixels.show();    
}

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>
#include <vector>
#include <algorithm>

// 5x7 ASCII font (from 0x00 to 0x7F)
// Each character is 5 pixels wide and 7 pixels tall
// Stored row-wise, LSB = top pixel
// You can render with drawPixel or pushPixels

#include <stdint.h>
#include "gfx.h"

const uint8_t font5x7[][5] = {
  // 0x00–0x1F (control chars, usually blank)
  {0,0,0,0,0}, // NUL
  {0,0,0,0,0}, // SOH
  {0,0,0,0,0}, // STX
  {0,0,0,0,0}, // ETX
  {0,0,0,0,0}, // EOT
  {0,0,0,0,0}, // ENQ
  {0,0,0,0,0}, // ACK
  {0,0,0,0,0}, // BEL
  {0,0,0,0,0}, // BS
  {0,0,0,0,0}, // TAB
  {0,0,0,0,0}, // LF
  {0,0,0,0,0}, // VT
  {0,0,0,0,0}, // FF
  {0,0,0,0,0}, // CR
  {0,0,0,0,0}, // SO
  {0,0,0,0,0}, // SI
  {0,0,0,0,0}, // DLE
  {0,0,0,0,0}, // DC1
  {0,0,0,0,0}, // DC2
  {0,0,0,0,0}, // DC3
  {0,0,0,0,0}, // DC4
  {0,0,0,0,0}, // NAK
  {0,0,0,0,0}, // SYN
  {0,0,0,0,0}, // ETB
  {0,0,0,0,0}, // CAN
  {0,0,0,0,0}, // EM
  {0,0,0,0,0}, // SUB
  {0,0,0,0,0}, // ESC
  {0,0,0,0,0}, // FS
  {0,0,0,0,0}, // GS
  {0,0,0,0,0}, // RS
  {0,0,0,0,0}, // US

  // 0x20 ' '
  {0x00,0x00,0x00,0x00,0x00}, // Space
  // 0x21 '!'
  {0x00,0x00,0x5F,0x00,0x00}, // !
  // 0x22 '"'
  {0x00,0x07,0x00,0x07,0x00}, // "
  // 0x23 '#'
  {0x14,0x7F,0x14,0x7F,0x14}, // #
  // 0x24 '$'
  {0x24,0x2A,0x7F,0x2A,0x12}, // $
  // 0x25 '%'
  {0x23,0x13,0x08,0x64,0x62}, // %
  // 0x26 '&'
  {0x36,0x49,0x55,0x22,0x50}, // &
  // 0x27 '''
  {0x00,0x05,0x03,0x00,0x00}, // '
  // 0x28 '('
  {0x00,0x1C,0x22,0x41,0x00}, // (
  // 0x29 ')'
  {0x00,0x41,0x22,0x1C,0x00}, // )
  // 0x2A '*'
  {0x14,0x08,0x3E,0x08,0x14}, // *
  // 0x2B '+'
  {0x08,0x08,0x3E,0x08,0x08}, // +
  // 0x2C ','
  {0x00,0x50,0x30,0x00,0x00}, // ,
  // 0x2D '-'
  {0x08,0x08,0x08,0x08,0x08}, // -
  // 0x2E '.'
  {0x00,0x60,0x60,0x00,0x00}, // .
  // 0x2F '/'
  {0x20,0x10,0x08,0x04,0x02}, // /
  // 0x30 '0'
  {0x3E,0x51,0x49,0x45,0x3E}, // 0
  // 0x31 '1'
  {0x00,0x42,0x7F,0x40,0x00}, // 1
  // 0x32 '2'
  {0x42,0x61,0x51,0x49,0x46}, // 2
  // 0x33 '3'
  {0x21,0x41,0x45,0x4B,0x31}, // 3
  // 0x34 '4'
  {0x18,0x14,0x12,0x7F,0x10}, // 4
  // 0x35 '5'
  {0x27,0x45,0x45,0x45,0x39}, // 5
  // 0x36 '6'
  {0x3C,0x4A,0x49,0x49,0x30}, // 6
  // 0x37 '7'
  {0x01,0x71,0x09,0x05,0x03}, // 7
  // 0x38 '8'
  {0x36,0x49,0x49,0x49,0x36}, // 8
  // 0x39 '9'
  {0x06,0x49,0x49,0x29,0x1E}, // 9
  // 0x3A ':'
  {0x00,0x36,0x36,0x00,0x00}, // :
  // 0x3B ';'
  {0x00,0x56,0x36,0x00,0x00}, // ;
  // 0x3C '<'
  {0x08,0x14,0x22,0x41,0x00}, // <
  // 0x3D '='
  {0x14,0x14,0x14,0x14,0x14}, // =
  // 0x3E '>'
  {0x00,0x41,0x22,0x14,0x08}, // >
  // 0x3F '?'
  {0x02,0x01,0x51,0x09,0x06}, // ?
  // 0x40 '@'
  {0x32,0x49,0x79,0x41,0x3E}, // @
  // 0x41 'A'
  {0x7E,0x11,0x11,0x11,0x7E}, // A
  // 0x42 'B'
  {0x7F,0x49,0x49,0x49,0x36}, // B
  // 0x43 'C'
  {0x3E,0x41,0x41,0x41,0x22}, // C
  // 0x44 'D'
  {0x7F,0x41,0x41,0x22,0x1C}, // D
  // 0x45 'E'
  {0x7F,0x49,0x49,0x49,0x41}, // E
  // 0x46 'F'
  {0x7F,0x09,0x09,0x09,0x01}, // F
  // 0x47 'G'
  {0x3E,0x41,0x49,0x49,0x7A}, // G
  // 0x48 'H'
  {0x7F,0x08,0x08,0x08,0x7F}, // H
  // 0x49 'I'
  {0x00,0x41,0x7F,0x41,0x00}, // I
  // 0x4A 'J'
  {0x20,0x40,0x41,0x3F,0x01}, // J
  // 0x4B 'K'
  {0x7F,0x08,0x14,0x22,0x41}, // K
  // 0x4C 'L'
  {0x7F,0x40,0x40,0x40,0x40}, // L
  // 0x4D 'M'
  {0x7F,0x02,0x0C,0x02,0x7F}, // M
  // 0x4E 'N'
  {0x7F,0x04,0x08,0x10,0x7F}, // N
  // 0x4F 'O'
  {0x3E,0x41,0x41,0x41,0x3E}, // O
  // 0x50 'P'
  {0x7F,0x09,0x09,0x09,0x06}, // P
  // 0x51 'Q'
  {0x3E,0x41,0x51,0x21,0x5E}, // Q
  // 0x52 'R'
  {0x7F,0x09,0x19,0x29,0x46}, // R
  // 0x53 'S'
  {0x46,0x49,0x49,0x49,0x31}, // S
  // 0x54 'T'
  {0x01,0x01,0x7F,0x01,0x01}, // T
  // 0x55 'U'
  {0x3F,0x40,0x40,0x40,0x3F}, // U
  // 0x56 'V'
  {0x1F,0x20,0x40,0x20,0x1F}, // V
  // 0x57 'W'
  {0x3F,0x40,0x38,0x40,0x3F}, // W
  // 0x58 'X'
  {0x63,0x14,0x08,0x14,0x63}, // X
  // 0x59 'Y'
  {0x07,0x08,0x70,0x08,0x07}, // Y
  // 0x5A 'Z'
  {0x61,0x51,0x49,0x45,0x43}, // Z
  // 0x5B '['
  {0x00,0x7F,0x41,0x41,0x00}, // [
  // 0x5C '\'
  {0x02,0x04,0x08,0x10,0x20}, // \
  // 0x5D ']'
  {0x00,0x41,0x41,0x7F,0x00}, // ]
  // 0x5E '^'
  {0x04,0x02,0x01,0x02,0x04}, // ^
  // 0x5F '_'
  {0x40,0x40,0x40,0x40,0x40}, // _
  // 0x60 '`'
  {0x00,0x01,0x02,0x04,0x00}, // `
  // 0x61 'a'
  {0x20,0x54,0x54,0x54,0x78}, // a
  // 0x62 'b'
  {0x7F,0x48,0x44,0x44,0x38}, // b
  // 0x63 'c'
  {0x38,0x44,0x44,0x44,0x20}, // c
  // 0x64 'd'
  {0x38,0x44,0x44,0x48,0x7F}, // d
  // 0x65 'e'
  {0x38,0x54,0x54,0x54,0x18}, // e
  // 0x66 'f'
  {0x08,0x7E,0x09,0x01,0x02}, // f
  // 0x67 'g'
  {0x0C,0x52,0x52,0x52,0x3E}, // g
  // 0x68 'h'
  {0x7F,0x08,0x04,0x04,0x78}, // h
  // 0x69 'i'
  {0x00,0x44,0x7D,0x40,0x00}, // i
  // 0x6A 'j'
  {0x20,0x40,0x44,0x3D,0x00}, // j
  // 0x6B 'k'
  {0x7F,0x10,0x28,0x44,0x00}, // k
  // 0x6C 'l'
  {0x00,0x41,0x7F,0x40,0x00}, // l
  // 0x6D 'm'
  {0x7C,0x04,0x18,0x04,0x78}, // m
  // 0x6E 'n'
  {0x7C,0x08,0x04,0x04,0x78}, // n
  // 0x6F 'o'
  {0x38,0x44,0x44,0x44,0x38}, // o
  // 0x70 'p'
  {0x7C,0x14,0x14,0x14,0x08}, // p
  // 0x71 'q'
  {0x08,0x14,0x14,0x18,0x7C}, // q
  // 0x72 'r'
  {0x7C,0x08,0x04,0x04,0x08}, // r
  // 0x73 's'
  {0x48,0x54,0x54,0x54,0x20}, // s
  // 0x74 't'
  {0x04,0x3F,0x44,0x40,0x20}, // t
  // 0x75 'u'
  {0x3C,0x40,0x40,0x20,0x7C}, // u
  // 0x76 'v'
  {0x1C,0x20,0x40,0x20,0x1C}, // v
  // 0x77 'w'
  {0x3C,0x40,0x30,0x40,0x3C}, // w
  // 0x78 'x'
  {0x44,0x28,0x10,0x28,0x44}, // x
  // 0x79 'y'
  {0x0C,0x50,0x50,0x50,0x3C}, // y
  // 0x7A 'z'
  {0x44,0x64,0x54,0x4C,0x44}, // z
  // 0x7B '{'
  {0x00,0x08,0x36,0x41,0x00}, // {
  // 0x7C '|'
  {0x00,0x00,0x7F,0x00,0x00}, // |
  // 0x7D '}'
  {0x00,0x41,0x36,0x08,0x00}, // }
  // 0x7E '~'
  {0x08,0x04,0x08,0x10,0x08}, // ~
  // 0x7F (DEL)
  {0,0,0,0,0}  
};

 
 

// Double buffer system
static uint16_t frontBuffer[ WIDTH *  HEIGHT];
static uint16_t backBuffer[ WIDTH *  HEIGHT];
static bool bufferDirty = false;

// Initialize the buffer system
void initTextBuffer() {
    memset(frontBuffer, 0, sizeof(frontBuffer));
    memset(backBuffer, 0, sizeof(backBuffer));
    bufferDirty = true;
}

// Clear the back buffer
void clearTextBuffer() {
    memset(backBuffer, 0, sizeof(backBuffer));
    bufferDirty = true;
}

// Set pixel in back buffer
 void setBufferPixel(int x, int y, uint16_t color) {
    if (x >= 0 && x <  WIDTH && y >= 0 && y <  HEIGHT) {
        backBuffer[y *  WIDTH + x] = color;
        bufferDirty = true;
    }
}

 uint16_t fastRGB565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

 


// Fill rectangle in buffer
void fillBufferRect(int x, int y, int w, int h, uint16_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            setBufferPixel(x + dx, y + dy, color);
        }
    }
}

// Get pixel from back buffer
  uint16_t getBufferPixel(int x, int y) {
    if (x >= 0 && x <  WIDTH && y >= 0 && y <  HEIGHT) {
        return backBuffer[y *  WIDTH + x];
    }
    return 0;
}

// Direct character drawing to buffer
void drawCharToBuffer(int x, int y, char c, uint16_t color) {
    if (c < 0x20 || c > 0x7F) return;
    
    for (int col = 0; col < 5; col++) {
        uint8_t line = font5x7[c - 0x20][col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                setBufferPixel(x + col, y + row, color);
            }
        }
    }
}

// Draw text string to buffer
void drawTextToBuffer(int x, int y, const char *text, uint16_t color) {
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        drawCharToBuffer(x + (i * 6), y, text[i], color);
    }
}

// RGB to 16-bit color conversion
 uint16_t rgb565(int r, int g, int b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Draw text with RGB values
void drawTextRGB(int x, int y, const char *text, int r, int g, int b) {
    uint16_t color = rgb565(r, g, b);
    drawTextToBuffer(x, y, text, color);
}

 
 

// Block copy method - fastest for full screen updates
void pageFlip() {
    if (!bufferDirty) return;
    
    // Copy buffer
    memcpy(frontBuffer, backBuffer, sizeof(frontBuffer));
    
    // Block update to display (if your display supports it)
    // This assumes your display library has a bulk update method
    // Replace with actual bulk update method if available
    
    // Fallback: row-by-row update for better performance
    for (register int y = 0; y <  HEIGHT; y++) {
        for (register int x = 0; x <  WIDTH; x++) {
            dma_display->drawPixel(x, y, frontBuffer[y * WIDTH + x]);
        }
    }
    
    bufferDirty = false;
}


  void proverbe() {
   // initTextBuffer(); // Initialize display buffer

    // Initialize buftext
    char buftext[92] = "\0";

    
    int r = random(0, 64); // Random value for selecting proverb (0-99)

     
    const char *proverbs[92] PROGMEM =   {
  "I m not arguing  I m just explaining why I m right\0" ,
  "To err is human  to blame it on someone else shows management potential\0" ,
  "I used to think I was indecisive  but now I m not too sure\0" ,
  "Why do they call it rush hour when nothing moves?\0" ,
  "I m on a seafood diet  I see food and I eat it\0" ,
  "If at first you don t succeed  skydiving is not for you\0" ,
  "Life is short  Smile while you still have teeth\0" ,
  "I told my computer I needed a break  and it said  404: Motivation not found \0" ,
  "Always borrow money from a pessimist  They wont expect it back\0" ,
  "Behind every successful person is a substantial amount of coffee\0" ,
  "If you think nobody cares if you re alive  try missing a couple of car payments\0" ,
  "My bed is a magical place where I suddenly remember everything I forgot to do\0" ,
  "The early bird might get the worm  but the second mouse gets the cheese\0" ,
  "I used to be a people person  but people ruined that for me\0" ,
  "Why yes  I do frequently burst out in song  Thanks for noticing\0" ,
  "Im not lazy  Im just on energy-saving mode\0" ,
  "Common sense is like deodorant  The people who need it most never use it\0" ,
  "Im not weird  Im limited edition\0" ,
  "Some people graduate with honors  I am just honored to graduate\0" ,
  "I put the pro in procrastinate\0", 
  "Im not short  Im concentrated awesome\0" ,
  "I tried to be normal once  Worst two minutes ever\0" ,
  "If life gives you lemons  squirt someone in the eye\0" ,
  "I dont suffer from insanityI enjoy every minute of it\0" ,
  "I m not lazy  I m just highly motivated to do nothing\0" ,
  "I thought I wanted a career  turns out I just wanted a paycheck\0" ,
  "I clean when Im angry  So if you see me cleaning  run\0" ,
  "Im not bossy  I just have better ideas\0",
  "Im not old  Im 25 plus shipping and handling\0" ,
  "I dance because theres no guarantee the world wont end tomorrow\0" ,
   "Im not a complete idiotsome parts are missing\0" ,
  "Im not arguing  Im just passionately expressing my point of view\0" ,
  "Im not sure how many problems I have because math is one of them\0" ,
  "Im not procrastinating  Im proactively delaying the inevitable\0" ,
  "Im not ignoring you  Im just giving you time to reflect on what you just said\0" ,
  "Im not late  Im just early for tomorrow\0" ,
  "Im not overthinking  Im just thinking more than you\0" ,
  "The unexamined life is not worth living  \0" ,
   "Imagination is more important than knowledge  \0" ,
  "All we have to decide is what to do with the time that is given us  \0" ,
  "Reality is merely an illusion  albeit a very persistent one \0" ,
  "The needs of the many outweigh the needs of the few  \0" ,
  "I think  therefore I am  \0" ,
  "The future is already here it s just not evenly distributed  \0" ,
  "A person is smart  People are dumb    \0" ,
    "Fear is the mind-killer  \0" ,
  "Not all those who wander are lost  \0" ,
  "Any sufficiently advanced technology is indistinguishable from magic  \0", 
  "We are all in the gutter  but some of us are looking at the stars  \0" ,
  "Life finds a way Jurassic Park\0" ,
  "You must unlearn what you have learned Yoda\0" ,
  "The cosmos is within us  We are made of star-stuff  \0" ,
  "There is no spoon The Matrix\0" ,
   "To boldly go where no one has gone before Star Trek\0" ,
  "In the middle of difficulty lies opportunity  \0" ,
  "The only true wisdom is in knowing you know nothing  \0" ,
  "You cant stop the signal  \0" ,
  "The truth is out there X-Files\0" ,
  "We are the music makers  and we are the dreamers of dreams  \0" ,
  "I have no special talents  I am only passionately curious  \0" ,
  "The mind is everything  What you think you become Buddha\0" ,
   "The only limit to our realization of tomorrow is our doubts of today FDR\0" ,
  "The universe is under no obligation to make sense to you  \0" ,
  "All models are wrong  but some are useful \0" ,
  "The best way to predict the future is to invent it  \0" ,
  "You cant take the sky from me  \0"
   
  };
     

    // Copy selected proverb from PROGMEM to buftext
    strcpy_P(buftext, (const char *)pgm_read_ptr(&proverbs[r]));

    const int charWidth = 8; // 6 pixels + 2 spacing for readability
    const int baseY = 30; // Fixed y position
    const int amplitude = 13; // Wave amplitude
    const float frequency = 5.0f; // Wave frequency
    int startX = 50; // Start on right side

    // Create a lowercase copy of buftext
    char lowerText[92];

    for (int i = 0; i < 92 && buftext[i] != '\0'; i++) {
   
        lowerText[i] = tolower(buftext[i]);
        lowerText[i+1]= '\0';
    }
 

    // Trim leading spaces from lowerText
    const char *text = lowerText;
    while (*text == ' ') text++;
    int textLength = strlen(text);

    float baz = 0.0f;

    // Retro color palette (neon-inspired: green, magenta, cyan, yellow)
    const uint16_t retroColors[4] = {
        rgb565(0, 255, 0),   // Neon green
        rgb565(255, 0, 255), // Neon magenta
        rgb565(0, 255, 255), // Neon cyan
        rgb565(255, 255, 0)  // Neon yellow
    };

    // Scroll until the entire string is off-screen to the left
    while (startX > -textLength * charWidth) {
        clearTextBuffer();

        // Draw each letter with correct order (first letter at startX)
        for (int l = 0; l < textLength && text[l] != '\0'; l++) {
            int x = startX + l * charWidth; // Position letters to the right of startX
            if (x >= -charWidth && x < 80) { // 80px screen width
                int waveY = baseY + amplitude * cos((baz + l * 1.5f) / frequency);
                // Cycle retro colors based on letter index and animation state
                int colorIndex = (int)(l + baz / 2.0f) % 4; // Smooth color cycling
                uint16_t color = retroColors[colorIndex];
                drawCharToBuffer(x, waveY, text[l], color);
            }
        }

        pageFlip();
        startX -= 1; // Move string left
        baz += 1.4f; // Smooth wave progression
        delay(30); // Smooth scrolling (~30 FPS)
    }
}


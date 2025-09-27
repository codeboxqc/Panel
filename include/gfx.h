#ifndef FIRE_H
#define FIRE_H

#include <stdint.h>



// ====================================================
// HUB75 Panel Configuration
// ====================================================
#define R1_PIN 17
#define B1_PIN 8
#define R2_PIN 3
#define B2_PIN 10
#define A_PIN 15
#define C_PIN 7
#define CLK_PIN 5
#define OE_PIN 12
#define G1_PIN 18
#define G2_PIN 2
#define E_PIN 13
#define B_PIN 11
#define D_PIN 4
#define LAT_PIN 6



 
#define PANEL_CHAIN 1       // How many panels chained

#define WIDTH 64
#define HEIGHT 64
 

void time(int x,int y);
void inittime();

 
extern MatrixPanel_I2S_DMA *dma_display;
 extern  uint16_t palette256[256];
extern  uint16_t palette[16];


extern  void initAsteroids();
extern void asteroid();
bool checkFirstBoot();

/////////////////////////////////////////
// Struct definition fire
typedef struct {
    uint8_t* buffer;
    uint8_t* fire_buffer;
    int width;
    int height;
} Flame;

// Global flame instance (declared here, defined once in a .cpp file)
extern Flame flame;

///////////////////////////////////////////////////


/////////////////////////////////////////////////////
//plasma
//#define PI 3.14159265358979323846
#define NB 80        // Number of stars for 64x64 panel
#define STAR_SPEED 4.0f // Speed of stars moving toward viewer
#define Z_FAR 300.0f   // Far z-plane (lower zoom)
#define Z_NEAR 10.0f   // Near z-plane (stars reset here)
#define TRAIL_LENGTH 11 // Number of trail pixels per star
#define TRAIL_FADE 0.9f // Fade factor per trail step (60% 
 
typedef struct {
    uint8_t* body;            // Plasma buffer
    uint8_t palette[257][3];  // 256-color palette
    int cosinus[256];         // Cosine table
    int width;
    int height;
} Plasma;
extern Plasma plasma;


// Functions
void initFlame(Flame* f);
void updateFlame(Flame* f);
void drawFlame(Flame* f);
/////////////////////////////////////////////////////////////////

void draw3DLineAnimations(unsigned long time) ;

/////////////////////////////////////////////////////////////////
// Additional parameters for enhanced effect
 // Matrix rain structure
typedef struct {
  uint8_t speed[64];        // Speed of each column
  uint8_t length[64];       // Length of each rain column
  uint8_t position[64];     // Current position of each column
  uint32_t last_update[64]; // Last update time for each column
  uint8_t intensity[64];    // Intensity/brightness of each column
  uint8_t chars[64][32];    // Characters for each column
} MatrixRain;

 void initMatrixRain(MatrixRain* rain);
 void updateMatrixRain(MatrixRain* rain);
 void drawMatrixRain(MatrixRain* rain) ;
/////////////////////////////////////////////////////////////////

 
 ///////////////////////////////////
 //Modern Particle Wave Animation
#define NUM_PARTICLES 9
  
typedef struct {
  float x, y;
  float vx, vy;
  uint8_t color_index;
  uint8_t size;
  uint8_t mass;
} Particle;



 
const uint32_t palettes[][8]  PROGMEM= {
  // Cyberpunk (blues and purples)
  {0xFF00FF, 0x00FFFF, 0x0000FF, 0xFF00FF, 0x00FF00, 0xFFFF00, 0xFF0000, 0xFFFFFF},
  // Neon (bright colors)
  {0xFF007F, 0x00FF7F, 0x7F00FF, 0xFF7F00, 0x7FFF00, 0x007FFF, 0xFF00FF, 0x00FFFF},
  // Pastel (soft colors)
  {0xFFB6C1, 0x87CEFA, 0x98FB98, 0xDDA0DD, 0xFFD700, 0xFFA07A, 0xADD8E6, 0xF0E68C},
  // Monochrome (blue tones)
  {0x0000FF, 0x0066FF, 0x00CCFF, 0x33FFFF, 0x66FFFF, 0x99FFFF, 0xCCFFFF, 0xFFFFFF}
};

void initParticleSystem();
void checkParticleCollision(Particle* p1, Particle* p2);
void updateParticleSystem() ;
void drawParticleSystem();
void nextColorMode();
 ///////////////////////////////////////////
 

void initPlasma(Plasma* plasma);
 void updatePlasma(Plasma* plasma);
 void drawPlasma(Plasma* plasma);

 void starfieldEffect(unsigned long time);
 void updateStarfield();
 void initStarfield();

 uint8_t getPixel(const unsigned char* buf, int idx);
 void setPixel(unsigned char* buf, int idx, uint8_t color);
 
 void putimageColorMap(int px, int py, const unsigned char* img, int color) ;
 void putimagesize(int px, int py, const unsigned char* img, int size);
 void putimage(int px, int py, const unsigned char* img);
 void putimageFade(int px, int py, const unsigned char* img, float fade);
 void putimageRotate(int px, int py, const unsigned char* img, int degrees);
 void putimageInvert(int px, int py, const unsigned char* img) ;
 void putimageBlend(int px, int py, const unsigned char* img1, const unsigned char* img2, float blend) ;
 
 
 
 void putimageCycle(int px, int py, const unsigned char* img, int offset);
 

 
 void plasmaEffect(unsigned long time);
 void pixelRainEffect(unsigned long time);
 void  Rain (unsigned long time);
 void rain16(unsigned long time);
 void matrixDigitalRain(unsigned long time);
 void fireAnimation(unsigned long time);
 void switchFractalPlasma(unsigned long time) ;
 void fastFractalPlasma(unsigned long time);
 void fastFractalMorph(unsigned long time);
 void vanGoghPaintAnimation(unsigned long time );
void Jetstream(float t);
void Pivotal(float t);
void Pillars(float t);
void RadialGlow(uint32_t time);
void RaymarchGlow(float t);
void TronMatrixPulse(float t);
void GooGlow(float t) ;

void textRenderingExample() ;
void initTextBuffer();
void clearTextBuffer();
 void setBufferPixel(int x, int y, uint16_t color);
 uint16_t getBufferPixel(int x, int y) ;
void drawCharToBuffer(int x, int y, char c, uint16_t color);
void drawTextToBuffer(int x, int y, const char *text, uint16_t color) ;
 uint16_t rgb565(int r, int g, int b);
void drawTextRGB(int x, int y, const char *text, int r, int g, int b);
void pageFlip();
void fillBufferRect(int x, int y, int w, int h, uint16_t color);
 uint16_t fastRGB565(uint8_t r, uint8_t g, uint8_t b);
uint16_t hsvToRgb(float h, float s, float v);
 

float fastSin(float x) ;
 void proverbe(  );
void matrixRain();

extern char buftext[256];
 
 extern unsigned char i1[2048] ;
extern unsigned char i2[2048] ;
extern unsigned char i3[2048] ;
extern unsigned char i4[2048] ;
extern unsigned char i5[2048] ;
extern unsigned char i6[2048] ;
extern unsigned char i7[2048] ;
extern unsigned char i8[2048] ;
extern unsigned char i9[2048] ;
extern unsigned char i10[2048] ;
extern unsigned char i11[2048] ;
extern unsigned char i12[2048] ;
extern unsigned char i13[2048] ;
extern unsigned char i14[2048] ;
extern unsigned char i15[2048] ;


  
 
#endif

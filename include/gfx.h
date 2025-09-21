#ifndef FIRE_H
#define FIRE_H

#include <stdint.h>



// ====================================================
// HUB75 Panel Configuration
// ====================================================
#define PANEL_RES_X 64      // Width of a single panel
#define PANEL_RES_Y 64      // Height of a single panel
#define PANEL_CHAIN 1       // How many panels chained


void time(int x,int y);
void inittime();

extern unsigned char i1[2048];
extern unsigned char i2[2048];
extern unsigned char i3[2048];
extern unsigned char i4[2048];
extern unsigned char i5[2048];
extern unsigned char i6[2048];
extern unsigned char i7[2048];
extern unsigned char i8[2048];
extern unsigned char i9[2048];
extern unsigned char i10[2048];
extern unsigned char i11[2048];
extern unsigned char i12[2048];
extern unsigned char i13[2048];
extern unsigned char i14[2048];
extern unsigned char i15[2048];
extern unsigned char i16[2048];
extern unsigned char i17[2048];
extern unsigned char i18[2048];
extern unsigned char i19[2048];
extern unsigned char i20[2048];
extern unsigned char i21[2048];
extern unsigned char i22[2048];
extern unsigned char i23[2048];

extern MatrixPanel_I2S_DMA *dma_display;
 
extern  uint16_t palette256[256];
extern  uint16_t palette[16];


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
#define NB 100        // Number of stars for 64x64 panel
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

#define HEIGHT 64
#define WIDTH 64

/////////////////////////////////////////////////////////////////
// Water ripple structure
typedef struct {
  int16_t current[HEIGHT][WIDTH];
  int16_t previous[HEIGHT][WIDTH];
  uint32_t last_update;
  uint8_t damping;
  uint8_t ripple_intensity;
  uint32_t last_ripple_time;
  uint8_t color_mode;
} WaterRipple;

void initWaterRipple(WaterRipple* water);
void updateWaterRipple(WaterRipple* water);
void drawWaterRipple(WaterRipple* water);
void addRippleAt(WaterRipple* water, int x, int y);
/////////////////////////////////////////////////////////////////


 /////////////////////////////////////////////////
typedef struct {
  float angle;
  float sweep_speed;
  uint8_t trail[HEIGHT][WIDTH];
  uint8_t blip_map[HEIGHT][WIDTH]; 
  uint32_t last_update;
} RadarScan;

extern RadarScan radar;

void drawRadarScan();
void updateRadarScan();
void initRadarScan() ;
 ///////////////////////////////////////////////


 ///////////////////////////////////
 //Modern Particle Wave Animation
#define NUM_PARTICLES 12
  
typedef struct {
  float x, y;
  float vx, vy;
  uint8_t color_index;
  uint8_t size;
  uint8_t mass;
} Particle;



// Modern color palettes
const uint32_t palettes[][8] = {
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
 void clipini();
 void putimageColorMap(int px, int py, const unsigned char* img, int color) ;
 void putimagesize(int px, int py, const unsigned char* img, int size);
 void putimage(int px, int py, const unsigned char* img);
 void putimageFade(int px, int py, const unsigned char* img, float fade);
 void putimageRotate(int px, int py, const unsigned char* img, int degrees);
 void putimageInvert(int px, int py, const unsigned char* img) ;
 void putimageBlend(int px, int py, const unsigned char* img1, const unsigned char* img2, float blend) ;
//{ // blend=0: img1, 1: img2
 

 void plasmaEffect(int px, int py, unsigned long timeOffset) ;
 void putimageCycle(int px, int py, const unsigned char* img, int offset);
 unsigned char* convertimagesize(int px, int py, const unsigned char* img, int size);



 
 void plasmaEffect(unsigned long time);
 void vortexEffect(unsigned long time);
 void vortexEffect16(unsigned long time);
 void pixelRainEffect(unsigned long time);
 void  Rain (unsigned long time);
 void rain16(unsigned long time);
void plasmaEffect(int px, int py, unsigned long timeOffset);
 void vortexEffect256Simple(unsigned long time);
 void vortexEffect256Fast(unsigned long time);
 void matrixDigitalRain(unsigned long time);
void fireAnimation(unsigned long time);
void switchFractalPlasma(unsigned long time) ;
 void fastFractalPlasma(unsigned long time);
 void fastFractalMorph(unsigned long time);
 void vanGoghPaintAnimation(unsigned long time);
void Jetstream(float t);
void Pivotal(float t);
void Pillars(float t);
void RadialGlow(float t);
void RaymarchGlow(float t);
void SineChaosGlow(float t);
void GooGlow(float t) ;



#endif

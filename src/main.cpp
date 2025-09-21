#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>

#include "gfx.h"
 
 

// ====================================================
//  
//  
//  
// ====================================================







// Pin configuration - adjust these for your ESP32 setup
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


MatrixPanel_I2S_DMA *dma_display;



uint16_t palette[16] = {
  0x0000, // 0 - black
  0xF800, // 1 - red
  0x07E0, // 2 - green
  0x001F, // 3 - blue
  0xFFE0, // 4 - yellow
  0xF81F, // 5 - magenta
  0x07FF, // 6 - cyan
  0xFFFF, // 7 - white
  0x7BEF, // 8 - gray
  0x4208, // 9 - dark gray
  0xFBE0, // 10 - orange
  0xAFE5, // 11 - teal
  0x780F, // 12 - purple
  0xFFE5, // 13 - light yellow
  0x07F3, // 14 - aqua
  0xAD55  // 15 - pinkish
};



// 256-color palette definition
uint16_t palette256[256] = {
    // Basic 16 colors repeated for simplicity
    0x0000, 0xF800, 0x07E0, 0x001F, 0xFFE0, 0xF81F, 0x07FF, 0xFFFF,
    0x7BEF, 0x4208, 0xFBE0, 0xAFE5, 0x780F, 0xFFE5, 0x07F3, 0xAD55,
    // Add more colors (gradient for now)
    // You should replace this with your actual 256-color palette
    0x0000, 0x0800, 0x1000, 0x1800, 0x2000, 0x2800, 0x3000, 0x3800, // Reds
    0x4000, 0x4800, 0x5000, 0x5800, 0x6000, 0x6800, 0x7000, 0x7800,
    0x8000, 0x8800, 0x9000, 0x9800, 0xA000, 0xA800, 0xB000, 0xB800,
    0xC000, 0xC800, 0xD000, 0xD800, 0xE000, 0xE800, 0xF000, 0xF800,
    // Greens
    0x01E0, 0x03E0, 0x05E0, 0x07E0, 0x09E0, 0x0BE0, 0x0DE0, 0x0FE0,
    0x11E0, 0x13E0, 0x15E0, 0x17E0, 0x19E0, 0x1BE0, 0x1DE0, 0x1FE0,
    0x21E0, 0x23E0, 0x25E0, 0x27E0, 0x29E0, 0x2BE0, 0x2DE0, 0x2FE0,
    0x31E0, 0x33E0, 0x35E0, 0x37E0, 0x39E0, 0x3BE0, 0x3DE0, 0x3FE0,
    // Blues
    0x0003, 0x0007, 0x000B, 0x000F, 0x0013, 0x0017, 0x001B, 0x001F,
    0x0023, 0x0027, 0x002B, 0x002F, 0x0033, 0x0037, 0x003B, 0x003F,
    0x0043, 0x0047, 0x004B, 0x004F, 0x0053, 0x0057, 0x005B, 0x005F,
    0x0063, 0x0067, 0x006B, 0x006F, 0x0073, 0x0077, 0x007B, 0x007F,
    // Yellows, magentas, cyans, etc.
    0xFFE0, 0xFFC0, 0xFFA0, 0xFF80, 0xFF60, 0xFF40, 0xFF20, 0xFF00,
    0xF81F, 0xF01F, 0xE81F, 0xE01F, 0xD81F, 0xD01F, 0xC81F, 0xC01F,
    0x07FF, 0x07DF, 0x07BF, 0x079F, 0x077F, 0x075F, 0x073F, 0x071F,
    0xFFFF, 0xEF7D, 0xDEFB, 0xCE79, 0xBDF7, 0xAD75, 0x9CF3, 0x8C71,
    // Grays
    0x7BEF, 0x73AE, 0x6B6D, 0x632C, 0x5AEB, 0x52AA, 0x4A69, 0x4228,
    0x4208, 0x39E7, 0x31A6, 0x2965, 0x2124, 0x18E3, 0x10A2, 0x0861,
    // More colors (fill remaining indices)
    0xFBE0, 0xF3A0, 0xEB60, 0xE320, 0xDAE0, 0xD2A0, 0xCA60, 0xC220,
    0xAFE5, 0xA7E5, 0x9FE5, 0x97E5, 0x8FE5, 0x87E5, 0x7FE5, 0x77E5,
    0x780F, 0x700F, 0x680F, 0x600F, 0x580F, 0x500F, 0x480F, 0x400F,
    0xFFE5, 0xF7C5, 0xEFA5, 0xE785, 0xDF65, 0xD745, 0xCF25, 0xC705,
    0x07F3, 0x07D3, 0x07B3, 0x0793, 0x0773, 0x0753, 0x0733, 0x0713,
    0xAD55, 0xA535, 0x9D15, 0x94F5, 0x8CD5, 0x84B5, 0x7C95, 0x7475,
    // Remaining indices (repeat or add more gradients)
    0x0000, 0x0800, 0x1000, 0x1800, 0x2000, 0x2800, 0x3000, 0x3800,
    0x4000, 0x4800, 0x5000, 0x5800, 0x6000, 0x6800, 0x7000, 0x7800,
    0x8000, 0x8800, 0x9000, 0x9800, 0xA000, 0xA800, 0xB000, 0xB800,
    0xC000, 0xC800, 0xD000, 0xD800, 0xE000, 0xE800, 0xF000, 0xF800,
    0x01E0, 0x03E0, 0x05E0, 0x07E0, 0x09E0, 0x0BE0, 0x0DE0, 0x0FE0,
    0x11E0, 0x13E0, 0x15E0, 0x17E0, 0x19E0, 0x1BE0, 0x1DE0, 0x1FE0 
};


Flame flame;
Plasma plasma;
MatrixRain matrix;
WaterRipple water;
RadarScan radar;










// ====================================================
// Setup
// ====================================================
void setup() {
  Serial.begin(115200);

  // Configure HUB75 pins
  HUB75_I2S_CFG::i2s_pins _pins = {
    R1_PIN,G1_PIN,B1_PIN,R2_PIN,G2_PIN,B2_PIN,
    A_PIN,B_PIN,C_PIN,D_PIN,E_PIN,LAT_PIN,OE_PIN,CLK_PIN
  };

  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);

  // Initialize display
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(100);
  dma_display->clearScreen();

randomSeed(analogRead(A0));

 

//putimage(0, 0, i1);

//putimagesize(1, 1, i1,  32);


putimage(0, 0, i20);
  
 inittime();
 initFlame(&flame);
 initPlasma(&plasma);
 initStarfield();
 initMatrixRain(&matrix);
 initWaterRipple( &water);
initParticleSystem();
  initRadarScan() ;
}

 

// Array of pointers to the image arrays
unsigned char *images[23] = {  i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16, i17, i18, i19, i20, i21, i22,i23};
float fade = 1.0f; // Start fully visible
 

void bubble(int q) {
    int bob = random(0, 23);  // Random index for 23 images (0 to 22)
    dma_display->clearScreen();

    if(q==0) putimage(0, 1, images[bob]);  // Display the selected image at (1, 1)

    if(q==1) { for(int i=0;i<50;i++ ) { putimageFade(0, 1,  images[bob],fade); fade -= 0.02f; 
                             delay(33);
                 }}

    if(q==2) {  
                    putimageColorMap(0, 1, images[bob] ,palette[random(1,16)]); 
                    
               }

    if(q==3) { putimageInvert(0, 1, images[bob]); }

    if(q!=1) delay(5000);
     
    dma_display->clearScreen();
}

 


// ====================================================
// Loop  
// ====================================================

 
 
unsigned long stateStartTime = 0;
const unsigned long showTimeDuration = 3UL * 60UL * 1000UL;  // 3 minutes
const unsigned long animInterval = 3UL * 60UL * 1000UL;    //  15 minutes
const unsigned long hourInterval = 58UL * 60UL * 1000UL;     // 60 minutes
unsigned long st =0;
uint8_t currentAnimation = 0;
bool showTime = false;  // Tracks whether to show time or animation

float Jets=0.0;

void loop() {
    unsigned long now = millis();

    // Check if it's time to show the clock (every hour)
    if (now - stateStartTime >= hourInterval) {
        showTime = true;  // Switch to showing time
        stateStartTime = now;  // Reset state start time
        currentAnimation = random(0, 25);  // Pick a new animation for after the time display
    }
    // Check if the 3-minute time display is over
    else if (showTime && now - stateStartTime >= showTimeDuration) {
        showTime = false;  // Switch back to animations
        stateStartTime = now;  // Reset state start time
    }
    // Check if it's time to change animation (every 15 minutes, only when not showing time)
    else if (!showTime && now - stateStartTime >= animInterval) {
        currentAnimation = random(0, 13);  // Pick a new animation
        stateStartTime = now;  // Reset state start time

        int i=random(0,3);
        bubble(i);  
        fade = 1.0f;
        //Jets=0.0f;
    }


    //currentAnimation=25;  //test

    // Execute the appropriate state
    if (showTime) {
        time(1, 15);  // Draw current time
    } else {
        switch (currentAnimation) {
            case 0:
                updateFlame(&flame);
                drawFlame(&flame);
                delay(33);
                break;

            case 1:
                updateParticleSystem();
                drawParticleSystem();
                delay(33);
                break;

            case 2:
                drawRadarScan();
                updateRadarScan();
                delay(33);
                break;

            case 3:
                updateMatrixRain(&matrix);
                drawMatrixRain(&matrix);
                delay(10);
                dma_display->clearScreen();
                break;

            case 4: 
                
             
                 matrixDigitalRain(millis());
               
                 delay(33);
                //dma_display->clearScreen();
                break;
                

            case 5: //////////////////
                 vortexEffect(millis());
                break;

            case 6:
                 
                 Rain(millis());
                //delay(10);
                //dma_display->clearScreen();
                break;

            case 7:
                
                starfieldEffect(millis());
                delay(10);
                break;

            case 8:
                 st = millis();
                while (millis() - st < 5000) pixelRainEffect(millis());
                delay(33);
                //dma_display->clearScreen();
                break;


            case 9:
                 st = millis();
                while (millis() - st < 8000) plasmaEffect(millis());
                delay(33);
                break;

            case 10:
                 st = millis();
                while (millis() - st < 1000) vortexEffect16(millis());
                //delay(10);
                break;

            case 11:
                  st = millis();
                  while (millis() - st < 5000) rain16(millis());
                  delay(33);
                  //dma_display->clearScreen();
                break;

            case 12:
                updateWaterRipple(&water);
                drawWaterRipple(&water);
                // Add some interactive ripples based on time
                static uint32_t last_interaction = 0;
                if (millis() - last_interaction > 2000) {
                    addRippleAt(&water, random(WIDTH), random(HEIGHT));
                    last_interaction = millis();
                }
                delay(33);
                break;

                 case 13://orange fall
                 fireAnimation(millis());
                 break;

                  case 14: 
                  
                    switchFractalPlasma(millis());
                  
                 break;

                  case 15:
                  fastFractalPlasma(millis());
                 break;

                 case 16:
                  
                 fastFractalMorph(millis());
                 break;

                 case 17:
                 vanGoghPaintAnimation(millis());
                 break;

                 case 18:
                   Jetstream( Jets);
                   Jets += 0.05;             
                   delay(10);
                 break;

                 case 19:
                 Pivotal(Jets);
                 Jets += 0.03;                  
                 break;

                 case 20:
                  st = millis();
                  while (millis() - st < 5000) {  Pillars(Jets);   Jets += 0.03; }
                                   
                 break;
                  
                 case 21:
                       RadialGlow(Jets); 
                       Jets += 0.03;
                  break;            
                       

                 case 22:
                       RaymarchGlow(Jets );
                       Jets += 0.03;
                                   
                 break;
                       
                 case 23:
                           SineChaosGlow(Jets );
                       Jets += 0.03;        
                 break;

                 case 24:
                 GooGlow (Jets );
                 Jets += 0.03;
                 break;

                 case 25:
                  
   
                 break;

                 case 26:                    
                 break;

                 case 27:                    
                 break;

                 case 28:                    
                 break;
                  


                 
                 

            default:
                // Fallback in case of invalid animation index
                time(1, 15);  // Draw current time
                break;
        }
    }
}

 
  
  
 
 
 
 

 



 


 
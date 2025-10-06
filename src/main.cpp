#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>
//#include <SPIFFS.h>
#include "gfx.h"
#include "firstboot.h" 
  


// Pin configuration - adjust these for your ESP32 setup
 
#define ANX 25+1
 

MatrixPanel_I2S_DMA *dma_display= nullptr; 

 
 
Flame  flame  ;
Plasma plasma  ;
MatrixRain matrix  ;

 
 

float fade = 1.0f; // Start fully visible
 
 
void bubble();
void pacman();


char buftext[256]={"\0"};


uint16_t palette[16] PROGMEM= {
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
uint16_t palette256[256]  PROGMEM= {
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





//////////////////////////////////////////////////////////////////
void fbootPIN() {   


    

 // Initialize first boot configuration
    int i=initFirstBoot();
    
   //Wait for configuration to complete
    while (!isConfigComplete()) {
        if(i==1) break;
          else handleFirstBoot();
     }

    if (dma_display != nullptr) {
        dma_display->clearScreen();
         delete dma_display;
        dma_display = nullptr;
        delay(100);
    }
    
    // Get configured pins
    MatrixPins matrixPins = getMatrixPins();
    
  
    // Create HUB75 configuration
    HUB75_I2S_CFG::i2s_pins _pins = matrixPins.getPinsConfig();
    HUB75_I2S_CFG mxconfig(WIDTH, HEIGHT, PANEL_CHAIN, _pins);
    
    // Initialize matrix
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);

    if (dma_display != nullptr) {
         
        dma_display->begin();
        dma_display->setBrightness8(100);
        dma_display->clearScreen();

    }

     
}
////////////////////////////////////////////////////////////////////

 

// ====================================================
// Setup
// ====================================================
void setup() {

   
 Serial.begin(115200);

 ///*
   if(checkFirstBoot()==true) {
 
  

  // Configure HUB75 pins
  HUB75_I2S_CFG::i2s_pins _pins = {
    R1_PIN,G1_PIN,B1_PIN,R2_PIN,G2_PIN,B2_PIN,
    A_PIN,B_PIN,C_PIN,D_PIN,E_PIN,LAT_PIN,OE_PIN,CLK_PIN
  };

   HUB75_I2S_CFG mxconfig(WIDTH, HEIGHT, PANEL_CHAIN, _pins);

   dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  // Initialize display
  if (dma_display != nullptr) {
        dma_display->begin();
        dma_display->setBrightness8(100);
        dma_display->clearScreen();

  
  dma_display->drawRect(0,0,WIDTH,HEIGHT,0x07E0);
  dma_display->println("        ");
  dma_display->println("Wifi:ESP32 ");
  dma_display->println(" http://19 2.168.4.1 ");
  dma_display->println("  Button  ");
  dma_display->println(" 2x reset ");

  
  }
  delay(100);
 }
//*/
 

////////////////////////////////////////////////////////////////////
//  resetConfig();
////////////////////////////if wired fuck reset///////////////////
 initTextBuffer();//init buffer first*************************
 clearTextBuffer(); 

  
 fbootPIN();

 
 
 //logo(); 
   
   

 inittime();
 initFlame(&flame);
 initPlasma(&plasma);
 initStarfield();
 initMatrixRain(&matrix);
  initParticleSystem();
  initAsteroids();
 initSupercharged();
 
  
 //if(random(0,4)==1) proverbe( );
 
 
}

 

// ====================================================
// Loop  
// ====================================================
unsigned long stateStartTime = 0;
const unsigned long showTimeDuration = 2UL * 60UL * 1000UL;  // 2 minutes
 const unsigned long animInterval = 4UL * 60UL * 1000UL;    //  4 minutes
//const unsigned long animInterval = 2000;    //test
const unsigned long hourInterval = 58UL * 60UL * 1000UL;     // 58 minutes
////////////////////////////////////////////////////////////////////////




unsigned long st =0;
uint8_t currentAnimation =   random(0,ANX);
bool showTime = false;  // Tracks whether to show time or animation
bool hasShownThisHour = false;  // Flag to prevent showing multiple times per hour

float Jets=0.0f;
 // Get current time
 struct tm timeinfo;

void loop() {
    unsigned long now = millis();


   
 //  checkResetButton(); ///fast double click reset

   
    if (!getLocalTime(&timeinfo)) {
        // If we can't get time, fall back to original behavior
        if (now - stateStartTime >= 60UL * 60UL * 1000UL) { // 60 minutes
            showTime = true;
            stateStartTime = now;
            currentAnimation = (currentAnimation + 1) % ANX; 
            hasShownThisHour = false;

           
             
        }
    } else {
        // Check if it's a new hour (minutes are 0 and we haven't shown time this hour)
        if (timeinfo.tm_min == 59 && !hasShownThisHour) {
            showTime = true;
            stateStartTime = now;
            currentAnimation = (currentAnimation + 1) % ANX; 

            hasShownThisHour = true;
            
        }
        
        // Reset the flag when we're past minute 0
        if (timeinfo.tm_min != 59) {
            hasShownThisHour = false;
        }
    }
    
    // Check if the 3-minute time display is over
    if (showTime && now - stateStartTime >= showTimeDuration) {
        showTime = false;  // Switch back to animations
        stateStartTime = now;  // Reset state start time
    }
    
    // Check if it's time to change animation (every 3 minutes, only when not showing time)
    if (!showTime && now - stateStartTime >= animInterval) {
        currentAnimation = (currentAnimation + 1) % ANX; 
        stateStartTime = now;  // Reset state start time
        
       
       // bubble();
       // if(random(0,4)==2) bubble();

       // if(random(0,4)==2) pacman();
        
        fade = 1.0f;
         Jets = 0.0f;
    }


/////////////////////////////
//  currentAnimation =0;  //test
///////////////////////////



    // Execute the appropriate state
    if (showTime) {
        timeE(1, 18);  // Draw current time
        dma_display->clearScreen();
    } else {
        switch (currentAnimation) {

            case 24:
            
            
             glob();
             delay(2);
            break;

             

            case 0:
             
                updateFlame(&flame);
                drawFlame(&flame);
                delay(11);
                break;

            case 1:
                 updateParticleSystem();
                 drawParticleSystem();
                delay(33);
                break;

            case 2:
                GooGlow(Jets );
                 Jets += 0.03;
                // pageFlip();
                break;

            case 3:
                 updateMatrixRain(&matrix);
                drawMatrixRain(&matrix);
                delay(10);
                //dma_display->clearScreen();
                break;

            case 4: 
                
              TronMatrixPulse(Jets );
              Jets += 0.03;     
              delay(2);   
               
                
                break;
                

            case 5: //////////////////
                       RaymarchGlow(Jets );
                       Jets += 0.03;
                      
                       delay(5);

                break;

            case 6:
                 
                // 
                 rain16(millis());
                //delay(10);
                //dma_display->clearScreen();
                break;

            case 7:
                
                starfieldEffect(millis());
                delay(10);
                break;

            case 8:
                 
                 pixelRainEffect(millis());
                delay(3);
                //dma_display->clearScreen();
                break;


            case 9:
                 
                plasmaEffect(millis());
                
                break;

            case 10:///////////////////////////////
                  Rain(millis());
                break;

            case 11:
                  
                   RadialGlow(millis()); 
                   break;

               case 12://////////////////////////////////////
                
                 Pillars(Jets);   
                 Jets += 0.03;
                break;

                 case 13://orange fall
                 fireAnimation(millis());
                 break;

                  case 14:  //?
                     switchFractalPlasma(millis());
                       break;

                  case 15:
                  fastFractalPlasma(millis());
                 break;

                 case 16:
                  
                 fastFractalMorph(millis());
                 break;

                 case 17:
                  
                 vanGoghPaintAnimation(millis() );
                  
                 break;

                 case 18:
                   Jetstream( Jets);
                   Jets += 0.05;             
                    
                 break;

                 case 19:
                 Pivotal(Jets);//////////////////to delete
                 Jets += 0.07;                  
                 break;

                 case 20:
               
                    glob2();                
                 break;
                  
                 case 21:
                       
                  
                    draw3DLineAnimations(millis() );
                  break;            
     
                  case 22:
                      asteroid();
                  break; 

                  case 23:
                   updateSupercharged();
                  break; 
                  case 25:
                  proverbe( );
                  currentAnimation=0;
                  break; 
                 

            default:
                // Fallback in case of invalid animation index
                timeE(1, 18);  // Draw current time
                dma_display->clearScreen();
                break;
        }
    }
}

 
  
  ///////////////////////////////////////////////////////////////////////


 
void bubble() {

    unsigned char *images[] PROGMEM= {  i1, i2, i3, i4, i5, i6, i7, i8, i9, i10  };
    int bob = random(0, 11);  // Random index  image.cpp
    //dma_display->clearScreen();
     clearTextBuffer(); 
    int q=0;

    if(q==0) putimage(0, 0, images[bob]);  // Display the selected image at (1, 1)

   

    if(q==1) {  
               putimageColorMap(0, 0, images[bob] ,palette[random(1,16)]); 
                    
               }

 
     pageFlip();
    if(q!=1) delay(7000);
     
   // dma_display->clearScreen();
   currentAnimation = (currentAnimation + 1) % ANX; 
}



void pacman() {
    int bob = random(0, 11); // Random image index
    

    unsigned char *images[] PROGMEM= {  i1, i2, i3, i4, i5, i6, i7, i8, i9, i10  };

   
    clearTextBuffer(); 
    //dma_display->clearScreen();
        
      for(int x=-10;x<100;x++) {
        int ja=random(0,3);
        putimagesize(x, 20+ja, images[bob], 32);
        pageFlip();
        clearTextBuffer(); 
        
       
    }
    currentAnimation = (currentAnimation + 1) % ANX; 
    
}


 
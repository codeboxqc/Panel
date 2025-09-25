#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>
#include <HTTPClient.h>
#include <time.h>
 #include "gfx.h"

// -------------------- Config --------------------
const char* ntpServer = "pool.ntp.org";
const char* timezone  = "EST5EDT,M3.2.0,M11.1.0"; // New York time
 char  ssid[64]      = "user";
 char  password[64]  = "password";

 

 


 const unsigned char z0[49] PROGMEM= {
  0,1,1,1,1,1,0,
  1,0,0,0,0,0,1,
  1,0,0,0,0,0,1,
  1,0,0,0,0,0,1,
  1,0,0,0,0,0,1,
  1,0,0,0,0,0,1,
  0,1,1,1,1,1,0
};


 const unsigned char z1[49]  PROGMEM={   
                        0,0,0,1,0,0,0,
                        0,0,1,1,0,0,0,
                        0,1,0,1,0,0,0,
                        0,0,0,1,0,0,0,
                        0,0,0,1,0,0,0,
                        0,0,0,1,0,0,0,
                        0,1,1,1,1,1,0
};


 const unsigned char z2[49]  PROGMEM={    
                        0,0,1,1,1,1,0,
                        0,1,0,0,0,1,0,
                        0,0,0,0,1,0,0,
                        0,0,0,1,0,0,0,
                        0,0,1,0,0,0,0,
                        0,1,0,0,0,0,0,
                        1,1,1,1,1,1,0
};

 const unsigned char z3[49]  PROGMEM={    
                        0,0,1,1,1,1,0,
                        0,1,0,0,0,0,1,
                        0,0,0,0,0,1,0,
                        0,0,0,0,1,0,0,
                        0,0,0,0,0,1,0,
                        0,1,0,0,0,0,1,
                        0,0,1,1,1,1,0
};



 const unsigned char z4[49]  PROGMEM={    
                        0,0,0,1,0,0,0,
                        0,0,1,1,0,0,0,
                        0,1,0,1,0,0,0,
                        1,0,0,1,0,0,0,
                        1,1,1,1,1,1,0,
                        0,0,0,1,0,0,0,
                        0,0,0,1,0,0,0
};



 const unsigned char z5[49]  PROGMEM={    
                        0,1,1,1,1,1,0,
                        0,1,0,0,0,0,0,
                        0,1,0,0,0,0,0,
                        0,1,1,1,1,1,0,
                        0,0,0,0,0,0,1,
                        0,0,0,0,0,0,1,
                        0,0,1,1,1,1,0
};

 const unsigned char z6[49]  PROGMEM={    
                        0,0,0,1,1,1,0,
                        0,0,1,0,0,0,0,
                        0,1,0,0,0,0,0,
                        0,1,1,1,1,1,0,
                        0,1,0,0,0,1,0,
                        0,1,0,0,0,1,0,
                        0,0,1,1,1,0,0
};


 const unsigned char z7[49]  PROGMEM={    
                        0,1,1,1,1,1,1,
                        0,0,0,0,0,1,0,
                        0,0,0,0,1,0,0,
                        0,0,0,1,0,0,0,
                        0,0,1,0,0,0,0,
                        0,1,0,0,0,0,0,
                        1,0,0,0,0,0,0
};


 const unsigned char z8[49]  PROGMEM={    
                       0,1,1,1,1,1,0,
                        1,0,0,0,0,0,1,
                        0,1,0,0,0,1,0,
                        0,0,1,1,1,0,0,
                        0,1,0,0,0,1,0,
                        1,0,0,0,0,0,1,
                        0,1,1,1,1,1,0
};


 const unsigned char z9[49]  PROGMEM={    
                       0,0,1,1,1,1,0,
                        0,1,0,0,0,0,1,
                        0,1,0,0,0,0,1,
                        0,0,1,1,1,1,1,
                        0,0,0,0,0,0,1,
                        0,0,0,0,0,0,1,
                        0,0,0,0,0,0,1
};



  

 


// -------------------- WiFi --------------------
bool connectWiFi() {


 
   

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 4) {
    delay(300);
    //Serial.print(".");
    attempts++;
  }
  return (WiFi.status() == WL_CONNECTED);
}

// -------------------- Time Sync --------------------
bool getTimeFromHTTP() {
  if (WiFi.status() != WL_CONNECTED) return false;
  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/timezone/America/New_York");
  int httpCode = http.GET();
  bool success = false;
  if (httpCode == 200) {
    String payload = http.getString();
    int idx = payload.indexOf("\"datetime\":\"");
    if (idx > 0) {
      String datetime = payload.substring(idx + 12, idx + 31);
      struct tm tm;
      if (strptime(datetime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm)) {
        timeval now = { mktime(&tm), 0 };
        settimeofday(&now, NULL);
        success = true;
      }
    }
  }
  http.end(); // Always release resources
  return success;
}


bool syncTime() {
  configTzTime(timezone, ntpServer);

  struct tm timeinfo;
  unsigned long start = millis();
  const unsigned long timeout = 1000; // 1 second max wait

  while (!getLocalTime(&timeinfo) && millis() - start < timeout) {
    delay(10); // small non-blocking wait
  }

  if (getLocalTime(&timeinfo)) {
    return true;
  }

  // Fallback to HTTP if NTP fails
  return getTimeFromHTTP();
}


// -------------------- Draw helpers --------------------
 
void drawDigit(int digit, int x, int y, uint16_t color, uint16_t bg) {
    if (digit < 0 || digit > 9) return;

    const unsigned char* digits[10] = { z0, z1, z2, z3, z4, z5, z6, z7, z8, z9 };

    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 7; col++) {
            int idx = row * 7 + col;
            int screenX = x + col;
            int screenY = y + row;

            if (digits[digit][idx] == 1)
             
                dma_display->drawPixel(screenX, screenY, color);
            else
                dma_display->drawPixel(screenX, screenY, bg);
        }
    }
}


 void drawColon(int x, int y, bool on, uint16_t color, uint16_t bg) {
  // 2 dots, 1 above 1 below
  if (on) {
    dma_display->drawPixel(x, y,     0xF800); //red
    dma_display->drawPixel(x, y + 4, 0xF800); 
  } else {
    dma_display->drawPixel(x, y,     bg);
    dma_display->drawPixel(x, y + 4, bg);
  }
}

// -------------------- Show time --------------------
void showtime(int xx, int yy, uint16_t color) {
    struct tm timeinfo={};

    memset(&timeinfo, 0, sizeof(timeinfo));

    if (!getLocalTime(&timeinfo)) {
        //Serial.println("No time yet!");
        return;
    }

    int hour   = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int second = timeinfo.tm_sec;

    // Clear screen each second
    //dma_display->clearScreen();

    // Base positions
    int x = 2 + xx;
    int y = 10 + yy;

    int digitWidth = 7;  // width of each digit
    int gap = 2;         // space between digits
    int spacing = digitWidth + gap;

    int px = x;  // current x position

    // Hours
    drawDigit(hour / 10, px, y, color, 0x0000);
    px += spacing;
    drawDigit(hour % 10, px, y, color, 0x0000);
    px += spacing;

    // Colon
    drawColon(px, y + 2, (second % 2) == 0, color, 0x0000);
    px += gap + 1;  // small extra gap after colon

    // Minutes
    drawDigit(minute / 10, px, y, color, 0x0000);
    px += spacing;
    drawDigit(minute % 10, px, y, color, 0x0000);
    px += spacing;

    // Colon
    drawColon(px, y + 2, (second % 2) == 0, color, 0x0000);
    px += gap + 1;

    // Seconds
    drawDigit(second / 10, px, y, 0x4208, 0x0000);//grey
    px += spacing;
    drawDigit(second % 10, px, y, 0x4208, 0x0000);//grey
}

// -------------------- External API --------------------
void inittime() {

   
  if (connectWiFi())  syncTime();
   
}

void time(int x,int y) {   // your external loop
  
  

  showtime(x,y,0x07E0);
  
}
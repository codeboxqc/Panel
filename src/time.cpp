#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>
#include <HTTPClient.h>
#include <time.h>
 #include "gfx.h"

// -------------------- Config --------------------
const char* ntpServer = "pool.ntp.org";
const char* timezone  = "EST5EDT,M3.2.0,M11.1.0"; // New York time
 char* ssid      = "class";
 char* password  = "class";

 String ssidGlob;
 String  passwordGlob;

int Tok=0;


 unsigned char z0[] = {
  0,1,1,1,1,1,0,
  1,0,0,0,0,0,1,
  1,0,0,0,0,0,1,
  1,0,0,0,0,0,1,
  1,0,0,0,0,0,1,
  1,0,0,0,0,0,1,
  0,1,1,1,1,1,0
};


unsigned char z1[]={    0,0,0,1,0,0,0,
                        0,0,1,1,0,0,0,
                        0,1,0,1,0,0,0,
                        0,0,0,1,0,0,0,
                        0,0,0,1,0,0,0,
                        0,0,0,1,0,0,0,
                        0,1,1,1,1,1,0
};


unsigned char z2[]={    0,0,1,1,1,1,0,
                        0,1,0,0,0,1,0,
                        0,0,0,0,1,0,0,
                        0,0,0,1,0,0,0,
                        0,0,1,0,0,0,0,
                        0,1,0,0,0,0,0,
                        1,1,1,1,1,1,0
};

unsigned char z3[]={    0,0,1,1,1,1,0,
                        0,1,0,0,0,0,1,
                        0,0,0,0,0,1,0,
                        0,0,0,0,1,0,0,
                        0,0,0,0,0,1,0,
                        0,1,0,0,0,0,1,
                        0,0,1,1,1,1,0
};



unsigned char z4[]={    0,0,0,1,0,0,0,
                        0,0,1,1,0,0,0,
                        0,1,0,1,0,0,0,
                        1,0,0,1,0,0,0,
                        1,1,1,1,1,1,0,
                        0,0,0,1,0,0,0,
                        0,0,0,1,0,0,0
};



unsigned char z5[]={    0,1,1,1,1,1,0,
                        0,1,0,0,0,0,0,
                        0,1,0,0,0,0,0,
                        0,1,1,1,1,1,0,
                        0,0,0,0,0,0,1,
                        0,0,0,0,0,0,1,
                        0,0,1,1,1,1,0
};

unsigned char z6[]={    0,0,0,1,1,1,0,
                        0,0,1,0,0,0,0,
                        0,1,0,0,0,0,0,
                        0,1,1,1,1,1,0,
                        0,1,0,0,0,1,0,
                        0,1,0,0,0,1,0,
                        0,0,1,1,1,0,0
};


unsigned char z7[]={    0,1,1,1,1,1,1,
                        0,0,0,0,0,1,0,
                        0,0,0,0,1,0,0,
                        0,0,0,1,0,0,0,
                        0,0,1,0,0,0,0,
                        0,1,0,0,0,0,0,
                        1,0,0,0,0,0,0
};


unsigned char z8[]={    0,1,1,1,1,1,0,
                        1,0,0,0,0,0,1,
                        0,1,0,0,0,1,0,
                        0,0,1,1,1,0,0,
                        0,1,0,0,0,1,0,
                        1,0,0,0,0,0,1,
                        0,1,1,1,1,1,0
};


unsigned char z9[]={    0,0,1,1,1,1,0,
                        0,1,0,0,0,0,1,
                        0,1,0,0,0,0,1,
                        0,0,1,1,1,1,1,
                        0,0,0,0,0,0,1,
                        0,0,0,0,0,0,1,
                        0,0,0,0,0,0,1
};


 unsigned char* digits[10] = { z0, z1, z2, z3, z4, z5, z6, z7, z8, z9 };


 


// -------------------- WiFi --------------------
bool connectWiFi() {




  if (ssidGlob.length() == 0)   return false;

  // Convert String to char*
  ssid =     (char*)  ssidGlob.c_str();     
  password = (char*)  passwordGlob.c_str();

  
   

  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
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
  if (httpCode == 200) {
    String payload = http.getString();
    int idx = payload.indexOf("\"datetime\":\"");
    if (idx > 0) {
      String datetime = payload.substring(idx + 12, idx + 31); // 2025-09-03T14:22:05
      struct tm tm;
      if (strptime(datetime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm)) {
        timeval now = { mktime(&tm), 0 };
        settimeofday(&now, NULL);
        http.end();
        return true;
      }
    }
  }
  http.end();
  return false;
}

bool syncTime() {
  configTzTime(timezone, ntpServer);
  delay(1000);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    return true;
  } else {
    return getTimeFromHTTP();
  }
}

// -------------------- Draw helpers --------------------
 
void drawDigit(int digit, int x, int y, uint16_t color, uint16_t bg) {
    if (digit < 0 || digit > 9) return;
    unsigned char* d = digits[digit];

    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 7; col++) {
            int idx = row * 7 + col;
            int screenX = x + col;
            int screenY = y + row;

            if (d[idx] == 1)
             
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
    struct tm timeinfo;

    memset(&timeinfo, 0, sizeof(timeinfo));

    if (!getLocalTime(&timeinfo)) {
        Serial.println("No time yet!");
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

   
 

  if (connectWiFi()) {
     
    syncTime();
  } else {
    
  }
}

void time(int x,int y) {   // your external loop
  
  
  // drawDigit(0, 10+(7*1), 10, 0x07E0, 0x0000);
  // drawDigit(1, 11+(7*2), 10, 0x07E0, 0x0000);
  // drawDigit(2, 12+(7*3), 10, 0x07E0, 0x0000);
  // drawDigit(3, 13+(7*4), 10, 0x07E0, 0x0000);
  // drawDigit(4, 15+(7*5), 10, 0x07E0, 0x0000);
  // drawDigit(5, 16+(7*6), 10, 0x07E0, 0x0000);
  // drawDigit(6, 10+(7*1), 10+(7*2), 0x07E0, 0x0000);
  // drawDigit(7, 11+(7*2), 11+(7*2), 0x07E0, 0x0000);
  // drawDigit(8, 13+(7*3), 12+(7*2), 0x07E0, 0x0000);
  // drawDigit(9, 10, 10, 0x07E0, 0x0000);


  showtime(x,y,0x07E0);
  // delay(30); // update once per second
 // dma_display->clearScreen();
}
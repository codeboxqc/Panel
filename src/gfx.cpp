#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <vector>
#include <math.h>

#include "gfx.h"
 

using namespace std;

 



struct RGB {
    uint8_t r, g, b;
};

// Helper to find closest palette color
uint8_t findClosestColor(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t bestIndex = 0;
    float bestDistance = 1e9;
    for (int i = 0; i < 16; i++) {
        uint16_t rgb565 = palette[i];
        uint8_t pr = ((rgb565 >> 11) & 0x1F) << 3; // Scale 5-bit to 8-bit
        uint8_t pg = ((rgb565 >> 5) & 0x3F) << 2;  // Scale 6-bit to 8-bit
        uint8_t pb = (rgb565 & 0x1F) << 3;         // Scale 5-bit to 8-bit
        float dist = sqrt(pow(r - pr, 2) + pow(g - pg, 2) + pow(b - pb, 2));
        if (dist < bestDistance) {
            bestDistance = dist;
            bestIndex = i;
        }
    }
    return bestIndex;
}
 

// ====================================================
// 4bpp pixel helpers
// ====================================================
// Existing functions (unchanged)
uint8_t getPixel(const unsigned char* buf, int idx) {
    return (idx & 1) ? (buf[idx >> 1] >> 4) & 0x0F : buf[idx >> 1] & 0x0F;
}

void setPixel(unsigned char* buf, int idx, uint8_t color) {
    color &= 0x0F;
    if(idx & 1) buf[idx >> 1] = (buf[idx >> 1] & 0x0F) | (color << 4);
    else buf[idx >> 1] = (buf[idx >> 1] & 0xF0) | color;
}

// ====================================================
// Normal 64x64 image
// ====================================================
void putimage(int px, int py, const unsigned char* img) {
    for(int y=0;y<64;y++){
        int screenY = py + y;
        if(screenY<0 || screenY>=PANEL_RES_Y) continue;

        for(int x=0;x<64;x++){
            int screenX = px + x;
            if(screenX<0 || screenX>=PANEL_RES_X) continue;

            int idx = y*64 + x;
            uint8_t col = getPixel(img, idx);
            if(col!=0) dma_display->drawPixel(screenX, screenY, palette[col]);
        }
    }
}

// ====================================================
// Resized image (16x16 or 32x32)
// ====================================================
// Updated putimagesize with RGB averaging for better color accuracy
void putimagesize(int px, int py, const unsigned char* img, int size) {
    if (size <= 0 || size > PANEL_RES_Y) return;
    for (int y = 0; y < size; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >= PANEL_RES_Y) continue;
        for (int x = 0; x < size; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= PANEL_RES_X) continue;
            // Calculate block of pixels to average
            float scale = 64.0f / size;
            int startX = (int)(x * scale);
            int startY = (int)(y * scale);
            int endX = (int)((x + 1) * scale);
            int endY = (int)((y + 1) * scale);
            if (endX > 64) endX = 64;
            if (endY > 64) endY = 64;

            // Average RGB values
            uint32_t rSum = 0, gSum = 0, bSum = 0;
            uint32_t count = 0;
            for (int sy = startY; sy < endY; sy++) {
                for (int sx = startX; sx < endX; sx++) {
                    int idx = sy * 64 + sx;
                    uint8_t col = getPixel(img, idx);
                    uint16_t rgb565 = palette[col];
                    
                    rSum += ((rgb565 >> 11) & 0x1F) << 3; // Scale to 8-bit
                    gSum += ((rgb565 >> 5) & 0x3F) << 2;
                    bSum += (rgb565 & 0x1F) << 3;
                    count++;
                    
                }
            }
            uint8_t avgR = count > 0 ? rSum / count : 0;
            uint8_t avgG = count > 0 ? gSum / count : 0;
            uint8_t avgB = count > 0 ? bSum / count : 0;
            uint8_t col = findClosestColor(avgR, avgG, avgB);
            if (col != 0  ) dma_display->drawPixel(screenX, screenY, palette[col]);
        }
    }
}

void putimagesize2(int px, int py, const unsigned char* img, int size) {
    if (size <= 0 || size > PANEL_RES_Y) return;
    for (int y = 0; y < size; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >= PANEL_RES_Y) continue;
        for (int x = 0; x < size; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= PANEL_RES_X) continue;
            // Use 3x3 block for 20x20 (approx 3.2x3.2)
            float scale = 64.0f / size;
            int startX = (int)(x * scale);
            int startY = (int)(y * scale);
            int endX = startX + 3; // Fixed 3x3 block
            int endY = startY + 3;
            if (endX > 64) endX = 64;
            if (endY > 64) endY = 64;
            uint32_t rSum = 0, gSum = 0, bSum = 0, count = 0;
            for (int sy = startY; sy < endY; sy++) {
                for (int sx = startX; sx < endX; sx++) {
                    int idx = sy * 64 + sx;
                    uint8_t col = getPixel(img, idx);
                    if (col != 0) { // Exclude black pixels
                        uint16_t rgb565 = palette[col];
                        rSum += ((rgb565 >> 11) & 0x1F) << 3;
                        gSum += ((rgb565 >> 5) & 0x3F) << 2;
                        bSum += (rgb565 & 0x1F) << 3;
                        count++;
                    }
                }
            }
            uint8_t col = (count > 0) ? findClosestColor(rSum / count, gSum / count, bSum / count) : 0;
            // Debug output for first few pixels
            if (x < 5 && y == 0 && count > 0) {
                Serial.print("Pixel ("); Serial.print(x); Serial.print(",0): RGB=");
                Serial.print(rSum/count); Serial.print(",");
                Serial.print(gSum/count); Serial.print(",");
                Serial.print(bSum/count); Serial.print(" -> col="); Serial.println(col);
            }
            if (col != 0) dma_display->drawPixel(screenX, screenY, palette[col]);
        }
    }
}

// ====================================================
// Color swap for non-zero pixels
// ====================================================
void putimageColorMap(int px, int py, const unsigned char* img, int color){
    for(int y=0;y<64;y++){
        int screenY = py + y;
        if(screenY<0 || screenY>=PANEL_RES_Y) continue;

        for(int x=0;x<64;x++){
            int screenX = px + x;
            if(screenX<0 || screenX>=PANEL_RES_X) continue;

            int idx = y*64 + x;
            uint8_t col = getPixel(img, idx);
            if(col!=0) dma_display->drawPixel(screenX, screenY, palette[color]);
        }
    }
}

// ====================================================
// Fade in/out image
// fade = 0.0 (black) -> 1.0 (full brightness)
// ====================================================
void putimageFade(int px, int py, const unsigned char* img, float fade){
    if(fade<0.0f) fade=0.0f;
    if(fade>1.0f) fade=1.0f;

    for(int y=0;y<64;y++){
        int screenY = py + y;
        if(screenY<0 || screenY>=PANEL_RES_Y) continue;

        for(int x=0;x<64;x++){
            int screenX = px + x;
            if(screenX<0 || screenX>=PANEL_RES_X) continue;

            int idx = y*64 + x;
            uint8_t col = getPixel(img, idx);
            if(col!=0){
                uint16_t r = ((palette[col] >> 11) & 0x1F) * fade;
                uint16_t g = ((palette[col] >> 5) & 0x3F) * fade;
                uint16_t b = (palette[col] & 0x1F) * fade;
                uint16_t color = (r<<11) | (g<<5) | b;
                dma_display->drawPixel(screenX, screenY, color);
            }
        }
    }
}


void putimageRotate(int px, int py, const unsigned char* img, int degrees) {
    degrees = degrees % 360;
    int rot = degrees / 90; // 0=0°, 1=90°, 2=180°, 3=270°
    for (int y = 0; y < 64; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >= PANEL_RES_Y) continue;
        for (int x = 0; x < 64; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= PANEL_RES_X) continue;
            int origX = x, origY = y;
            switch (rot) {
                case 1: origX = 63 - y; origY = x; break;      // 90°
                case 2: origX = 63 - x; origY = 63 - y; break; // 180°
                case 3: origX = y; origY = 63 - x; break;      // 270°
            }
            int idx = origY * 64 + origX;
            uint8_t col = getPixel(img, idx);
            if (col != 0) dma_display->drawPixel(screenX, screenY, palette[col]);
        }
    }
}


void putimageInvert(int px, int py, const unsigned char* img) {
    for (int y = 0; y < 64; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >= PANEL_RES_Y) continue;
        for (int x = 0; x < 64; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= PANEL_RES_X) continue;
            int idx = y * 64 + x;
            uint8_t col = getPixel(img, idx);
            if (col != 0) {
                uint8_t invCol = 15 - col; // Simple inversion (0<->15, 1<->14, etc.)
                dma_display->drawPixel(screenX, screenY, palette[invCol]);
            }
        }
    }
}


void putimageBlend(int px, int py, const unsigned char* img1, const unsigned char* img2, float blend) { // blend=0: img1, 1: img2
    blend = constrain(blend, 0.0f, 1.0f);
    for (int y = 0; y < 64; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >= PANEL_RES_Y) continue;
        for (int x = 0; x < 64; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= PANEL_RES_X) continue;
            int idx = y * 64 + x;
            uint8_t col1 = getPixel(img1, idx);
            uint8_t col2 = getPixel(img2, idx);
            if (col1 != 0 || col2 != 0) {
                uint8_t blendedCol = (uint8_t)(col1 * (1 - blend) + col2 * blend);
                dma_display->drawPixel(screenX, screenY, palette[blendedCol]);
            }
        }
    }
}




void putimageCycle(int px, int py, const unsigned char* img, int offset) {
    for (int y = 0; y < 64; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >= PANEL_RES_Y) continue;

        for (int x = 0; x < 64; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= PANEL_RES_X) continue;

            int idx = y * 64 + x;
            uint8_t col = getPixel(img, idx);
            if (col != 0) {
                int cycledCol = (col + offset) % 16;
                dma_display->drawPixel(screenX, screenY, palette[cycledCol]);
            }
        }
    }
}


// convert 64x64 image to 32x32
unsigned char* convertimagesize(int px, int py, const unsigned char* img, int size) {
    if (size != 32) return nullptr; // Only support 32x32 output for now
    unsigned char* output = new unsigned char[512];
    memset(output, 0, 512);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            int origX = x * 2, origY = y * 2;
            int idx1 = origY * 64 + origX;
            int idx2 = origY * 64 + (origX + 1);
            int idx3 = (origY + 1) * 64 + origX;
            int idx4 = (origY + 1) * 64 + (origX + 1);
            uint8_t col1 = getPixel(img, idx1);
            uint8_t col2 = getPixel(img, idx2);
            uint8_t col3 = getPixel(img, idx3);
            uint8_t col4 = getPixel(img, idx4);
            uint8_t avgCol = (col1 + col2 + col3 + col4) / 4;
            int outIdx = y * 32 + x;
            setPixel(output, outIdx, avgCol);
        }
    }
    return output;
}










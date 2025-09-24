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
        if(screenY<0 || screenY>=HEIGHT) continue;

        for(int x=0;x<64;x++){
            int screenX = px + x;
            if(screenX<0 || screenX>=WIDTH) continue;

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
    if (size <= 0 || size > HEIGHT) return;
    for (int y = 0; y < size; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >=HEIGHT) continue;
        for (int x = 0; x < size; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= WIDTH) continue;
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

 
 
 
// ====================================================
// Color swap for non-zero pixels
// ====================================================
void putimageColorMap(int px, int py, const unsigned char* img, int color){
    for(int y=0;y<64;y++){
        int screenY = py + y;
        if(screenY<0 || screenY>=HEIGHT) continue;

        for(int x=0;x<64;x++){
            int screenX = px + x;
            if(screenX<0 || screenX>=WIDTH) continue;

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
        if(screenY<0 || screenY>=HEIGHT) continue;

        for(int x=0;x<64;x++){
            int screenX = px + x;
            if(screenX<0 || screenX>=WIDTH) continue;

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

 

void putimageInvert(int px, int py, const unsigned char* img) {
    for (int y = 0; y < 64; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >= HEIGHT) continue;
        for (int x = 0; x < 64; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= WIDTH) continue;
            int idx = y * 64 + x;
            uint8_t col = getPixel(img, idx);
            if (col != 0) {
                uint8_t invCol = 15 - col; // Simple inversion (0<->15, 1<->14, etc.)
                dma_display->drawPixel(screenX, screenY, palette[invCol]);
            }
        }
    }
}

 
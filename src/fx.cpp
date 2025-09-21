#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "gfx.h"




#define WIDTH 64
#define HEIGHT 64




void plasmaAnimation(unsigned long time) {
    static unsigned long lastUpdate = 0;
    static float offset = 0.0;
    
    if (time - lastUpdate > 30) { // Update every 30ms for smooth animation
        lastUpdate = time;
        offset += 0.02; // Increment for animation
        
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                // Calculate plasma values using multiple sine waves
                float value1 = sin((x * 0.1) + offset);
                float value2 = sin((y * 0.1) + offset * 0.7);
                float value3 = sin((x * 0.15 + y * 0.1) + offset * 1.3);
                float value4 = sin(sqrt((x - WIDTH/2) * (x - WIDTH/2) + 
                                       (y - HEIGHT/2) * (y - HEIGHT/2)) * 0.1 + offset);
                
                // Combine the values
                float plasma = (value1 + value2 + value3 + value4) * 0.25;
                
                // Map to HSV color space for vibrant colors
                float hue = fmod(plasma * 2.0 + offset, 1.0);
                
                // Convert HSV to RGB
                float r, g, b;
                hue *= 6.0;
                int i = (int)hue;
                float f = hue - i;
                float p = 0.0;
                float q = 1.0 - f;
                float t = f;
                
                switch (i % 6) {
                    case 0: r = 1.0; g = t; b = p; break;
                    case 1: r = q; g = 1.0; b = p; break;
                    case 2: r = p; g = 1.0; b = t; break;
                    case 3: r = p; g = q; b = 1.0; break;
                    case 4: r = t; g = p; b = 1.0; break;
                    case 5: r = 1.0; g = p; b = q; break;
                }
                
                // Scale to 0-255 and draw pixel
                uint8_t red = (uint8_t)(r * 255);
                uint8_t green = (uint8_t)(g * 255);
                uint8_t blue = (uint8_t)(b * 255);
                
                dma_display->drawPixel(x, y, dma_display->color565(red, green, blue));
            }
        }
    }
}


void fireAnimation(unsigned long time) {
    static uint16_t heat[WIDTH][HEIGHT];
    static unsigned long lastUpdate = 0;
    static unsigned long lastSpread = 0;
    
    if (time - lastUpdate > 50) {
        lastUpdate = time;
        
        // Generate rising heat
        for (int x = 0; x < WIDTH; x++) {
            // Add random heat to the bottom row
            heat[x][HEIGHT-1] = random(150, 256);
            
            // Make the heat rise and cool
            for (int y = 0; y < HEIGHT; y++) {
                // Heat rises and cools
                if (y > 0) {
                    heat[x][y-1] = constrain(heat[x][y] - random(10, 30), 0, 255);
                }
                
                // Map heat to color
                uint8_t intensity = heat[x][y];
                if (intensity > 0) {
                    uint8_t r = constrain(intensity * 2, 0, 255);
                    uint8_t g = constrain(intensity * 0.8, 0, 255);
                    uint8_t b = constrain(intensity * 0.2, 0, 200);
                    dma_display->drawPixel(x, HEIGHT-1-y, dma_display->color565(r, g, b));
                } else {
                    dma_display->drawPixel(x, HEIGHT-1-y, 0);
                }
            }
        }
        
        // Occasionally add bigger flames
        if (random(100) < 10) {
            int x = random(WIDTH);
            for (int i = -1; i <= 1; i++) {
                int fx = constrain(x + i, 0, WIDTH-1);
                heat[fx][HEIGHT-1] = random(200, 256);
            }
        }
    }
    
    // Make the fire spread sideways occasionally
    if (time - lastSpread > 100) {
        lastSpread = time;
        
        for (int x = 1; x < WIDTH-1; x++) {
            for (int y = HEIGHT-2; y < HEIGHT; y++) {
                if (heat[x][y] > 100) {
                    // Spread heat to adjacent pixels
                    heat[x-1][y] = constrain(heat[x-1][y] + random(0, 30), 0, 255);
                    heat[x+1][y] = constrain(heat[x+1][y] + random(0, 30), 0, 255);
                }
            }
        }
    }
}

 void matrixDigitalRain(unsigned long time) {
    static int columns[WIDTH];
    static int colSpeed[WIDTH];
    static int colBrightness[WIDTH];
    static unsigned long lastUpdate = 0;
    static unsigned long lastCharChange = 0;
    
    if (time - lastUpdate > 60) {
        lastUpdate = time;
        
        // Fade the screen by drawing a semi-transparent black overlay
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                // 25% chance to fade each pixel to black
                if (random(100) < 25) {
                    dma_display->drawPixel(x, y, dma_display->color565(0, random(5), 0));
                }
            }
        }
        
        // Update characters occasionally
        if (time - lastCharChange > 200) {
            lastCharChange = time;
            // Randomly change some characters (simulated with bright pixels)
            for (int x = 0; x < WIDTH; x++) {
                if (random(100) < 15) {
                    // Draw a bright pixel to simulate a character
                    int yPos = random(HEIGHT);
                    dma_display->drawPixel(x, yPos, dma_display->color565(0, 255, 0));
                    // Add a couple more pixels to make it look more like a character
                    if (yPos > 0) dma_display->drawPixel(x, yPos-1, dma_display->color565(0, 200, 0));
                    if (yPos < HEIGHT-1) dma_display->drawPixel(x, yPos+1, dma_display->color565(0, 150, 0));
                }
            }
        }
        
        // Update columns
        for (int x = 0; x < WIDTH; x++) {
            if (random(100) < 3) { // 3% chance to start new drop
                columns[x] = 0;
                colSpeed[x] = random(1, 5);
                colBrightness[x] = random(200, 256);
            }
            
            if (columns[x] < HEIGHT) {
                // Draw bright head of the rain drop
                dma_display->drawPixel(x, columns[x], dma_display->color565(0, colBrightness[x], 0));
                
                // Draw fading trail
                for (int i = 1; i < 10; i++) {
                    if (columns[x] - i >= 0) {
                        uint8_t intensity = colBrightness[x] - (i * 25);
                        if (intensity > 10) {
                            dma_display->drawPixel(x, columns[x] - i, dma_display->color565(0, intensity, 0));
                        }
                    }
                }
                
                columns[x] += colSpeed[x];
                colBrightness[x] = max(50, colBrightness[x] - 5); // Gradually dim
            } else {
                columns[x] = -random(5, 20); // Reset with random delay
            }
        }
    }
}

// HSV to RGB conversion function for 16-bit color (R5G6B5)
uint16_t hsvToRgb(float h, float s, float v) {
    int i = floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);
    
    float r, g, b;
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    
    // Convert to 16-bit color (5 bits red, 6 bits green, 5 bits blue)
    return ((uint16_t)(r * 31) << 11) | ((uint16_t)(g * 63) << 5) | (uint16_t)(b * 31);
}

// Fast HSV to RGB conversion
uint32_t fastHSVtoRGB(float h, float s, float v) {
    h *= 6.0f;
    int i = (int)h;
    float f = h - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));
    
    float r, g, b;
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    
    return dma_display->color565((uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255));
}

// Generate a random 16-bit color
uint16_t random16BitColor() {
    return ((uint16_t)random(0, 32) << 11) |  // Red: 5 bits
           ((uint16_t)random(0, 64) << 5) |   // Green: 6 bits
           (uint16_t)random(0, 32);           // Blue: 5 bits
}

// Generate a color based on position and time (more interesting than random)
uint16_t dynamicColor(float x, float y, float time) {
    float hue = fmod((x + y) * 0.01 + time * 0.0001, 1.0);
    float saturation = 0.8 + 0.2 * sin(time * 0.0005);
    float value = 0.7 + 0.3 * cos(time * 0.0003);
    
    return hsvToRgb(hue, saturation, value);
}

// 1. Enhanced Plasma Effect with full color range
void plasmaEffect(unsigned long time) {
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            float val = sin(x * 0.1 + time * 0.001) + cos(y * 0.1 + time * 0.002) + sin((x + y) * 0.05 + time * 0.003);
            uint8_t col = (uint8_t)((val + 3) / 6 * 15) + 1; // 1-15
            dma_display->drawPixel(x, y, palette[col % 16]);
        }
    }
}


void plasmaEffect(int px, int py, unsigned long timeOffset) {
    for (int y = 0; y < 64; y++) {
        int screenY = py + y;
        if (screenY < 0 || screenY >= PANEL_RES_Y) continue;
        for (int x = 0; x < 64; x++) {
            int screenX = px + x;
            if (screenX < 0 || screenX >= PANEL_RES_X) continue;
            float v = sin(x * 0.1 + timeOffset * 0.01) + sin(y * 0.1 + timeOffset * 0.01) + sin((x + y) * 0.05 + timeOffset * 0.015);
            uint8_t col = (uint8_t)((v + 3) / 6 * 15); // Map to 0-15
            dma_display->drawPixel(screenX, screenY, palette[col]);
        }
    }
}

// 2. Enhanced Pixel Rain with full color range
void pixelRainEffect(unsigned long time) {
    static int columns[WIDTH];
    static int colSpeed[WIDTH];
    static int colLength[WIDTH];
    static uint16_t colColors[WIDTH]; // Store full colors instead of palette indices
    static unsigned long lastUpdate = 0;
    
    if (time - lastUpdate > 50) {
        lastUpdate = time;
        
        // Fade the screen by drawing a semi-transparent black overlay
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                // 15% chance to fade each pixel to black
                if (random(100) < 15) {
                    dma_display->drawPixel(x, y, 0);
                }
            }
        }
        
        // Update columns
        for (int x = 0; x < WIDTH; x++) {
            if (random(100) < 5) { // 5% chance to start new drop
                columns[x] = 0;
                colSpeed[x] = random(1, 4);
                colLength[x] = random(5, 15);
                colColors[x] = dynamicColor(x, 0, time); // Dynamic color based on position
            }
            
            if (columns[x] < HEIGHT) {
                // Draw new drop head (brightest part)
                dma_display->drawPixel(x, columns[x], colColors[x]);
                
                // Draw trail with fading intensity
                for (int i = 1; i < colLength[x]; i++) {
                    if (columns[x] - i >= 0) {
                        // Calculate faded color
                        uint16_t fadedColor = colColors[x];
                        // Simple fade by reducing intensity
                        uint8_t r = (fadedColor >> 11) & 0x1F;
                        uint8_t g = (fadedColor >> 5) & 0x3F;
                        uint8_t b = fadedColor & 0x1F;
                        
                        // Apply fade based on distance from head
                        float fadeFactor = 1.0 - (i / (float)colLength[x]);
                        r = (uint8_t)(r * fadeFactor);
                        g = (uint8_t)(g * fadeFactor);
                        b = (uint8_t)(b * fadeFactor);
                        
                        uint16_t trailColor = (r << 11) | (g << 5) | b;
                        dma_display->drawPixel(x, columns[x] - i, trailColor);
                    }
                }
                
                columns[x] += colSpeed[x];
            } else {
                columns[x] = HEIGHT + 10; // Reset when off screen
            }
        }
    }
}

void rain16(unsigned long time) {
    static int columns[WIDTH];
    static int colSpeed[WIDTH];
    static int colLength[WIDTH];
    static unsigned long lastUpdate = 0;
    
    if (time - lastUpdate > 50) {
        lastUpdate = time;
        
        // Fade the screen by drawing a semi-transparent black overlay
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                // 20% chance to fade each pixel to black
                if (random(100) < 20) {
                    dma_display->drawPixel(x, y, 0);
                }
            }
        }
        
        // Update columns
        for (int x = 0; x < WIDTH; x++) {
            if (random(100) < 5) { // 5% chance to start new drop
                columns[x] = 0;
                colSpeed[x] = random(1, 4);
                colLength[x] = random(5, 15);
            }
            
            if (columns[x] < HEIGHT) {
                // Draw new drop head
                dma_display->drawPixel(x, columns[x], palette[10]); // Bright green
                
                // Draw trail
                for (int i = 1; i < colLength[x]; i++) {
                    if (columns[x] - i >= 0) {
                        uint8_t intensity = 10 - i;
                        if (intensity > 0) {
                            dma_display->drawPixel(x, columns[x] - i, palette[intensity]);
                        }
                    }
                }
                
                columns[x] += colSpeed[x];
            } else {
                columns[x] = HEIGHT + 10; // Reset when off screen
            }
        }
    }
}


// Helper to get random color index from palette (excluding black=0)
uint8_t randomColor() {
    return random(1, 16); // 1 to 15
}

// 3. Rain  color r 
void  Rain (unsigned long time) {
    static struct Pixel {
        float x, y;
        float speed;
        uint8_t color;
        bool active;
    } pixels[100];
    
    static unsigned long lastSpawn = 0;
    static unsigned long lastFade = 0;
    
    // Fade the screen occasionally
    if (time - lastFade > 100) {
        lastFade = time;
        // Draw random black pixels to create fade effect
        for (int i = 0; i < 66; i++) {
            int x = random(WIDTH);
            int y = random(HEIGHT);
            dma_display->drawPixel(x, y, 0);
        }
    }
    
    // Spawn new pixels
    if (time - lastSpawn > 35) {
        lastSpawn = time;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 100; j++) {
                if (!pixels[j].active) {
                    pixels[j].x = random(0, WIDTH);
                    pixels[j].y = 0;
                    pixels[j].speed = random(5, 15) * 0.1;
                    pixels[j].color = randomColor();
                    pixels[j].active = true;
                    break;
                }
            }
        }
    }
    
    // Update and draw pixels
    for (int i = 0; i < 100; i++) {
        if (pixels[i].active) {
            pixels[i].y += pixels[i].speed;
            if (pixels[i].y < HEIGHT) {
                dma_display->drawPixel(pixels[i].x, pixels[i].y, palette[pixels[i].color]);
            } else {
                pixels[i].active = false;
            }
        }
    }
}


void vortexEffect16(unsigned long time) {
    float centerX = WIDTH / 2.0;
    float centerY = HEIGHT / 2.0;
    float timeFactor = time * 0.001;
    
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            // Calculate distance and angle from center
            float dx = x - centerX;
            float dy = y - centerY;
            float distance = sqrt(dx*dx + dy*dy);
            float angle = atan2(dy, dx);
            
            // Create swirling effect
            float swirl = sin(distance * 0.1 - timeFactor * 2.0);
            float pattern = sin(angle * 5.0 + distance * 0.2 - timeFactor * 3.0 + swirl);
            
            // Map to color
            uint8_t col = (uint8_t)((pattern + 1.0) * 7.5) % 16;
            dma_display->drawPixel(x, y, palette[col]);
        }
    }
}
// 4. Enhanced Vortex Effect with full color range
void vortexEffect(unsigned long time) {
    float centerX = WIDTH / 2.0;
    float centerY = HEIGHT / 2.0;
    float timeFactor = time * 0.001;
    
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            // Calculate distance and angle from center
            float dx = x - centerX;
            float dy = y - centerY;
            float distance = sqrt(dx*dx + dy*dy);
            float angle = atan2(dy, dx);
            
            // Create swirling effect with multiple layers
            float swirl1 = sin(distance * 0.1 - timeFactor * 2.0);
            float swirl2 = cos(angle * 3.0 + timeFactor * 1.5);
            float pattern = sin(angle * 5.0 + distance * 0.2 - timeFactor * 3.0 + swirl1 + swirl2);
            
            // Map to HSV for smooth color transitions
            float hue = fmod(angle / (2 * PI) + timeFactor * 0.2, 1.0);
            float saturation = 0.8 + 0.2 * cos(timeFactor * 0.7);
            float value = 0.5 + 0.5 * pattern;
            
            // Ensure values are in valid range
            hue = fmod(hue + 1.0, 1.0);
            saturation = fmax(0.0, fmin(1.0, saturation));
            value = fmax(0.0, fmin(1.0, value));
            
            uint16_t color = hsvToRgb(hue, saturation, value);
            dma_display->drawPixel(x, y, color);
        }
    }
}

 







// ====================================================
// fire flame
// ====================================================




void initFlame(Flame* f) {
    f->width = WIDTH;
    f->height = HEIGHT;
    f->buffer = (uint8_t*)malloc(WIDTH * HEIGHT);
    f->fire_buffer = (uint8_t*)malloc(WIDTH * HEIGHT);
    memset(f->buffer, 0, WIDTH * HEIGHT);
    memset(f->fire_buffer, 0, WIDTH * HEIGHT);
}

void updateFlame(Flame* f) {
    // Add random "coals" at bottom
    for (int x = 2; x < WIDTH - 2; x += random(4, 7)) {
        if (random(3) == 0) {
            for (int dx = 0; dx < 10; dx++) {
                int y = HEIGHT - 1;
                f->buffer[y * WIDTH + x + dx] = 255;
            }
        }
    }

    // Propagate upward
    for (int y = 1; y < HEIGHT; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            int idx = y * WIDTH + x;
            int avg = (f->buffer[idx] +
                       f->buffer[idx - 1] +
                       f->buffer[idx + 1] +
                       f->buffer[idx - WIDTH]) >> 2;
            int newY = y - 1;
            f->fire_buffer[newY * WIDTH + x] = avg > 2 ? avg - 2 : 0;
        }
    }

    memcpy(f->buffer, f->fire_buffer, WIDTH * HEIGHT);
    memset(f->fire_buffer, 0, WIDTH * HEIGHT);
}

void drawFlame(Flame* f) {
    for (int y = 0; y < HEIGHT ; y++) {
        for (int x = 0; x < WIDTH; x++) {
            uint8_t v = f->buffer[y * WIDTH + x];
            uint8_t r, g, b;
            if (v == 0) {
                r = g = b = 0;
            } else if (v < 85) {
                r = v * 3; g = 0; b = 0;
            } else if (v < 170) {
                r = 255; g = (v - 85) * 3; b = 0;
            } else {
                r = 255; g = 255; b = (v - 170) * 3;
            }
            dma_display->drawPixelRGB888(x, y, r, g, b);
        }
    }
}


 


 
// ====================================================
// plasma
// ====================================================

void initPlasma(Plasma* plasma) {
    plasma->width  = 256;
    plasma->height = 256;
    plasma->body   = (uint8_t*)malloc(256 * 256);
    memset(plasma->body, 0, 256 * 256);

    for (int i = 0; i < 256; i++) {
        plasma->cosinus[i] = (int)(30 * cos(i * (PI / 128)));
    }

    // Same palette init as before …
    for (int i = 0; i < 256; i++) {
        plasma->palette[i][0] = plasma->palette[i][1] = plasma->palette[i][2] = 0;
    }
    for (int x = 1; x <= 32; x++) {
        plasma->palette[x][0] = 0;
        plasma->palette[x][1] = 0;
        plasma->palette[x][2] = (x * 2 - 1) * 4;
        plasma->palette[x + 32][0] = (x * 2 - 1) * 4;
        plasma->palette[x + 32][1] = 0;
        plasma->palette[x + 32][2] = 255;
        plasma->palette[x + 64][0] = 255;
        plasma->palette[x + 64][1] = (x * 2 - 1) * 4;
        plasma->palette[x + 64][2] = 255;
        plasma->palette[x + 96][0] = 255;
        plasma->palette[x + 96][1] = 255;
        plasma->palette[x + 96][2] = 255;
        plasma->palette[x + 128][0] = 255;
        plasma->palette[x + 128][1] = 255;
        plasma->palette[x + 128][2] = 255;
        plasma->palette[x + 160][0] = 255;
        plasma->palette[x + 160][1] = 255;
        plasma->palette[x + 160][2] = (63 - (x * 2 - 1)) * 4;
        plasma->palette[x + 192][0] = 255;
        plasma->palette[x + 192][1] = (63 - (x * 2 - 1)) * 4;
        plasma->palette[x + 192][2] = 0;
        plasma->palette[x + 224][0] = (63 - (x * 2 - 1)) * 4;
        plasma->palette[x + 224][1] = 0;
        plasma->palette[x + 224][2] = 0;
    }
}

void updatePlasma(Plasma* plasma) {
    static uint8_t p1 = 0, p2 = 0, p3 = 0, p4 = 0;
    uint8_t t1, t2, t3, t4;
    int col;

    t1 = p1;
    t2 = p2;
    for (int y = 0; y < plasma->height; y++) {
        t3 = p3;
        t4 = p4;
        for (int x = 0; x < plasma->width; x++) {
            col = plasma->cosinus[t1] + plasma->cosinus[t2] +
                  plasma->cosinus[t3] + plasma->cosinus[t4];
            plasma->body[y * plasma->width + x] = (uint8_t)col;
            t3 = (t3 + 1) & 0xFF;
            t4 = (t4 + 3) & 0xFF;
        }
        t1 = (t1 + 2) & 0xFF;
        t2 = (t2 + 1) & 0xFF;
    }

    p1 = (p1 + 1) & 0xFF;
    p2 = (p2 - 2) & 0xFF;
    p3 = (p3 - 1) & 0xFF;
    p4 = (p4 + 3) & 0xFF;
}

void drawPlasma(Plasma* plasma) {
    for (int y = 0; y < HEIGHT; y++) {
        int iy = (y * plasma->height) / HEIGHT;
        for (int x = 0; x < WIDTH; x++) {
            int ix = (x * plasma->width) / WIDTH;
            uint8_t c = plasma->body[iy * plasma->width + ix];
            uint8_t r = plasma->palette[c][0];
            uint8_t g = plasma->palette[c][1];
            uint8_t b = plasma->palette[c][2];
            dma_display->drawPixelRGB888(x, y, r, g, b);
        }
    }
}

 

// ====================================================
//starfieldEffect 
// ====================================================
// Starfield globals
 
static float lx[NB], ly[NB], lz[NB]; // Star positions
static float vx[NB], vy[NB], vz[NB]; // Star velocities
static int xd, yd; // Projected 2D coordinates
static int n = NB; // Number of active stars
static int nn = 0;

void initStarfield() {
    // Initialize stars at random positions in 3D space
    for (int i = 0; i < NB; i++) {
        lx[i] = random(-100, 100);
        ly[i] = random(-100, 100);
        lz[i] = random(Z_NEAR, Z_FAR); // Random depth
        vx[i] = 0;
        vy[i] = 0;
        vz[i] = -STAR_SPEED; // Velocity toward viewer
    }
}

void updateStarfield() {
    // Update star positions
    for (int i = 0; i < NB; i++) {
        lz[i] += vz[i]; // Move stars toward viewer
        // Reset stars that get too close
        if (lz[i] < Z_NEAR) {
            lx[i] = random(-100, 100);
            ly[i] = random(-100, 100);
            lz[i] = Z_FAR;
        }
    }
}

void starfieldEffect(unsigned long time) {
    // Clear the display
    dma_display->clearScreen();

    // Render stars and trails
    for (nn = 0; nn < n; nn++) {
        // Main star
        if (lz[nn] > 0) {
            float inv_z = 0.25f / lz[nn]; // Lower zoom
            xd = (WIDTH / 2) + (int)(WIDTH * lx[nn] * inv_z);
            yd = (HEIGHT / 2) - (int)(HEIGHT * ly[nn] * inv_z);

            if (xd >= 0 && xd < WIDTH && yd >= 0 && yd < HEIGHT) {
                // Map brightness to 256-color palette
                int brightness = (int)(4096.0f / lz[nn]); // Increased for 256 colors
                brightness = brightness > 255 ? 255 : brightness;
                brightness = brightness < 16 ? 16 : brightness; // Avoid too dim
                dma_display->drawPixel(xd, yd, palette256[brightness]);
                
                // Draw trail
                for (int t = 1; t <= TRAIL_LENGTH; t++) {
                    // Calculate trail position
                    float trail_z = lz[nn] + t * STAR_SPEED;
                    if (trail_z > 0 && trail_z < Z_FAR) {
                        float trail_inv_z = 0.25f / trail_z;
                        int trail_xd = (WIDTH / 2) + (int)(WIDTH * lx[nn] * trail_inv_z);
                        int trail_yd = (HEIGHT / 2) - (int)(HEIGHT * ly[nn] * trail_inv_z);

                        if (trail_xd >= 0 && trail_xd < WIDTH && trail_yd >= 0 && trail_yd < HEIGHT) {
                            // Fade brightness for trail
                            int trail_brightness = (int)(brightness * powf(TRAIL_FADE, t));
                            trail_brightness = trail_brightness < 16 ? 16 : trail_brightness;
                            dma_display->drawPixel(trail_xd, trail_yd, palette256[trail_brightness]);
                        }
                    }
                }
            }
        }
    }

    // Update starfield
    updateStarfield();
}






// ====================================================
// MatrixRain
// ====================================================

void initMatrixRain(MatrixRain* rain) {
  for (int i = 0; i < WIDTH; i++) {
    rain->speed[i] = 5 + random(20);        // Random speed
    rain->length[i] = 5 + random(20);       // Random length
    rain->position[i] = random(HEIGHT * 2); // Random start position
    rain->last_update[i] = 0;
    rain->intensity[i] = 200 + random(55);  // Brightness
    
    // Initialize random characters for this column (0-9, A-Z)
    for (int j = 0; j < 32; j++) {
      rain->chars[i][j] = random(36);
      if (rain->chars[i][j] < 10) {
        rain->chars[i][j] += '0';  // Numbers 0-9
      } else {
        rain->chars[i][j] += 'A' - 10; // Letters A-Z
      }
    }
  }
}

void updateMatrixRain(MatrixRain* rain) {
  static uint32_t last_time = 0;
  uint32_t current_time = millis();
  
  for (int x = 0; x < WIDTH; x++) {
    // Update this column based on its speed
    if (current_time - rain->last_update[x] > rain->speed[x]) {
      rain->position[x] = (rain->position[x] + 1) % (HEIGHT + rain->length[x] + 10);
      rain->last_update[x] = current_time;
      
      // Occasionally change properties
      if (random(100) < 2) {
        rain->speed[x] = 5 + random(20);
      }
      if (random(100) < 1) {
        rain->length[x] = 5 + random(20);
      }
      if (random(100) < 5) {
        // Change a random character in this column
        int char_pos = random(rain->length[x]);
        rain->chars[x][char_pos] = random(36);
        if (rain->chars[x][char_pos] < 10) {
          rain->chars[x][char_pos] += '0';
        } else {
          rain->chars[x][char_pos] += 'A' - 10;
        }
      }
    }
  }
}

void drawMatrixRain(MatrixRain* rain) {
  // Clear screen with dark green
  dma_display->fillScreen(dma_display->color565(0, 10, 0));
  
  for (int x = 0; x < WIDTH; x++) {
    int start_pos = rain->position[x];
    
    for (int i = 0; i < rain->length[x]; i++) {
      int y_pos = start_pos - i;
      
      if (y_pos >= 0 && y_pos < HEIGHT) {
        // Calculate brightness (fade out toward the tail)
        uint8_t brightness = rain->intensity[x] * (rain->length[x] - i) / rain->length[x];
        
        // Head of the rain is bright green
        if (i == 0) {
          dma_display->drawPixel(x, y_pos, dma_display->color565(0, 255, 0));
        } 
        // Body fades from green to dark green
        else {
          uint8_t green = brightness;
          dma_display->drawPixel(x, y_pos, dma_display->color565(0, green, 0));
        }
      }
    }
  }
}




// ====================================================
// WaterRipple
// ====================================================

void initWaterRipple(WaterRipple* water) {
  water->last_update = 0;
  water->damping = 24;  // Higher damping for more realistic effect
  water->ripple_intensity = 180;
  water->last_ripple_time = 0;
  water->color_mode = 0; // 0: Blue, 1: Multi-color, 2: Psychedelic
  
  // Clear buffers with more precision
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      water->current[y][x] = 0;
      water->previous[y][x] = 0;
    }
  }
}

void addRipple(WaterRipple* water, int x, int y, int intensity) {
  if (x >= 1 && x < WIDTH-1 && y >= 1 && y < HEIGHT-1) {
    water->previous[y][x] = intensity;
    water->previous[y-1][x] = intensity * 0.7;
    water->previous[y+1][x] = intensity * 0.7;
    water->previous[y][x-1] = intensity * 0.7;
    water->previous[y][x+1] = intensity * 0.7;
  }
}

void updateWaterRipple(WaterRipple* water) {
  uint32_t current_time = millis();
  
  // Add automatic ripples at random intervals
  if (current_time - water->last_ripple_time > 800 + random(1200)) {
    int x = 10 + random(WIDTH - 20);
    int y = 10 + random(HEIGHT - 20);
    addRipple(water, x, y, water->ripple_intensity);
    water->last_ripple_time = current_time;
  }
  
  // Add occasional random droplets
  if (random(100) < 15) {
    int x = random(WIDTH);
    int y = random(HEIGHT);
    addRipple(water, x, y, 80 + random(100));
  }
  
  // Enhanced wave propagation with better physics
  for (int y = 1; y < HEIGHT - 1; y++) {
    for (int x = 1; x < WIDTH - 1; x++) {
      // Improved wave equation with smoothing
      int16_t new_value = (
        water->previous[y-1][x] + 
        water->previous[y+1][x] + 
        water->previous[y][x-1] + 
        water->previous[y][x+1] +
        water->previous[y-1][x-1] * 0.7 +
        water->previous[y-1][x+1] * 0.7 +
        water->previous[y+1][x-1] * 0.7 +
        water->previous[y+1][x+1] * 0.7
      ) / 4 - water->current[y][x];
      
      // Apply damping with better curve
      water->current[y][x] = new_value * (255 - water->damping) / 256;
      
      // Add slight random noise for more natural look
      if (random(100) < 2) {
        water->current[y][x] += random(-5, 6);
      }
    }
  }
  
  // Handle edges with reflection
  for (int y = 0; y < HEIGHT; y++) {
    water->current[y][0] = water->current[y][1] * 0.8;
    water->current[y][WIDTH-1] = water->current[y][WIDTH-2] * 0.8;
  }
  for (int x = 0; x < WIDTH; x++) {
    water->current[0][x] = water->current[1][x] * 0.8;
    water->current[HEIGHT-1][x] = water->current[HEIGHT-2][x] * 0.8;
  }
  
  // Swap buffers
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int16_t temp = water->previous[y][x];
      water->previous[y][x] = water->current[y][x];
      water->current[y][x] = temp;
    }
  }
}

uint32_t getWaterColor(int16_t height, uint8_t color_mode) {
  int16_t h = constrain(height + 128, 0, 255);
  
  switch (color_mode) {
    case 1: // Multi-color mode
      {
        // Create rainbow effect based on height
        uint8_t r, g, b;
        if (h < 85) {
          r = h * 3;
          g = 0;
          b = 255 - h * 3;
        } else if (h < 170) {
          r = 255;
          g = (h - 85) * 3;
          b = 0;
        } else {
          r = 255 - (h - 170) * 3;
          g = 255;
          b = (h - 170) * 3;
        }
        return dma_display->color565(r, g, b);
      }
      
    case 2: // Psychedelic mode
      {
        uint32_t time = millis() / 50;
        uint8_t r = (h + time) % 256;
        uint8_t g = (h + time / 2) % 256;
        uint8_t b = (h + time / 3) % 256;
        return dma_display->color565(r, g, b);
      }
      
    default: // Classic blue water mode
      {
        uint8_t intensity = abs(height) / 2;
        uint8_t b = 64 + intensity;
        uint8_t g = 32 + intensity / 2;
        uint8_t r = intensity / 4;
        
        // Add specular highlights for more realistic water
        if (height > 20) {
          uint8_t highlight = min(255, (height - 20) * 3);
          r = min(255, r + highlight / 2);
          g = min(255, g + highlight / 2);
          b = min(255, b + highlight);
        }
        
        return dma_display->color565(r, g, b);
      }
  }
}

void drawWaterRipple(WaterRipple* water) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int16_t height = water->current[y][x];
      uint32_t color = getWaterColor(height, water->color_mode);
      dma_display->drawPixel(x, y, color);
    }
  }
}

// Function to change color mode
void nextColorMode(WaterRipple* water) {
  water->color_mode = (water->color_mode + 1) % 3;
}

// Function to add ripple at specific position (for interactivity)
void addRippleAt(WaterRipple* water, int x, int y) {
  addRipple(water, x, y, water->ripple_intensity);
}
  











// ====================================================
// radar
// ====================================================

void initRadarScan() {
  radar.angle = 0;
  radar.sweep_speed = 0.03;
  radar.last_update = 0;
  
  // Clear trail and blip maps
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      radar.trail[y][x] = 0;
      radar.blip_map[y][x] = 0;
    }
  }
}

void updateRadarScan() {
  // Update angle
  radar.angle += radar.sweep_speed;
  if (radar.angle >= TWO_PI) radar.angle = 0;
  
  // Fade existing trails (green sweep)
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (radar.trail[y][x] > 0) {
        radar.trail[y][x] -= 2;
      }
      // Fade white blips
      if (radar.blip_map[y][x] > 0) {
        radar.blip_map[y][x] -= 3;
      }
    }
  }
  
  // Add new sweep line (green)
  int center_x = WIDTH / 2;
  int center_y = HEIGHT / 2;
  int max_radius = min(WIDTH, HEIGHT) / 2;
  
  for (int r = 0; r < max_radius; r++) {
    int x = center_x + cos(radar.angle) * r;
    int y = center_y + sin(radar.angle) * r;
    
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
      // Brightness decreases with distance
      uint8_t brightness = 255 - (r * 255 / max_radius);
      radar.trail[y][x] = max(radar.trail[y][x], brightness);
    }
  }
  
  // Add random blips (targets) - WHITE
  if (random(100) < 5) {
    int r = random(max_radius/2, max_radius);
    float blip_angle = random(TWO_PI * 100) / 100.0;
    
    int x = center_x + cos(blip_angle) * r;
    int y = center_y + sin(blip_angle) * r;
    
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
      // Create white blip with glow effect
      for (int dy = 0; dy <= 0; dy++) {
        for (int dx = 0; dx <= 1; dx++) {
          int nx = x + dx;
          int ny = y + dy;
          if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
            int distance = max(abs(dx), abs(dy));
            uint8_t glow = 180 - (distance * 55);
            if (glow > 0) {
              radar.blip_map[ny][nx] = max(radar.blip_map[ny][nx], glow);
            }
          }
        }
      }
    }
  }
}

void drawRadarScan() {
  dma_display->fillScreen(dma_display->color565(0, 10, 0));
  
  // Draw green trails (sweep)
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (radar.trail[y][x] > 0) {
        uint8_t red = radar.trail[y][x];
        dma_display->drawPixel(x, y, dma_display->color565(red, 0, 0));
      }
    }
  }
  
  // Draw white blips (targets)
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (radar.blip_map[y][x] > 0) {
        uint8_t white = radar.blip_map[y][x];
        dma_display->drawPixel(x, y, dma_display->color565(white, white, white));
      }
    }
  }
  
  // Draw radar circle (green)
  int center_x = WIDTH / 2;
  int center_y = HEIGHT / 2;
  int radius = min(WIDTH, HEIGHT) / 2 - 2;
  
  dma_display->drawCircle(center_x, center_y, radius, dma_display->color565(0, 100, 0));
  dma_display->drawCircle(center_x, center_y, radius/2, dma_display->color565(0, 80, 0));
  
  // Draw crosshairs (green)
  dma_display->drawLine(center_x, 2, center_x, HEIGHT-3, dma_display->color565(0, 80, 0));
  dma_display->drawLine(2, center_y, WIDTH-3, center_y, dma_display->color565(0, 80, 0));
  
  // Draw center dot (red)
  dma_display->fillCircle(center_x, center_y, 1, dma_display->color565(0, 80, 0));

}










// ====================================================
// Modern Particle Wave Animation
// ====================================================

Particle particles[NUM_PARTICLES];
float time_counter = 0;
uint8_t color_mode = 0;

void initParticleSystem() {
  time_counter = 0;
  
  for (int i = 0; i < NUM_PARTICLES; i++) {
    particles[i].x = random(WIDTH);
    particles[i].y = random(HEIGHT);
    particles[i].vx = random(-200, 200) / 100.0;
    particles[i].vy = random(-200, 200) / 100.0;
    particles[i].color_index = random(8);  // Use all 8 colors
    particles[i].size = 1 + random(4);     // Sizes: 1-5 pixels (varied)
    particles[i].mass = 1 + particles[i].size;
  }
}

void checkParticleCollision(Particle* p1, Particle* p2) {
  float dx = p2->x - p1->x;
  float dy = p2->y - p1->y;
  float distance = sqrt(dx * dx + dy * dy);
  float min_distance = p1->size + p2->size;
  
  if (distance < min_distance && distance > 0) {
    // Collision detected - calculate rebound
    float angle = atan2(dy, dx);
    float sin_angle = sin(angle);
    float cos_angle = cos(angle);
    
    // Rotate velocity vectors
    float vx1_rot = p1->vx * cos_angle + p1->vy * sin_angle;
    float vy1_rot = p1->vy * cos_angle - p1->vx * sin_angle;
    float vx2_rot = p2->vx * cos_angle + p2->vy * sin_angle;
    float vy2_rot = p2->vy * cos_angle - p2->vx * sin_angle;
    
    // Exchange x velocities (elastic collision)
    float temp = vx1_rot;
    vx1_rot = vx2_rot;
    vx2_rot = temp;
    
    // Rotate back
    p1->vx = vx1_rot * cos_angle - vy1_rot * sin_angle;
    p1->vy = vy1_rot * cos_angle + vx1_rot * sin_angle;
    p2->vx = vx2_rot * cos_angle - vy2_rot * sin_angle;
    p2->vy = vy2_rot * cos_angle + vx2_rot * sin_angle;
    
    // Separate particles to prevent sticking
    float overlap = min_distance - distance + 0.5;
    p1->x -= overlap * cos_angle * 0.5;
    p1->y -= overlap * sin_angle * 0.5;
    p2->x += overlap * cos_angle * 0.5;
    p2->y += overlap * sin_angle * 0.5;
  }
}

void updateParticleSystem() {
  time_counter += 0.02;
  
  // Update particle positions and check screen boundaries
  for (int i = 0; i < NUM_PARTICLES; i++) {
    Particle* p = &particles[i];
    
    // Add subtle wave motion
    p->vx += sin(time_counter * 0.5 + p->y * 0.1) * 0.1;
    p->vy += cos(time_counter * 0.4 + p->x * 0.1) * 0.1;
    
    // Update position
    p->x += p->vx;
    p->y += p->vy;
    
    // Screen boundary collision - bounce with energy loss
    if (p->x < p->size) {
      p->x = p->size;
      p->vx = -p->vx * 0.9;
    }
    if (p->x >= WIDTH - p->size) {
      p->x = WIDTH - p->size - 1;
      p->vx = -p->vx * 0.9;
    }
    if (p->y < p->size) {
      p->y = p->size;
      p->vy = -p->vy * 0.9;
    }
    if (p->y >= HEIGHT - p->size) {
      p->y = HEIGHT - p->size - 1;
      p->vy = -p->vy * 0.9;
    }
    
    // Apply slight damping
    p->vx *= 0.995;
    p->vy *= 0.995;
  }
  
  // Check particle-particle collisions
  for (int i = 0; i < NUM_PARTICLES; i++) {
    for (int j = i + 1; j < NUM_PARTICLES; j++) {
      checkParticleCollision(&particles[i], &particles[j]);
    }
  }
  
  // Occasionally change color mode
  if (random(1000) < 1) {
    color_mode = (color_mode + 1) % 4;
  }
}

void drawParticleSystem() {
  // Clear with very dark background for trail effect
  dma_display->fillRect(0, 0, WIDTH, HEIGHT, dma_display->color565(1, 1, 2));
  
  for (int i = 0; i < NUM_PARTICLES; i++) {
    Particle* p = &particles[i];
    
    // Get color from current palette
    uint32_t color_val = palettes[color_mode][p->color_index];
    uint8_t r = (color_val >> 16) & 0xFF;
    uint8_t g = (color_val >> 8) & 0xFF;
    uint8_t b = color_val & 0xFF;
    
    // Draw the particle with size
    if (p->size == 1) {
      dma_display->drawPixel(p->x, p->y, dma_display->color565(r, g, b));
    } else {
      dma_display->fillCircle(p->x, p->y, p->size, dma_display->color565(r, g, b));
    }
    
    // Draw subtle motion trail
    for (int j = 1; j <= 2; j++) {
      int trail_x = p->x - p->vx * j;
      int trail_y = p->y - p->vy * j;
      
      if (trail_x >= 0 && trail_x < WIDTH && trail_y >= 0 && trail_y < HEIGHT) {
        uint8_t alpha = 100 - j * 40;
        dma_display->drawPixel(trail_x, trail_y, 
          dma_display->color565(r * alpha / 255, 
                               g * alpha / 255, 
                               b * alpha / 255));
      }
    }
  }
}

void nextColorMode() {
  color_mode = (color_mode + 1) % 4;
}








//////////////////////////////////////////////////////////////////


// Fast sine approximation using cubic curve (3x faster than standard sin)
float fastSin(float x) {
    // Wrap x to [0, 2*PI] - much faster than fmod
    x = x - 6.283185307f * floor(x / 6.283185307f);
    
    // Cubic approximation for sine
    if (x < 3.141592654f) {
        return (4.0f * x * (3.141592654f - x)) / (9.869604401f);
    } else {
        x = 6.283185307f - x;
        return -(4.0f * x * (3.141592654f - x)) / (9.869604401f);
    }
}

// Fast cosine using sine identity
float fastCos(float x) {
    return fastSin(x + 1.570796327f); // PI/2
}

// Fast inverse square root (famous Quake algorithm)
float fastInvSqrt(float x) {
    union { float f; uint32_t i; } u;
    u.f = x;
    u.i = 0x5f3759df - (u.i >> 1);
    return u.f * (1.5f - 0.5f * x * u.f * u.f);
}

// Fast square root using inverse square root
float fastSqrt(float x) {
    return x * fastInvSqrt(x);
}

// Fast atan2 approximation (good enough for visual effects)
float fastAtan2(float y, float x) {
    float abs_x = abs(x);
    float abs_y = abs(y);
    float a = min(abs_x, abs_y) / max(abs_x, abs_y);
    float s = a * a;
    float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;
    
    if (abs_y > abs_x) r = 1.570796327f - r;
    if (x < 0) r = 3.141592654f - r;
    if (y < 0) r = -r;
    
    return r;
}

 

// Fast floor function
float fastFloor(float x) {
    return (float)((int)x - (x < 0));
}

// Fast absolute value
float fastAbs(float x) {
    union { float f; uint32_t i; } u;
    u.f = x;
    u.i &= 0x7FFFFFFF;
    return u.f;
}

// Fast min/max functions
float fastMin(float a, float b) { return a < b ? a : b; }
float fastMax(float a, float b) { return a > b ? a : b; }






////////////////////////////////////////////////////////////////////////
void fastFractalPlasma(unsigned long time) {
    static unsigned long lastUpdate = 0;
    static float timeOffset = 0.0;
    
    if (time - lastUpdate > 16) { // ~60 FPS
        lastUpdate = time;
        timeOffset += 0.04;
        
        // Pre-calculate some values for speed
        float halfW = WIDTH * 0.5f;
        float halfH = HEIGHT * 0.5f;
        float timeFactor1 = timeOffset * 1.3f;
        float timeFactor2 = timeOffset * 0.7f;
        float timeFactor3 = timeOffset * 1.7f;
        
        for (int x = 0; x < WIDTH; x++) {
            float fx = (float)x;
            for (int y = 0; y < HEIGHT; y++) {
                float fy = (float)y;
                
                // Fast fractal plasma calculation using optimized trig approximations
                float dx = fx - halfW;
                float dy = fy - halfH;
                float dist = fastSqrt(dx*dx + dy*dy) * 0.1f;
                
                // Fast sine approximations using polynomial curves
                float plasma = 
                    fastSin(fx * 0.08f + timeFactor1) * 0.25f +
                    fastSin(fy * 0.09f + timeFactor2) * 0.25f +
                    fastSin((fx + fy) * 0.05f + timeFactor3) * 0.25f +
                    fastSin(dist + timeOffset) * 0.25f;
                
                // Map to vibrant colors with fast HSV conversion
                float hue = fmod(plasma * 2.0f + timeOffset * 0.5f, 1.0f);
                uint32_t rgb = fastHSVtoRGB(hue, 1.0f, 1.0f);
                
                dma_display->drawPixel(x, y, rgb);
            }
        }
    }
}





// Color palette functions
uint32_t paletteRainbow(float plasma, float timeOffset) {
    float hue = fmod(plasma * 2.0f + timeOffset * 0.5f, 1.0f);
    return fastHSVtoRGB(hue, 1.0f, 1.0f);
}

uint32_t paletteFire(float plasma) {
    plasma = (plasma + 1.0f) * 0.5f; // Normalize to 0-1
    uint8_t r = constrain(plasma * 255, 0, 255);
    uint8_t g = constrain(plasma * plasma * 200, 0, 200);
    uint8_t b = constrain(plasma * plasma * plasma * 100, 0, 100);
    return dma_display->color565(r, g, b);
}

uint32_t paletteIce(float plasma) {
    plasma = (plasma + 1.0f) * 0.5f;
    uint8_t r = constrain(plasma * 100, 0, 100);
    uint8_t g = constrain(plasma * 150 + 50, 50, 200);
    uint8_t b = constrain(plasma * 200 + 55, 55, 255);
    return dma_display->color565(r, g, b);
}

uint32_t paletteNeon(float plasma, float timeOffset) {
    float hue = fmod(plasma * 1.5f + timeOffset * 0.3f, 1.0f);
    uint8_t r = (sin(hue * 6.283f + 0.0f) * 127 + 128);
    uint8_t g = (sin(hue * 6.283f + 2.094f) * 127 + 128);
    uint8_t b = (sin(hue * 6.283f + 4.188f) * 127 + 128);
    return dma_display->color565(r, g, b);
}

uint32_t palettePastel(float plasma) {
    plasma = (plasma + 1.0f) * 0.5f;
    uint8_t r = 127 + sin(plasma * 6.283f) * 64;
    uint8_t g = 127 + sin(plasma * 6.283f + 2.094f) * 64;
    uint8_t b = 127 + sin(plasma * 6.283f + 4.188f) * 64;
    return dma_display->color565(r, g, b);
}

uint32_t paletteMono(float plasma) {
    uint8_t intensity = constrain((plasma + 1.0f) * 127.5f, 0, 255);
    return dma_display->color565(intensity, intensity, intensity);
}



// Specific color palette functions
uint32_t electricPulseColor(float val, float time) {
    uint8_t r = (fastSin(val * 10.0f + time * 5.0f) * 127 + 128);
    uint8_t g = (fastSin(val * 8.0f + time * 4.0f + 2.0f) * 127 + 128);
    uint8_t b = 255;
    return dma_display->color565(r, g, b);
}

uint32_t lavaFlowColor(float val) {
    uint8_t r = constrain(val * 255 + 100, 100, 255);
    uint8_t g = constrain(val * val * 180, 0, 180);
    uint8_t b = constrain(val * val * val * 80, 0, 80);
    return dma_display->color565(r, g, b);
}

uint32_t oceanWaveColor(float val, float time) {
    uint8_t r = 0;
    uint8_t g = constrain((val + 1.0f) * 80 + fastSin(time) * 30, 50, 150);
    uint8_t b = constrain((val + 1.0f) * 120 + fastCos(time) * 40, 80, 200);
    return dma_display->color565(r, g, b);
}

uint32_t cosmicNebulaColor(float val, int x, int y) {
    float dist = fastSqrt(x*x + y*y) * 0.01f;
    uint8_t r = (fastSin(val * 6.0f + dist) * 100 + 100);
    uint8_t g = (fastCos(val * 7.0f + dist) * 80 + 80);
    uint8_t b = (fastSin(val * 8.0f + dist + 1.57f) * 120 + 120);
    return dma_display->color565(r, g, b);
}

uint32_t forestEnergyColor(float val, float time) {
    uint8_t r = constrain(val * 80 + fastSin(time) * 30, 0, 120);
    uint8_t g = constrain(val * 180 + fastCos(time) * 50, 100, 230);
    uint8_t b = constrain(val * 100 + fastSin(time * 1.3f) * 20, 50, 150);
    return dma_display->color565(r, g, b);
}

uint32_t metallicShiftColor(float val, float time) {
    uint8_t intensity = (fastSin(val * 12.0f + time * 3.0f) * 80 + 150);
    return dma_display->color565(intensity, intensity, intensity);
}

uint32_t psychedelicColor(float val, float time) {
    uint8_t r = (fastSin(val * 8.0f + time * 2.0f) * 127 + 128);
    uint8_t g = (fastSin(val * 6.0f + time * 3.0f + 2.0f) * 127 + 128);
    uint8_t b = (fastSin(val * 7.0f + time * 4.0f + 4.0f) * 127 + 128);
    return dma_display->color565(r, g, b);
}

uint32_t binaryStormColor(float val, float time) {
    if (fastSin(val * 20.0f + time * 8.0f) > 0.7f) {
        return dma_display->color565(255, 255, 255);
    } else {
        return dma_display->color565(0, 0, 0);
    }
}



void switchFractalPlasma(unsigned long time) {
    static unsigned long lastUpdate = 0;
    static float timeOffset = 0.0;
    static int paletteMode = 0;
    static unsigned long lastPaletteChange = 0;
    
    if (time - lastUpdate > 16) { // ~60 FPS
        lastUpdate = time;
        timeOffset += 0.04;
        
        // Change palette every 5 seconds *10
        if (time - lastPaletteChange > 5000*10) {
            lastPaletteChange = time;
            paletteMode = (paletteMode + 1) % 6;
        }
        
        // Pre-calculate some values for speed
        float halfW = WIDTH * 0.5f;
        float halfH = HEIGHT * 0.5f;
        float timeFactor1 = timeOffset * 1.3f;
        float timeFactor2 = timeOffset * 0.7f;
        float timeFactor3 = timeOffset * 1.7f;
        
        for (int x = 0; x < WIDTH; x++) {
            float fx = (float)x;
            for (int y = 0; y < HEIGHT; y++) {
                float fy = (float)y;
                
                // Fast fractal plasma calculation
                float dx = fx - halfW;
                float dy = fy - halfH;
                float dist = fastSqrt(dx*dx + dy*dy) * 0.1f;
                
                float plasma = 
                    fastSin(fx * 0.08f + timeFactor1) * 0.25f +
                    fastSin(fy * 0.09f + timeFactor2) * 0.25f +
                    fastSin((fx + fy) * 0.05f + timeFactor3) * 0.25f +
                    fastSin(dist + timeOffset) * 0.25f;
                
                // Apply different color palettes
                uint32_t color;
                switch (paletteMode) {
                    case 0: // Rainbow
                        color = paletteRainbow(plasma, timeOffset);
                        break;
                    case 1: // Fire
                        color = paletteFire(plasma);
                        break;
                    case 2: // Ice
                        color = paletteIce(plasma);
                        break;
                    case 3: // Neon
                        color = paletteNeon(plasma, timeOffset);
                        break;
                    case 4: // Pastel
                        color = palettePastel(plasma);
                        break;
                    case 5: // Monochrome
                        color = paletteMono(plasma);
                        break;
                }
                
                dma_display->drawPixel(x, y, color);
            }
        }
    }
}





/////////////////////////////////////////////////////////////////////////////////

  



// Different fractal pattern generators
float fastFractalNoise(float x, float y, float time) {
    return fastSin(x * 0.8f + time) * fastSin(y * 0.6f + time * 1.3f);
}

float fastFractalCircles(float x, float y, float time) {
    float dist = fastSqrt(x*x + y*y) * 0.1f;
    return fastSin(dist * 3.0f + time * 2.0f);
}

float fastFractalSpiral(float x, float y, float time) {
    float angle = atan2(y, x);
    float dist = fastSqrt(x*x + y*y) * 0.05f;
    return fastSin(angle * 5.0f + dist * 8.0f + time * 3.0f);
}

float fastFractalGrid(float x, float y, float time) {
    return fastSin(x * 0.15f + time) * fastCos(y * 0.12f + time * 1.7f);
}


// Color generation for different modes
uint32_t getFractalColor(float fractal, float time, int x, int y, int mode) {
    switch (mode) {
        case 0: // Electric pulses
            return electricPulseColor(fractal, time);
        case 1: // Lava flow
            return lavaFlowColor(fractal);
        case 2: // Ocean waves
            return oceanWaveColor(fractal, time);
        case 3: // Cosmic nebula
            return cosmicNebulaColor(fractal, x, y);
        case 4: // Forest energy
            return forestEnergyColor(fractal, time);
        case 5: // Metallic shift
            return metallicShiftColor(fractal, time);
        case 6: // Psychedelic
            return psychedelicColor(fractal, time);
        case 7: // Binary storm
            return binaryStormColor(fractal, time);
        default:
            return dma_display->color565(255, 255, 255);
    }
}

void fastFractalMorph(unsigned long time) {
    static unsigned long lastUpdate = 0;
    static float timeOffset = 0.0;
    static int colorMode = 0;
    static unsigned long lastModeChange = 0;
    
    if (time - lastUpdate > 16) { // ~60 FPS
        lastUpdate = time;
        timeOffset += 0.05;
        
        // Change color mode every 4 seconds
        if (time - lastModeChange > 4000*4) {
            lastModeChange = time;
            colorMode = (colorMode + 1) % 8;
        }
        
        float halfW = WIDTH * 0.5f;
        float halfH = HEIGHT * 0.5f;
        
        for (int x = 0; x < WIDTH; x++) {
            float fx = (float)x - halfW;
            for (int y = 0; y < HEIGHT; y++) {
                float fy = (float)y - halfH;
                
                // Multiple fractal patterns that morph over time
                float pattern1 = fastFractalNoise(fx * 0.1f, fy * 0.1f, timeOffset * 0.3f);
                float pattern2 = fastFractalCircles(fx, fy, timeOffset * 0.5f);
                float pattern3 = fastFractalSpiral(fx, fy, timeOffset * 0.7f);
                float pattern4 = fastFractalGrid(fx, fy, timeOffset * 0.9f);
                
                // Combine patterns with time-varying weights
                float weight1 = 0.25f + 0.2f * fastSin(timeOffset * 0.4f);
                float weight2 = 0.25f + 0.2f * fastSin(timeOffset * 0.6f + 1.57f);
                float weight3 = 0.25f + 0.2f * fastSin(timeOffset * 0.8f + 3.14f);
                float weight4 = 0.25f + 0.2f * fastSin(timeOffset * 1.0f + 4.71f);
                
                float fractal = (pattern1 * weight1 + pattern2 * weight2 + 
                               pattern3 * weight3 + pattern4 * weight4) / 
                               (weight1 + weight2 + weight3 + weight4);
                
                // Get color based on current mode
                uint32_t color = getFractalColor(fractal, timeOffset, x, y, colorMode);
                
                dma_display->drawPixel(x, y, color);
            }
        }
    }
}





///////////////////////////////////
void vanGoghPaintAnimation(unsigned long time) {
    static int columns[WIDTH];
    static int colSpeed[WIDTH];
    static int colBrightness[WIDTH];
    static unsigned long lastUpdate = 0;
    static unsigned long lastSwirlChange = 0;
    
    if (time - lastUpdate > 100) { // Keep slow update for smooth, dreamy effect
        lastUpdate = time;
        
        // Fade screen with semi-transparent dark blue overlay (Starry Night sky)
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                if (random(100) < 20) { // 20% chance to fade to dark blue
                    dma_display->drawPixel(x, y, dma_display->color565(0, random(10, 30), random(50, 80)));
                }
            }
        }
        
        // Update swirling "brushstrokes" occasionally
        if (time - lastSwirlChange > 300) { // Slower updates for brushstroke changes
            lastSwirlChange = time;
            for (int x = 0; x < WIDTH; x++) {
                if (random(100) < 10) { // 10% chance to add new stroke
                    int yPos = 0; // Start at top for downward motion
                    // Starry Night colors: bright yellows, whites, or blues
                    uint8_t r = random(100) < 30 ? random(150, 255) : 0; // Yellow/white or blue
                    uint8_t g = random(100) < 30 ? random(150, 255) : random(10, 50);
                    uint8_t b = random(100) < 70 ? random(100, 200) : random(50, 100);
                    // Draw a "brushstroke" (cluster of pixels)
                    dma_display->drawPixel(x, yPos, dma_display->color565(r, g, b));
                    // Simulate Van Gogh's swirling strokes with adjacent pixels
                    if (yPos < HEIGHT-1) dma_display->drawPixel(x, yPos+1, dma_display->color565(r*0.8, g*0.8, b*0.8));
                    if (x > 0) dma_display->drawPixel(x-1, yPos, dma_display->color565(r*0.7, g*0.7, b*0.7));
                    if (x < WIDTH-1) dma_display->drawPixel(x+1, yPos, dma_display->color565(r*0.7, g*0.7, b*0.7));
                }
            }
        }
        
        // Update columns for downward swirling movement
        for (int x = 0; x < WIDTH; x++) {
            if (random(100) < 5) { // 5% chance to start new downward swirl
                columns[x] = 0; // Start at top
                colSpeed[x] = random(1, 3); // Slower movement for downward flow
                colBrightness[x] = random(150, 256); // Brighter for vivid colors
            }
            
            if (columns[x] < HEIGHT) {
                // Draw head of the swirl in Starry Night colors
                uint8_t r = random(100) < 30 ? colBrightness[x] : 0; // Yellow/white or blue
                uint8_t g = random(100) < 30 ? colBrightness[x] : colBrightness[x] * 0.3;
                uint8_t b = random(100) < 70 ? colBrightness[x] : colBrightness[x] * 0.5;
                
                // Add sinusoidal swirl effect with downward motion
                int xOffset = sin((float)columns[x] / 8.0) * 3; // Increased swirl amplitude
                int xPos = x + xOffset;
                if (xPos >= 0 && xPos < WIDTH) {
                    dma_display->drawPixel(xPos, columns[x], dma_display->color565(r, g, b));
                }
                
                // Draw fading, wavy trail to mimic brushstrokes
                for (int i = 1; i < 8; i++) {
                    if (columns[x] - i >= 0) {
                        uint8_t intensity = colBrightness[x] - (i * 20);
                        if (intensity > 20) {
                            // Apply swirl to trail with slight variation
                            int trailXOffset = sin((float)(columns[x] - i) / 8.0) * 3;
                            int trailXPos = x + trailXOffset;
                            if (trailXPos >= 0 && trailXPos < WIDTH) {
                                dma_display->drawPixel(trailXPos, columns[x] - i, dma_display->color565(r * (intensity/255.0), g * (intensity/255.0), b * (intensity/255.0)));
                            }
                        }
                    }
                }
                
                columns[x] += colSpeed[x]; // Move downward
                colBrightness[x] = max(50, colBrightness[x] - 3); // Slower dimming
            } else {
                columns[x] = -random(10, 30); // Reset at top with random delay
            }
        }
    }
}







 
void Jetstream(float t) {
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      float fx = (float)x / 64.0;
      float fy = (float)y / 64.0;

      // Simulate swirling jetstream pattern
      float dx = fx - 0.5;
      float dy = fy - 0.5;
      float dist = sqrt(dx * dx + dy * dy);
      float angle = atan2(dy, dx) + t * 0.5;
      float wave = sin(10.0 * dist - t * 2.0 + angle);

      // Color modulation
      uint8_t r = (uint8_t)(128 + 127 * sin(wave + t));
      uint8_t g = (uint8_t)(128 + 127 * sin(wave + t + 2));
      uint8_t b = (uint8_t)(128 + 127 * sin(wave + t + 4));

     dma_display->drawPixel(x, y,dma_display->color565(r, g, b));
    }
  }
}



void Pivotal(float t) {
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      // Normalize coordinates to center
      float px = (float)x / 64.0;
      float py = (float)y / 64.0;
      float nx = (px * 2.0 - 1.0) / 0.2;
      float ny = (py * 2.0 - 1.0) / 0.2;

      // Fractional swirl offset
      float fx = nx - floor(nx);
      float fy = ny - floor(ny);
      float cx = fx - 0.5 - nx * 0.2;
      float cy = fy - 0.5 - ny * 0.2;

      // Angular color modulation
      float angle = atan2(cy, cx);
      uint8_t r = (uint8_t)(127 + 128 * cos(angle + t));
      uint8_t g = (uint8_t)(127 + 128 * cos(angle + t + 1.0));
      uint8_t b = (uint8_t)(127 + 128 * cos(angle + t + 2.0));

      dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
    }
  }
}


void Pillars(float t) {
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      // Normalize coordinates
      float px = (float)x / 64.0;
      float py = (float)y / 64.0;

      // Transform to pillar space
      float nx = (px * 2.0 - 1.0) / 0.3 + t * (2.0 / PI);
      float ny = (py * 2.0 - 1.0) / 0.3 + t;

      // Modulo for repeating columns
      float wx = fmod(nx, 2.0) - 1.0;

      // Avoid sqrt domain error
      float arc = sqrt(fmax(0.0, 1.0 - wx * wx));

      // Phase shift per column
      float phase = ny - arc * cos(ceil(nx * 0.5) * PI);

      // Color modulation
      uint8_t r = (uint8_t)(127 + 128 * sin(phase));
      uint8_t g = (uint8_t)(127 + 128 * sin(phase + 1.0));
      uint8_t b = (uint8_t)(127 + 128 * sin(phase + 2.0));

      dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
    }
  }
}


  
void RadialGlow(float t) {
  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      // Normalize coordinates to center
      float fx = (float)x / 64.0;
      float fy = (float)y / 64.0;
      float nx = (fx * 2.0 - 1.0);
      float ny = (fy * 2.0 - 1.0);

      // Apply matrix transformation (approximate mat4x2)
      float tx = nx * -0.8 + ny * 0.4 + t * 0.1;
      float ty = nx * 0.7 + ny * -0.7 + t * 0.1;

      // Radial fade
      float dotVal = tx * tx + ty * ty;
      float glow = sqrt(fmax(0.0, 0.9 - dotVal));

      // Color modulation
      uint8_t r = (uint8_t)(128 + 127 * sin(glow + t));
      uint8_t g = (uint8_t)(128 + 127 * sin(glow + t + 2));
      uint8_t b = (uint8_t)(128 + 127 * sin(glow + t + 4));

      dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
    }
  }
}






void RaymarchGlow(float t) {
  float centerX = 32.0;
  float centerY = 32.0;

  for (int y = 0; y < 64; y++) {
    for (int x = 0; x < 64; x++) {
      // Normalize coordinates
      float fx = (x - centerX) / 32.0;
      float fy = (y - centerY) / 32.0;

      // Rotation
      float angle = t * 0.5;
      float cosA = cos(angle);
      float sinA = sin(angle);

      float rx = fx * cosA - fy * sinA;
      float ry = fx * sinA + fy * cosA;

      // Distance field (approximate shape)
      float d = fmax(fmax(fabs(rx), fabs(ry)), fabs(rx + ry)) - 0.5;

      // Glow intensity
      float glow = fmax(0.0, 1.0 - d * 4.0);

      // Rim lighting (simulate dot product, adjusted for stronger effect)
      float rim = pow(fmax(0.0, rx * 0.0 + ry * 0.0 + 1.0), 3.0); // Increased exponent for sharper rim

      // Starry Night-inspired color palette
      // Base color oscillates with time for dynamic effect
      float colorPhase = sin(t * 0.3 + rx * 0.5 + ry * 0.5) * 0.5 + 0.5; // Smooth oscillation
      uint8_t r, g, b;
      if (colorPhase < 0.4) { // Deep blues (Starry Night sky)
        r = (uint8_t)((random(0, 50) / 25) * 25); // ~2 bits for low red
        g = (uint8_t)((random(50, 100) / 25) * 25); // ~3 bits
        b = (uint8_t)((random(100, 200) / 25) * 25); // ~3 bits
      } else if (colorPhase < 0.8) { // Bright yellows (stars)
        r = (uint8_t)((random(200, 256) / 36) * 36); // ~3 bits
        g = (uint8_t)((random(200, 256) / 36) * 36); // ~3 bits
        b = (uint8_t)((random(50, 100) / 25) * 25); // ~3 bits
      } else { // Whites (bright highlights)
        r = (uint8_t)((random(200, 256) / 36) * 36); // ~3 bits
        g = (uint8_t)((random(200, 256) / 36) * 36); // ~3 bits
        b = (uint8_t)((random(200, 256) / 36) * 36); // ~3 bits
      }

      // Modulate with glow and rim for luminosity
      r = (uint8_t)(r * glow * rim * 0.9 + 20); // Add slight base intensity
      g = (uint8_t)(g * glow * rim * 0.8 + 20);
      b = (uint8_t)(b * glow * rim * 0.7 + 20);

      // Clamp and quantize for 256-color palette
      r = (r / 36) * 36; // Snap to ~3 bits
      g = (g / 36) * 36; // Snap to ~3 bits
      b = (b / 25) * 25; // Snap to ~3 bits

      dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
    }
  }
}



 void SineChaosGlow(float t) {
  const int W = 64, H = 64;
  const float scale = 0.1;

  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      // Normalize coordinates to [-1, 1]
      float px = (2.0 * x - W) / H;
      float py = (2.0 * y - H) / H;

      // Warp coordinates
      float denom = 3.0 - py;
      float fx = px / scale / denom + t;
      float fy = py / scale / denom + t;

      // Nested sine chaos
      float sx = fx + sin(fy * 3.0);
      float sy = fy + sin(fx * 3.0);
      float dotVal = ceil(sx) * 17.0 + ceil(sy) * 79.0;
      float wave = sin(sin(dotVal) * 7000.0);

      // Color modulation
      float r = wave + 0.5;
      float g = sin(sin(dotVal) * 7000.0 + 1.0) + 0.5;
      float b = sin(sin(dotVal) * 7000.0 + 2.0) + 0.5;

      // Clamp and convert
      uint8_t R = (uint8_t)(fmin(1.0, fmax(0.0, r)) * 255);
      uint8_t G = (uint8_t)(fmin(1.0, fmax(0.0, g)) * 255);
      uint8_t B = (uint8_t)(fmin(1.0, fmax(0.0, b)) * 255);

      dma_display->drawPixel(x, y, dma_display->color565(R, G, B));
    }
  }
}
 
 

void GooGlow(float t) {
  const int W = 64, H = 64;
  const float scale = 6.0f;

  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      // Normalize coordinates to panel space
      float px = (float)x / H * scale;
      float py = (float)y / H * scale;

      // Swirl loop
      for (int i = 0; i < 8; i++) {
        px += sin(py + i + t * 0.3f);

        // Apply 2D rotation matrix
        float newX = px * 0.75f - py * 1.0f;
        float newY = px * 1.0f + py * 0.75f;
        px = newX;
        py = newY;
      }

      // Color modulation
      float r = sin(px * 0.3f + 0.0f) * 0.5f + 0.5f;
      float g = sin(px * 0.3f + 1.0f) * 0.5f + 0.5f;
      float b = sin(px * 0.3f + 2.0f) * 0.5f + 0.5f;

      // Clamp and convert
      uint8_t R = (uint8_t)(fmin(1.0f, fmax(0.0f, r)) * 255);
      uint8_t G = (uint8_t)(fmin(1.0f, fmax(0.0f, g)) * 255);
      uint8_t B = (uint8_t)(fmin(1.0f, fmax(0.0f, b)) * 255);

      dma_display->drawPixel(x, y, dma_display->color565(R, G, B));
    }
  }
}

 
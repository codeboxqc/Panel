#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "gfx.h"


 
 


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

 

//   Enhanced Pixel Rain with full color range
void pixelRainEffect(unsigned long time) { //ok
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







void rain16(unsigned long time) { //ok
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

 

 
void  Rain (unsigned long time) { //ok
    static struct Pixel {
        float x, y;
        float speed;
        uint8_t color;
        bool active;
    } pixels[60];
    
    static unsigned long lastSpawn = 0;
    static unsigned long lastFade = 0;
    
    // Fade the screen occasionally
    if (time - lastFade > 100) {
        lastFade = time;
        // Draw random black pixels to create fade effect
        for (int i = 0; i < 60; i++) {
            int x = random(WIDTH);
            int y = random(HEIGHT);
            dma_display->drawPixel(x, y, 0);
        }
    }
    
    // Spawn new pixels
    if (time - lastSpawn > 35) {
        lastSpawn = time;
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 60; j++) {
                if (!pixels[j].active) {
                    pixels[j].x = random(0, WIDTH);
                    pixels[j].y = 0;
                    pixels[j].speed = random(5, 15) * 0.1;
                    pixels[j].color = random(1, 16);
                    pixels[j].active = true;
                    break;
                }
            }
        }
    }
    
    // Update and draw pixels
    for (int i = 0; i <60; i++) {
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


// ====================================================
// fire flame
// ====================================================



 
void initFlame(Flame* f) {
    f->width = WIDTH;
    f->height = HEIGHT;
    f->buffer = (uint8_t*)malloc(f->width * f->height);
    f->fire_buffer = (uint8_t*)malloc(f->width * f->height);
    memset(f->buffer, 0, f->width * f->height);
    memset(f->fire_buffer, 0, f->width * f->height);
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

void drawFlame(Flame* f) { //ok
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
    plasma->width  = WIDTH;
    plasma->height = HEIGHT;
    plasma->body   = (uint8_t*)malloc(WIDTH * HEIGHT);
    memset(plasma->body, 0, WIDTH * HEIGHT);

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

void starfieldEffect(unsigned long time) { //ok
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

void drawMatrixRain(MatrixRain* rain) { //ok
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

void drawParticleSystem() { //ok
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
    static uint8_t colorMode = 0;
    static unsigned long lastModeChange = 0;
    
    // Update at ~30 FPS to reduce CPU/memory load
    if (time - lastUpdate < 10) return;
    lastUpdate = time;
    timeOffset += 0.05f;
    
    // Change color mode every 5 seconds
    if (time - lastModeChange > 5000*3) {
        lastModeChange = time;
        colorMode = (colorMode + 1) & 7; // Bitwise mod 8
    }
    
    const float halfW = WIDTH * 0.5f;
    const float halfH = HEIGHT * 0.5f;
    
    // Precompute time-based weights to reduce redundant calculations
    float weight1 = 0.3f + 0.2f * fastSin(timeOffset * 0.4f);
    float weight2 = 0.3f + 0.2f * fastSin(timeOffset * 0.6f + 1.57f);
    float weight3 = 0.3f + 0.2f * fastSin(timeOffset * 0.8f + 3.14f);
    float weightSum = weight1 + weight2 + weight3;
    
    for (int x = 0; x < WIDTH; x++) {
        float fx = (float)x - halfW;
        for (int y = 0; y < HEIGHT; y++) {
            float fy = (float)y - halfH;
            
            // Use only three patterns to reduce memory and computation
            float pattern1 = fastFractalNoise(fx * 0.1f, fy * 0.1f, timeOffset * 0.3f);
            float pattern2 = fastFractalCircles(fx, fy, timeOffset * 0.5f);
            float pattern3 = fastFractalSpiral(fx, fy, timeOffset * 0.7f);
            
            // Combine patterns with precomputed weights
            float fractal = (pattern1 * weight1 + pattern2 * weight2 + pattern3 * weight3) / weightSum;
            
            // Get color and draw pixel
            uint32_t color = getFractalColor(fractal, timeOffset, x, y, colorMode);
            dma_display->drawPixel(x, y, color);
        }
    }
}




///////////////////////////////////
// Predefined colors converted from hex to RGB tuples
static const uint8_t colors[10][3] = {
    {35, 87, 137},   // #235789
    {193, 41, 46},   // #c1292e
    {241, 211, 2},   // #f1d302
    {255, 255, 255}, // #ffffff
    {214, 122, 177}, // #d67ab1
    {255, 140, 66},  // #ff8c42
    {129, 193, 75},  // #81c14b
    {46, 147, 60},   // #2e933c
    {228, 87, 46},   // #e4572e
    {23, 190, 187}   // #17bebb
};

// Easing function: easeInOutExpo
float easeInOutExpo(float x) {
    if (x == 0.0f) return 0.0f;
    if (x == 1.0f) return 1.0f;
    if (x < 0.5f) return powf(2.0f, 20.0f * x - 10.0f) / 2.0f;
    return (2.0f - powf(2.0f, -20.0f * x + 10.0f)) / 2.0f;
}
 
// Helper function to draw filled rectangle optimized
void drawFilledRect(int x, int y, int w, int h, uint16_t color) {
   if (w <= 0 || h <= 0) return;
  
    dma_display->fillRect(x, y, w, h, color);  // Assume library supports
}





void vanGoghPaintAnimation(unsigned long time) {
    static unsigned long lastUpdate = 0;
    static float offset = 0.0f;
    static int drc[64] = {0};       // Direction for each cell (0-4)
    static int pdrc[64] = {0};      // Previous direction
    static float cx[64] = {0.0f};   // Current x center
    static float cy[64] = {0.0f};   // Current y center
    static float cx0[64] = {0.0f};  // Start x center
    static float cy0[64] = {0.0f};  // Start y center
    static float cx1[64] = {0.0f};  // Target x center
    static float cy1[64] = {0.0f};  // Target y center
    static float d[64] = {0.0f};    // Size of moving rectangle
    static int t[64] = {0};         // Animation time
    static const int t1 = 50;       // Animation duration (frames)
    static uint16_t cols[64][4];    // Pre-converted colors for each cell
    static bool initialized = false;
    static bool needsFullRedraw = true;

    // Initialize the grid (runs once)
    if (!initialized) {
        const int c = 8; // 8x8 grid
        const float w = WIDTH / (float)c;
        int idx = 0;
        for (int i = 0; i < c; i++) {
            for (int j = 0; j < c; j++) {
                cx[idx] = i * w + w / 2.0f;
                cy[idx] = j * w + w / 2.0f;
                cx0[idx] = cx[idx];
                cy0[idx] = cy[idx];
                
                // Randomize initial colors and pre-convert to 16-bit
                int colorIndices[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
                for (int k = 9; k > 0; k--) {
                    int swapIdx = rand() % (k + 1);
                    int temp = colorIndices[k];
                    colorIndices[k] = colorIndices[swapIdx];
                    colorIndices[swapIdx] = temp;
                }
                for (int k = 0; k < 4; k++) {
                    cols[idx][k] = dma_display->color565(
                        colors[colorIndices[k]][0],
                        colors[colorIndices[k]][1],
                        colors[colorIndices[k]][2]
                    );
                }
                idx++;
            }
        }
        initialized = true;
    }

    
    if (time - lastUpdate > 12) {
        lastUpdate = time;
        offset += 0.02f;

        //  clear screen  
        if (needsFullRedraw) {
            for (int x = 0; x < WIDTH; x++) {
                for (int y = 0; y < HEIGHT; y++) {
                   // dma_display->drawPixel(x, y, 0); // Black background
                }
            }
            needsFullRedraw = false;
        }

        const int c = 8;
        const float w = WIDTH / (float)c;
        int idx = 0;
        bool anyMovementStarted = false;

        for (int i = 0; i < c; i++) {
            for (int j = 0; j < c; j++) {
                float prevCx = cx[idx];
                float prevCy = cy[idx];
                
                // Update movement
                if (t[idx] >= 0 && t[idx] < t1) {
                    float n = t[idx] / (float)t1;
                    cx[idx] = cx0[idx] + (cx1[idx] - cx0[idx]) * easeInOutExpo(n);
                    cy[idx] = cy0[idx] + (cy1[idx] - cy0[idx]) * easeInOutExpo(n);
                }
                if (t[idx] >= t1) {
                    // Initialize new movement
                    pdrc[idx] = drc[idx];
                    do {
                        drc[idx] = rand() % 5;
                    } while (drc[idx] == pdrc[idx]);
                    d[idx] = w * ((rand() % 35 + 40) / 100.0f); // Random 0.4 to 0.75
                    if (drc[idx] == 0) {
                        cx1[idx] = i * w + w / 2.0f + (w / 2.0f - d[idx] / 2.0f);
                        cy1[idx] = j * w + w / 2.0f + (w / 2.0f - d[idx] / 2.0f);
                    } else if (drc[idx] == 1) {
                        cx1[idx] = i * w + w / 2.0f - (w / 2.0f - d[idx] / 2.0f);
                        cy1[idx] = j * w + w / 2.0f + (w / 2.0f - d[idx] / 2.0f);
                    } else if (drc[idx] == 2) {
                        cx1[idx] = i * w + w / 2.0f + (w / 2.0f - d[idx] / 2.0f);
                        cy1[idx] = j * w + w / 2.0f - (w / 2.0f - d[idx] / 2.0f);
                    } else if (drc[idx] == 3) {
                        cx1[idx] = i * w + w / 2.0f - (w / 2.0f - d[idx] / 2.0f);
                        cy1[idx] = j * w + w / 2.0f - (w / 2.0f - d[idx] / 2.0f);
                    } else if (drc[idx] == 4) {
                        cx1[idx] = i * w + w / 2.0f;
                        cy1[idx] = j * w + w / 2.0f;
                    }
                    cx0[idx] = cx[idx];
                    cy0[idx] = cy[idx];
                    t[idx] = 0;
                    anyMovementStarted = true;
                }
                t[idx]++;

                // Only redraw this cell if position changed significantly or movement just started
                if (abs((int)cx[idx] - (int)prevCx) > 0 || abs((int)cy[idx] - (int)prevCy) > 0 || anyMovementStarted || needsFullRedraw) {
                    // Clear the cell area first (draw background)
                    int cellX = (int)(i * w);
                    int cellY = (int)(j * w);
                    int cellW = (int)w + 1; // Add 1 to prevent gaps
                    int cellH = (int)w + 1;
                    
                    // Clear cell background
                    drawFilledRect(cellX, cellY, cellW, cellH, 0);
                    
                    // Calculate rectangle positions
                    float xx = i * w + w / 2.0f - w / 2.0f;
                    float yy = j * w + w / 2.0f - w / 2.0f;
                    float ww = cx[idx] - xx;
                    float hh = cy[idx] - yy;
                    float off = w * 0.1f;

                    // Draw four rectangles using optimized rectangle drawing
                    // Top-left rectangle
                    int x1 = (int)(xx + off / 2);
                    int y1 = (int)(yy + off / 2);
                    int w1 = (int)(ww - off / 2);
                    int h1 = (int)(hh - off / 2);
                    if (w1 > 0 && h1 > 0) {
                        drawFilledRect(x1, y1, w1, h1, cols[idx][0]);
                    }

                    // Top-right rectangle
                    int x2 = (int)(xx + ww + off / 2);
                    int y2 = (int)(yy + off / 2);
                    int w2 = (int)(w - ww - off);
                    int h2 = (int)(hh - off);
                    if (w2 > 0 && h2 > 0) {
                        drawFilledRect(x2, y2, w2, h2, cols[idx][1]);
                    }

                    // Bottom-right rectangle
                    int x3 = (int)(cx[idx] + off / 2);
                    int y3 = (int)(cy[idx] + off / 2);
                    int w3 = (int)(w - ww - off);
                    int h3 = (int)(w - hh - off);
                    if (w3 > 0 && h3 > 0) {
                        drawFilledRect(x3, y3, w3, h3, cols[idx][2]);
                    }

                    // Bottom-left rectangle
                    int x4 = (int)(xx + off / 2);
                    int y4 = (int)(yy + hh + off / 2);
                    int w4 = (int)(ww - off);
                    int h4 = (int)(w - hh - off);
                    if (w4 > 0 && h4 > 0) {
                        drawFilledRect(x4, y4, w4, h4, cols[idx][3]);
                    }
                }
                
                idx++;
            }
        }
    }
}


  


 


 

void Jetstream(float t) {//ok
 
  
  const int W = 64, H = 64;
  const int numLines = 36;
  const float radius = 28.0f;
  const float centerX = W / 2;
  const float centerY = H / 2;

   dma_display->fillScreen(0);

  for (int i = 0; i < numLines; i++) {
    float angle = (TWO_PI / numLines) * i + t * 0.5f;
    float x1 = centerX + radius * cosf(angle);
    float y1 = centerY + radius * sinf(angle);
    float x2 = centerX + radius * cosf(angle + PI);
    float y2 = centerY + radius * sinf(angle + PI);

    // Color cycling
    uint8_t hue = (uint8_t)((i * 7 + t * 40)) % 256;
    uint8_t r = sinf(hue * 0.024f + 0.0f) * 127 + 128;
    uint8_t g = sinf(hue * 0.024f + 2.0f) * 127 + 128;
    uint8_t b = sinf(hue * 0.024f + 4.0f) * 127 + 128;
    uint16_t color = dma_display->color565(r, g, b);

     dma_display->drawPixel((int)x1/2+16, (int)y1/2+16,color );
     

    // Draw radial line 
    int mx = (int)((x1 + x2) / 2);
    int my = (int)((y1 + y2) / 2);
    dma_display->drawPixel((int)x1, (int)y1, color);
    dma_display->drawPixel((int)x2, (int)y2, color);

    

    dma_display->drawPixel(mx, my, color);
     
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


void Pillars(float t) { //ok
    const float invSize = 1.0f / 64.0f;
    const float scale = 1.0f / 0.3f;
    const float speed = 2.0f / PI;

    for (int y = 0; y < 64; y++) {
        float py = (float)y * invSize;
        float nyBase = (py * 2.0f - 1.0f) * scale + t;

        for (int x = 0; x < 64; x++) {
            float px = (float)x * invSize;
            float nx = (px * 2.0f - 1.0f) * scale + t * speed;

            float ny = nyBase;
            float wx = fmodf(nx, 2.0f) - 1.0f;

            float arc = sqrtf(fmaxf(0.0f, 1.0f - wx * wx));

            float phase = ny - arc * cosf(ceilf(nx * 0.5f) * PI);

            // Extra colorful cycling
            float hueShift = t * 2.0f;  // faster shifting
            uint8_t r = (uint8_t)(127 + 128 * sinf(phase + hueShift * 0.7f));
            uint8_t g = (uint8_t)(127 + 128 * sinf(phase + 2.0f + hueShift * 1.1f));
            uint8_t b = (uint8_t)(127 + 128 * sinf(phase + 4.0f + hueShift * 1.7f));

            dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
        }
    }
}



  
void RadialGlow(uint32_t time) {
  const int W = 64, H = 64;
  static uint8_t heat[W][H];
  static uint32_t lastUpdate = 0;

  if (time - lastUpdate < 30) return;
  lastUpdate = time;

  // Seed edges
  for (int x = 0; x < W; x++) {
    heat[x][0] = 180 + (rand() & 75); // Faster than random(180, 256)
    heat[x][H - 1] = 180 + (rand() & 75);
  }
  for (int y = 0; y < H; y++) {
    heat[0][y] = 180 + (rand() & 75);
    heat[W - 1][y] = 180 + (rand() & 75);
  }

  // Propagate inward with stronger central fade
  for (int x = 1; x < W - 1; x++) {
    for (int y = 1; y < H - 1; y++) {
      int avg = (heat[x - 1][y] + heat[x + 1][y] + heat[x][y - 1] + heat[x][y + 1]) >> 2;
      int dx = x < W/2 ? x : W - 1 - x;
      int dy = y < H/2 ? y : H - 1 - y;
      int dist = (dx + dy) >> 1; // Simplified distance approximation
      int atten = (255 - dist * 25) * (255 - dist * 25) >> 8; // Quadratic falloff for center fade
      heat[x][y] = (avg * atten >> 8) + ((rand() & 31) - 15); // Faster random noise
      if (heat[x][y] > 255) heat[x][y] = 255;
      if (heat[x][y] < 0) heat[x][y] = 0;
    }
  }

  // Draw pixels
  for (int x = 0; x < W; x++) {
    for (int y = 0; y < H; y++) {
      uint8_t v = heat[x][y];
      if (v > 20) { // Skip low-intensity pixels
        uint16_t r = (v * 5) >> 2; // Faster multiplication
        uint16_t g = (v * 4) / 5;
        uint16_t b = v / 10;
        if (r > 255) r = 255-random(0,9);
        if (g > 200) g = 200+random(0,9);
        if (b > 50) b = 50;
        dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
      } else {
        dma_display->drawPixel(x, y, 0); // Clear dim pixels for fade effect
      }
    }
  }

  // Corner bursts
  if ((rand() % 100) < 15) {
    int c = rand() & 3;
    int sx = (c & 1) ? W - 3 : 0;
    int sy = (c & 2) ? H - 3 : 0;
    for (int x = sx; x < sx + 3; x++) {
      for (int y = sy; y < sy + 3; y++) {
        heat[x][y] = 200 + (rand() & 55); // Faster random
      }
    }
  }
}

 



float lerp(float a, float b, float amt) {
  return a + (b - a) * amt;
}


void RaymarchGlow(float t) {  //ok
  
  const int grid = 6;
  const int maxForms = grid * grid;
  const float cellSize = WIDTH / grid;

  static bool initialized = false;
  static struct Form {
    float x, y, d, r, s, ang;
    int n, t;
    uint16_t c0[6], cc[6], dcl;
  } forms[maxForms];

  if (!initialized) {
    for (int i = 0; i < grid; i++) {
      for (int j = 0; j < grid; j++) {
        int idx = i * grid + j;
        float x = i * cellSize + cellSize / 2;
        float y = j * cellSize + cellSize / 2;
        int n = 2 + rand() % 5;
        forms[idx] = {x, y, cellSize * 0.75f, cellSize * 0.375f, cellSize * 0.3f, 0.0f, n, -60};

        for (int k = 0; k < n; k++) {
          uint8_t h = rand() % 256;
          uint8_t r = sin(h * 0.024f + 0.0f) * 127 + 128;
          uint8_t g = sin(h * 0.024f + 2.0f) * 127 + 128;
          uint8_t b = sin(h * 0.024f + 4.0f) * 127 + 128;
          forms[idx].c0[k] = dma_display->color565(r, g, b);
          forms[idx].cc[k] = forms[idx].c0[k];
        }

        uint8_t h = rand() % 256;
        uint8_t r = sin(h * 0.024f + 0.0f) * 127 + 128;
        uint8_t g = sin(h * 0.024f + 2.0f) * 127 + 128;
        uint8_t b = sin(h * 0.024f + 4.0f) * 127 + 128;
        forms[idx].dcl = dma_display->color565(r, g, b);
      }
    }
    initialized = true;
  }

  dma_display->fillScreen(0);

  for (int i = 0; i < maxForms; i++) {
    Form &f = forms[i];
    int t1 = 30, t2 = t1 + 40, t3 = t2 + 30, t4 = t3 + 60;

    // Animate
    if (f.t > 0 && f.t < t1) {
      float amt = (float)(f.t) / (t1 - 1);
      float ease = amt * amt * amt;
      f.ang = ease * TWO_PI;
      f.r = lerp(f.d / 2, 0, ease);
    }
    if (f.t > t1 && f.t < t2) {
      float amt = (float)(f.t - t1) / (t2 - t1 - 1);
      float ease = pow(2, -10 * amt) * sin((amt * 10 - 0.75f) * ((2 * PI) / 3)) + 1;
      f.s = lerp(f.d * 0.3f, f.d * 0.5f, ease);
    }
    if (f.t > t3 && f.t < t4) {
      float amt = (float)(f.t - t3) / (t4 - t3 - 1);
      float ease = pow(2, -10 * amt) * sin((amt * 10 - 0.75f) * ((2 * PI) / 3)) + 1;
      f.s = lerp(f.d * 0.5f, f.d * 0.3f, ease);
      f.r = lerp(0, f.d / 2, ease);
    }
    if (f.t > t4) f.t = -60;
    f.t++;

    // Draw
    for (int k = 0; k < f.n; k++) {
      float a = (TWO_PI / f.n) * k + f.ang;
      int px = (int)(f.x + f.r * cosf(a));
      int py = (int)(f.y + f.r * sinf(a));
      dma_display->drawPixel(px, py, f.cc[k]);
    }
  }
}





 
  



 
  void TronMatrixPulse(float t) { //ok
  
  
  const int MAX_TYPES = 4;
  const int MAX_POINTS = 8;
  const float maxSpeed = 0.3;
  const float influence = 0.03;

  static bool initialized = false;
  static struct Point {
    float x, y, dx, dy;
    uint16_t color;
  } points[MAX_TYPES][MAX_POINTS];

  // Initialize once
  if (!initialized) {
    for (int type = 0; type < MAX_TYPES; type++) {
      uint8_t r = sin(type * 1.2) * 127 + 128;
      uint8_t g = sin(type * 1.2 + 2.0) * 127 + 128;
      uint8_t b = sin(type * 1.2 + 4.0) * 127 + 128;
      uint16_t c = dma_display->color565(r, g, b);

      for (int i = 0; i < MAX_POINTS; i++) {
        points[type][i] = {
          (float)(rand() % WIDTH),
          (float)(rand() % HEIGHT),
          0.0f,
          0.0f,
          c
        };
      }
    }
    initialized = true;
  }

  dma_display->fillScreen(0); // Clear panel

  for (int type = 0; type < MAX_TYPES; type++) {
    for (int i = 0; i < MAX_POINTS; i++) {
      Point &p = points[type][i];
      float fx = 0, fy = 0;

      for (int j = 0; j < MAX_POINTS; j++) {
        if (i == j) continue;
        Point &other = points[type][j];
        float dx = other.x - p.x;
        float dy = other.y - p.y;
        float dist = sqrtf(dx * dx + dy * dy) + 0.001;
        fx += dx / dist;
        fy += dy / dist;
      }

      // Apply influence
      p.dx += fx * influence;
      p.dy += fy * influence;

      // Limit speed
      float mag = sqrtf(p.dx * p.dx + p.dy * p.dy);
      if (mag > maxSpeed) {
        p.dx *= maxSpeed / mag;
        p.dy *= maxSpeed / mag;
      }

      // Update position
      p.x += p.dx;
      p.y += p.dy;

      // Wrap around
      if (p.x < 0) p.x +=  WIDTH;
      if (p.x >=  WIDTH) p.x -=  WIDTH;
      if (p.y < 0) p.y += HEIGHT;
      if (p.y >= HEIGHT) p.y -= HEIGHT;

      // Draw pixel
      dma_display->drawPixel((int)p.x, (int)p.y, p.color);
       
    }
  }
 

}

void GooGlow(float t) { //ok
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













void fireAnimation(unsigned long time) { //ok
    static uint16_t heat  [WIDTH][HEIGHT];
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
        if (random(100) < 20) {
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



 // 1. Enhanced Plasma Effect with full color range
void plasmaEffect(unsigned long time) { //ok
    static float offset = 0.0f;
    offset += 0.02f;  // Animate

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            
            float val = fastSin(x * 0.1f + offset) + fastCos(y * 0.1f + offset * 0.7f) +
                        fastSin((x + y) * 0.05f + offset * 1.3f);
            uint8_t col = (uint8_t)((val + 3.0f) / 6.0f * 15.0f) + 1;
             
            dma_display->drawPixel(x, y, fastHSVtoRGB((float)col / 16.0f, 1.0f, 1.0f));

            
        }
    }
}




 
  
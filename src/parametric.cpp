#include <Arduino.h>
#include <math.h>
#include <string.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "gfx.h"

// Display dimensions - adjust for your LED matrix

#define CENTER_X (WIDTH/2)
#define CENTER_Y (HEIGHT/2)

// Animation parameters
static float t = 0.0f;
static int currentCurve = 0;
static int nextCurve = 1;
static float transitionProgress = 0.0f;
static unsigned long lastChangeTime = 0;
static const float animationSpeed = 0.05f;
static const int maxRadius = min(WIDTH, HEIGHT) * 0.45f;

// Color palettes (HSV converted to RGB565)
struct ColorPalette {
    uint16_t colors[3];
};

static ColorPalette colorPalettes[] = {
    {{0xF800, 0xFBE0, 0xFFE0}}, // Fire: Red, Orange, Yellow
    {{0xFBE0, 0xFDA0, 0xFEC0}}, // Sun: Warm colors
    {{0x07FF, 0x05FF, 0x03FF}}, // Cold: Cyan, Light Blue, Ice Blue
    {{0x781F, 0xF81F, 0xFC1F}}, // Plasma: Purple, Magenta, Pink
    {{0x07E0, 0x07F0, 0x05E0}}, // Forest: Green, Teal
    {{0xFC00, 0x8400, 0x6C00}}, // Earth: Browns
    {{0x001F, 0x201F, 0x401F}}, // Cosmic: Deep Blues
    {{0xF800, 0x07E0, 0x001F}}, // Primary: RGB
    {{0xFD20, 0x87E0, 0xF81F}}  // Neon: Bright colors
};




static const int numPalettes = sizeof(colorPalettes) / sizeof(ColorPalette);

// Particle system
struct Parti  {
    float x, y;
    float speed;
    float offset;
    uint16_t color;
    float size;
    float life;
    float birthTime;
    int shapeType;
};

 Parti  par[33];
 const int numParticles = 33;
 

static float fastCos(float x) {
    return fastSin(x + PI/2);
}

// HSV to RGB565 conversion
uint16_t hsvToRgb565(float h, float s, float v) {
    h = fmod(h, 360.0f);
    if (h < 0) h += 360.0f;
    
    float c = v * s;
    float x = c * (1.0f - abs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    
    float r, g, b;
    
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    r = (r + m) * 255;
    g = (g + m) * 255;
    b = (b + m) * 255;
    
    return fastRGB565((uint8_t)r, (uint8_t)g, (uint8_t)b);
}



// Simple map function
float maper(float value, float inMin, float inMax, float outMin, float outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

// Constrain function
float constraint(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Plasma color generation
uint16_t getPlasmaColor(float x, float y, float time) {
    float scale = 0.1f;
    x *= scale;
    y *= scale;
    time *= 0.1f;
    
    // Simplified plasma equation
    float val = sin(x + time) + sin(y + time) + 
                sin((x + y + time) * 0.5f) + 
                sin(sqrt(x*x + y*y) + time);
    
    val = (val + 4.0f) / 8.0f; // Normalize to [0,1]
    float hue = val * 360.0f + time * 50.0f;
    
    return hsvToRgb565(hue, 0.8f, 0.9f);
}

// Calculate parametric curve point
void calculateCurvePoint(float theta, float radius, int curveType, float *outX, float *outY) {
    float x = 0, y = 0;
    float tFactor = t * 0.8f;
    float r=0.0f;
    
    switch(curveType % 72) {
        case 0: // Rose curve
            x = radius * fastCos(3 * theta + t) * fastCos(theta);
            y = radius * fastSin(3 * theta + t) * fastSin(theta);
            break;
            
        case 1: // Spiral
            {
                float spiral = radius * (0.6f + 0.3f * sin(theta/8 + t));
                x = spiral * fastCos(theta) + radius/5 * fastCos(9 * theta + t);
                y = spiral * fastSin(theta) - radius/5 * fastSin(7 * theta - t);
            }
            break;
            
        case 2: // Lissajous
            x = radius * fastSin(theta * 2 + t) * fastCos(theta * 3 - t/2);
            y = radius * fastCos(theta * 3 - t) * fastSin(theta * 2 + t/3);
            break;
            
        case 3: // Modulated circle
            x = radius * fastCos(theta) * (1 + 0.25f * fastSin(7 * theta + t));
            y = radius * fastSin(theta) * (1 + 0.25f * fastCos(5 * theta - t));
            break;
            
        case 4: // Vortex
            {
                float vortex = radius * (0.3f + 0.5f * pow(fastSin(theta/2 + t), 2));
                x = vortex * fastCos(theta + 5 * fastSin(theta/3 + t/4));
                y = vortex * fastSin(theta + 5 * fastCos(theta/4 - t/5));
            }
            break;
            
        case 5: // Complex wave
            x = radius * fastSin(3 * theta + t) * fastCos(2 * theta - t);
            y = radius * fastCos(4 * theta - t) * fastSin(5 * theta + t);
            break;
            
        case 6: // Branching pattern
            {
                float branch = radius * 0.6f;
                x = branch * fastCos(theta) * (1 + 0.3f * fastSin(13 * theta + t));
                y = branch * fastSin(theta) * (1 + 0.3f * fastCos(11 * theta - t));
            }
            break;
            
        case 7: // Orbital motion
            {
                float orbital = radius * (0.7f + 0.2f * fastSin(theta * 2 + t));
                x = orbital * fastCos(theta + fastSin(theta * 7 + t));
                y = orbital * fastSin(theta + fastCos(theta * 6 - t));
            }
            break;
            
        case 8: // Hypocycloid
            {
                float a = radius * 0.6f, b = radius * 0.2f;
                x = (a + b) * fastCos(theta) - b * fastCos((a/b + 1) * theta + t);
                y = (a + b) * fastSin(theta) - b * fastSin((a/b + 1) * theta + t);
            }
            break;
            
        case 9: // Epicycloid
            {
                float c = radius * 0.7f, d = radius * 0.175f;
                x = (c - d) * fastCos(theta) + d * fastCos((c/d - 1) * theta - t);
                y = (c - d) * fastSin(theta) - d * fastSin((c/d - 1) * theta - t);
            }
            break;
            
        case 10: // Butterfly curve
            {
                float r = radius * 0.5f * exp(fastCos(theta + t)) - 2 * fastCos(4 * theta) - pow(fastSin(theta/12), 5);
                x = r * fastSin(theta);
                y = r * fastCos(theta);
            }
            break;
            
        case 11: // Cardioid
            {
                float card = radius * (1 + fastCos(theta + t));
                x = card * fastCos(theta);
                y = card * fastSin(theta);
            }
            break;
            
        case 12: // Limacon
            {
                float lem = radius * (0.5f + fastCos(theta + t));
                x = lem * fastCos(theta);
                y = lem * fastSin(theta);
            }
            break;
            
        case 13: // Astroid
            x = radius * pow(fastCos(theta + t), 3);
            y = radius * pow(fastSin(theta + t), 3);
            break;
            
        case 14: // Cycloid
            {
                float a = radius * 0.5f;
                x = a * (theta - fastSin(theta + t));
                y = a * (1 - fastCos(theta + t));
            }
            break;
            
        case 15: // Trochoid
            {
                float a = radius * 0.6f, b = radius * 0.4f;
                x = a * theta - b * fastSin(theta + t);
                y = a - b * fastCos(theta + t);
            }
            break;
            
        case 16: // Folium
            {
                float denom = 1 + pow(tan(theta + t), 3);
                if (abs(denom) > 0.01f) {
                    x = radius * 3 * fastCos(theta) / denom;
                    y = radius * 3 * fastSin(theta) * fastCos(theta + t) / denom;
                }
            }
            break;
            
        case 17: // Logarithmic spiral
            {
                float r = radius * 0.3f * exp(0.1f * theta);
                x = r * fastCos(theta + t);
                y = r * fastSin(theta + t);
            }
            break;
            
        case 18: // Archimedean spiral
            {
                float r = radius * 0.02f * theta;
                x = r * fastCos(theta + t);
                y = r * fastSin(theta + t);
            }
            break;

            case 19: // Nephroid
            {
                float a = radius * 0.3f;
                x = a * 3 * fastCos(theta) - a * fastCos(3 * theta + t);
                y = a * 3 * fastSin(theta) - a * fastSin(3 * theta + t);
            }
            break;
            
        case 20: // Deltoid
            {
                float a = radius * 0.4f;
                x = a * 2 * fastCos(theta) + a * fastCos(2 * theta + t);
                y = a * 2 * fastSin(theta) - a * fastSin(2 * theta + t);
            }
            break;
            
        case 21: // Epitrochoid
            {
                float R = radius * 0.4f, r = radius * 0.2f, d = radius * 0.3f;
                x = (R + r) * fastCos(theta) - d * fastCos((R + r) * theta / r + t);
                y = (R + r) * fastSin(theta) - d * fastSin((R + r) * theta / r + t);
            }
            break;
            
        case 22: // Hypotrochoid
            {
                float R = radius * 0.5f, r = radius * 0.15f, d = radius * 0.25f;
                x = (R - r) * fastCos(theta) + d * fastCos((R - r) * theta / r - t);
                y = (R - r) * fastSin(theta) - d * fastSin((R - r) * theta / r - t);
            }
            break;
            
        case 23: // Cissoid
            {
                float a = radius * 0.4f;
                float denom = 1 - fastCos(theta + t);
                if (abs(denom) > 0.01f) {
                    x = a * 2 * fastSin(theta) * fastSin(theta + t) / denom;
                    y = a * 2 * fastSin(theta) * fastSin(theta) * fastSin(theta + t) / denom;
                }
            }
            break;
            
        case 24: // Conchoid
            {
                float a = radius * 0.3f, b = radius * 0.4f;
                x = a + b * fastCos(theta + t);
                y = a * tan(theta + t) + b * fastSin(theta + t);
            }
            break;
            
        case 25: // Kappa curve
            {
                float a = radius * 0.5f;
                x = a * fastCos(theta + t) * tan(theta);
                y = a * fastSin(theta + t);
            }
            break;
            
        case 26: // Serpentine
            {
                float a = radius * 0.4f, b = radius * 0.3f;
                x = a / tan(theta + t);
                y = b * fastSin(theta + t) * fastCos(theta + t);
            }
            break;
            
        case 27: // Witch of Agnesi
            {
                float a = radius * 0.5f;
                float denom = 1 + theta * theta;
                x = a * 2 * theta / denom;
                y = a * 2 / denom;
            }
            break;
            
        case 28: // Tractrix
            {
                float a = radius * 0.4f;
                x = a * (log(tan(theta/2 + PI/4 + t*0.1f)) - fastSin(theta + t));
                y = a * fastCos(theta + t);
            }
            break;
            
        case 29: // Catenary
            {
                float a = radius * 0.3f;
                x = theta * a;
                y = a * (exp((theta + t)/a) + exp(-(theta + t)/a)) / 2;
            }
            break;
            
        case 30: // Clothoid (Euler spiral)
            {
                float s = theta * 0.2f + t * 0.3f;
                x = radius * 0.4f * cos(s * s);
                y = radius * 0.4f * sin(s * s);
            }
            break;
            
        case 31: // Cornu spiral
            {
                float s = sqrt(theta + t);
                x = radius * 0.5f * cos(PI * s * s / 2);
                y = radius * 0.5f * sin(PI * s * s / 2);
            }
            break;
            
        case 32: // Devil's curve
            {
                float a = radius * 0.4f, b = radius * 0.6f;
                float y2 = b * b - a * a * fastSin(theta + t) * fastSin(theta + t);
                if (y2 >= 0) {
                    y = sqrt(y2);
                    x = a * fastCos(theta + t);
                }
            }
            break;
            
        case 33: // Eight curve
            {
                float a = radius * 0.5f;
                x = a * fastSin(theta + t);
                y = a * fastSin(theta + t) * fastCos(theta + t);
            }
            break;
            
        case 34: // Kampyle of Eudoxus
            {
                float a = radius * 0.4f;
                x = a / (1 + theta * theta);
                y = a * theta / (1 + theta * theta);
            }
            break;
            
        case 35: // Lemniscate of Bernoulli
            {
                float a = radius * 0.5f;
                float denom = 1 + pow(fastSin(theta + t), 4);
                x = a * sqrt(2) * fastCos(theta + t) / sqrt(denom);
                y = a * sqrt(2) * fastSin(theta + t) * fastCos(theta + t) / sqrt(denom);
            }
            break;
            
        case 36: // Lemniscate of Gerono
            {
                float a = radius * 0.5f;
                x = a * fastCos(theta + t);
                y = a * fastSin(theta + t) * fastCos(theta + t);
            }
            break;
            
        case 37: // Piriform
            {
                float a = radius * 0.4f, b = radius * 0.3f;
                x = a * (1 + fastSin(theta + t));
                y = b * fastCos(theta + t) * (1 + fastSin(theta + t));
            }
            break;
            
        case 38: // Quadrifolium
            {
                float a = radius * 0.5f;
                float r = a * fastCos(2 * theta + t);
                x = r * fastCos(theta);
                y = r * fastSin(theta);
            }
            break;
            
        case 39: // Rhodonea (rose)
            {
                float a = radius * 0.5f;
                int k = 5; // 5-petaled rose
                float r = a * fastCos(k * theta + t);
                x = r * fastCos(theta);
                y = r * fastSin(theta);
            }
            break;
            
        case 40: // Scarabaeus
            {
                float a = radius * 0.4f, b = radius * 0.6f;
                float r = b * fastCos(2 * theta + t) - a * fastCos(theta + t);
                x = r * fastCos(theta);
                y = r * fastSin(theta);
            }
            break;
            
        case 41: // Semicubical parabola
            {
                float a = radius * 0.3f;
                x = a * theta * theta;
                y = a * theta * theta * theta;
            }
            break;
            
        case 42: // Strophoid
            {
                float a = radius * 0.4f;
                float denom = fastCos(2 * theta + t);
                if (abs(denom) > 0.01f) {
                    x = a * (1 - tan(theta + t) * tan(theta + t)) / denom;
                    y = a * tan(theta + t) * (1 - tan(theta + t) * tan(theta + t)) / denom;
                }
            }
            break;
            
        case 43: // Trisectrix
            {
                float a = radius * 0.4f;
                x = a * (1 + 2 * fastCos(theta + t)) * fastCos(theta);
                y = a * (1 + 2 * fastCos(theta + t)) * fastSin(theta);
            }
            break;
            
        case 44: // Viviani curve
            {
                float a = radius * 0.4f;
                x = a * (1 + fastCos(theta + t));
                y = a * fastSin(theta + t);
            }
            break;
            
        case 45: // Watt's curve
            {
                float a = radius * 0.3f, b = radius * 0.4f, c = radius * 0.5f;
                x = (a + b * fastCos(theta + t)) * fastCos(theta);
                y = (a + b * fastCos(theta + t)) * fastSin(theta);
            }
            break;
            
        case 46: // Bicorn
            {
                float a = radius * 0.4f;
                float denom = 2 - fastCos(theta + t);
                x = a * fastSin(theta + t);
                y = a * fastCos(theta + t) * (2 + fastCos(theta + t)) / denom;
            }
            break;
            
        case 47: // Cocked hat
            {
                float a = radius * 0.4f;
                x = a * fastSin(theta + t) * (exp(fastCos(theta + t)) - 2 * fastCos(4 * theta) - pow(fastSin(theta/12), 5));
                y = a * fastCos(theta + t) * (exp(fastCos(theta + t)) - 2 * fastCos(4 * theta) - pow(fastSin(theta/12), 5));
            }
            break;
            
        case 48: // Cross curve
            {
                float a = radius * 0.4f;
                x = a / fastCos(theta + t);
                y = a * tan(theta + t);
            }
            break;
            
        case 49: // Cruciform
            {
                float a = radius * 0.4f, b = radius * 0.3f;
                x = a / sqrt(1 + tan(theta + t) * tan(theta + t));
                y = b / sqrt(1 + 1/(tan(theta + t) * tan(theta + t)));
            }
            break;
            
        case 50: // Dipole curve
            {
                float a = radius * 0.4f;
                x = a * fastCos(theta + t) / (1 + fastSin(theta + t) * fastSin(theta + t));
                y = a * fastSin(theta + t) * fastCos(theta + t) / (1 + fastSin(theta + t) * fastSin(theta + t));
            }
            break;
            
        case 51: // Fish curve
            {
                float a = radius * 0.4f;
                x = a * fastCos(theta + t) - a/2 * fastCos(2 * theta + t);
                y = a * fastSin(theta + t) - a/2 * fastSin(2 * theta + t);
            }
            break;
            
        case 52: // Heart curve
            {
                float a = radius * 0.3f;
                x = a * 16 * pow(fastSin(theta + t), 3);
                y = a * (13 * fastCos(theta + t) - 5 * fastCos(2 * theta + t) - 2 * fastCos(3 * theta + t) - fastCos(4 * theta + t));
            }
            break;
            
        case 53: // Hippopede
            {
                float a = radius * 0.4f, b = radius * 0.6f;
                float temp = b * b - a * a * fastSin(theta + t) * fastSin(theta + t);
                if (temp >= 0) {
                    float r = sqrt(temp);
                    x = r * fastCos(theta);
                    y = a * fastSin(theta + t);
                }
            }
            break;
            
        case 54: // Maclaurin trisectrix
            {
                float a = radius * 0.4f;
                x = a * (3 * fastCos(theta + t) - fastCos(3 * theta + t));
                y = a * (3 * fastSin(theta + t) - fastSin(3 * theta + t));
            }
            break;
            
        case 55: // Maltese cross
            {
                float a = radius * 0.4f;
                x = a * 2 * fastCos(theta + t) / (1 + pow(fastSin(theta + t), 4));
                y = a * 2 * fastSin(theta + t) / (1 + pow(fastCos(theta + t), 4));
            }
            break;
            
        case 56: // Ophiuride
            {
                float a = radius * 0.3f, b = radius * 0.4f;
                x = (a + b * fastSin(theta + t)) * fastSin(theta) / theta;
                y = (a + b * fastSin(theta + t)) * fastCos(theta) / theta;
            }
            break;
            
        case 57: // Pisces
            {
                float a = radius * 0.4f;
                x = a * fastCos(theta + t) * (1 - fastCos(theta + t));
                y = a * fastSin(theta + t) * (1 - fastCos(theta + t));
            }
            break;
            
        case 58: // Swastika curve
            {
                float a = radius * 0.4f;
                x = a * (fastCos(theta + t) - fastSin(theta + t)) / (fastCos(theta + t) + fastSin(theta + t));
                y = a * (fastCos(theta + t) + fastSin(theta + t)) / (fastCos(theta + t) - fastSin(theta + t));
            }
            break;
            
        case 59: // Teardrop
            {
                float a = radius * 0.4f;
                x = a * fastCos(theta + t) * (fastSin(theta + t) * fastSin(theta + t) + 1);
                y = a * fastSin(theta + t) * (fastSin(theta + t) * fastSin(theta + t) + 1);
            }
            break;
            
        case 60: // Three-leaf rose
            {
                float a = radius * 0.5f;
                float r = a * fastSin(3 * theta + t);
                x = r * fastCos(theta);
                y = r * fastSin(theta);
            }
            break;



          case 61:  // Spiral with time-based expansion
            r = radius * (0.7f + 0.3f * sin(t * 0.5f));
            x = r * cos(theta) * (1.0f + 0.3f * sin(t + theta * 2.0f));
            y = r * sin(theta) * (1.0f + 0.3f * cos(t + theta * 1.5f));
            break;
            
        case 62:  // Pulsating flower
            r = radius * (0.6f + 0.4f * sin(t * 0.7f + theta));
            x = r * cos(theta) * (1.0f + 0.5f * sin(theta * 5.0f + t * 2.0f));
            y = r * sin(theta) * (1.0f + 0.5f * cos(theta * 5.0f + t * 2.0f));
            break;
            
        case 63:  // Rotating star with breathing effect
            r = radius * (0.5f + 0.5f * sin(t * 0.3f));
            x = r * cos(theta * 3.0f) * cos(theta + t);
            y = r * sin(theta * 3.0f) * sin(theta + t);
            break;
            
        case 64:  // Lissajous with time variation
            x = radius * sin(theta * 2.0f + t * 1.3f) * (0.8f + 0.2f * sin(t));
            y = radius * sin(theta * 3.0f + t * 1.7f) * (0.8f + 0.2f * cos(t));
            break;
            
        case 65:  // Worm-like undulation
            x = radius * cos(theta) * (1.0f + 0.4f * sin(theta * 8.0f + t * 3.0f));
            y = radius * sin(theta) * (1.0f + 0.4f * cos(theta * 8.0f + t * 3.0f));
            break;
            
        case 66:  // Pulsating circles with offset
            r = radius * (0.4f + 0.6f * sin(t * 0.8f + theta));
            x = r * cos(theta + sin(t * 0.5f));
            y = r * sin(theta + cos(t * 0.5f));
            break;
            
        case 67:  // Hyperbolic spiral
            r = radius / (theta + 1.0f) * (1.5f + sin(t));
            x = r * cos(theta + t);
            y = r * sin(theta + t);
            break;
            
        case 68:  // Epicycloid
            x = radius * (cos(theta) + 0.3f * cos(7.0f * theta + t * 2.0f));
            y = radius * (sin(theta) + 0.3f * sin(7.0f * theta + t * 2.0f));
            break;
            
        case 69:  // Heart curve with pulse
            r = radius * (0.7f + 0.3f * sin(t));
            x = r * 16.0f * pow(sin(theta), 3.0f);
            y = r * -(13.0f * cos(theta) - 5.0f * cos(2.0f * theta) - 2.0f * cos(3.0f * theta) - cos(4.0f * theta));
            x *= 0.03f; y *= 0.03f;  // Scale down
            break;
            
        case 70:  // Butterfly curve
            r = radius * exp(cos(theta)) - 2.0f * cos(4.0f * theta) + pow(sin(theta/12.0f), 5.0f);
            x = r * cos(theta + t) * 0.2f;
            y = r * sin(theta + t) * 0.2f;
            break;
            
        case 71: // Rotating square morph
            r = radius * (0.8f + 0.2f * sin(t * 2.0f));
            x = r * (cos(theta) + 0.3f * cos(3.0f * theta + t));
            y = r * (sin(theta) + 0.3f * sin(3.0f * theta + t));
            break;
            
        case 72: // Breathing grid
            x = radius * cos(theta) * (1.0f + 0.5f * sin(t * 2.0f + theta * 4.0f));
            y = radius * sin(theta) * (1.0f + 0.5f * cos(t * 2.0f + theta * 4.0f));
            break;
            
        case 73: // Twisting ribbon
            x = radius * (0.7f + 0.3f * cos(t)) * cos(theta + sin(t * 1.5f));
            y = radius * (0.7f + 0.3f * sin(t)) * sin(theta + cos(t * 1.5f));
            break;
            
        case 74: // Pulsating waves
            x = radius * cos(theta) * (0.8f + 0.4f * sin(theta * 6.0f + t * 3.0f));
            y = radius * sin(theta) * (0.8f + 0.4f * cos(theta * 6.0f + t * 3.0f));
            break;
            
        case 75: // Morphing blob
            r = radius * (0.6f + 0.4f * sin(theta * 5.0f + t * 2.0f));
            x = r * cos(theta + t * 0.7f);
            y = r * sin(theta + t * 0.7f);
            break;
            
        case 76: // Double helix
            x = radius * cos(theta) + radius * 0.3f * cos(3.0f * theta + t);
            y = radius * sin(theta) + radius * 0.3f * sin(3.0f * theta + t);
            break;
            
        case 77: // Chaotic attractor-like
            x = radius * sin(theta * 0.7f + t) * cos(theta * 1.3f);
            y = radius * cos(theta * 0.7f + t) * sin(theta * 1.3f);
            break;
            
        case 78: // Pulsating rings
            r = radius * (0.5f + 0.5f * sin(t + theta * 2.0f));
            x = r * cos(theta * 2.0f);
            y = r * sin(theta * 2.0f);
            break;
            
        case 79: // Swirling vortex
            r = radius * (0.3f + 0.7f * (theta / TWO_PI));
            x = r * cos(theta * 2.0f + t * 2.0f);
            y = r * sin(theta * 2.0f + t * 2.0f);
            break;
            
        default:
            r = radius * (0.6f + 0.4f * sin(t * 0.9f + theta * 1.7f));
            x = r * cos(theta) * (1.0f + 0.3f * sin(t * 1.2f + theta * 3.0f));
            y = r * sin(theta) * (1.0f + 0.3f * cos(t * 1.2f + theta * 3.0f));
            break;
    }
    
    *outX = x;
    *outY = y;
}

 

// Draw a curve with enhanced visibility
void drawCurve(int curveType, float alpha) {
    const int resolution = 600; // High resolution for smooth curves
    const float radius = maxRadius * 1.0f;
    
    for (int i = 0; i <= resolution; i++) {
        float theta = maper(i, 0, resolution, 0, TWO_PI * 3);
        float x, y;
        calculateCurvePoint(theta, radius, curveType, &x, &y);
        
        int screenX = (int)(CENTER_X + x);
        int screenY = (int)(CENTER_Y + y);
        
        if (screenX >= 0 && screenX < WIDTH && screenY >= 0 && screenY < HEIGHT) {
            // Bright color cycling through spectrum
            float hue = fmod((i * 1.2f + t * 50.0f), 360.0f);
            uint16_t color = hsvToRgb565(hue, 1.0f, 1.0f);
            
            // Draw single pixel
            setBufferPixel(screenX, screenY, color);
        }
    }
}

 

void initParticles() {
    for (int i = 0; i < numParticles; i++) {
        par[i].x = 0;
        par[i].y = 0;
        par[i].speed = random(5, 100) / 1000.0f;
        par[i].offset = random(0, 628) / 100.0f; // 0 to 2π
        par[i].color = colorPalettes[currentCurve % numPalettes].colors[random(0, 3)];
        par[i].size =random(1, 2); // Fixed size of 1 pixel
        par[i].life = random(200, 2500) / 1000.0f;
        par[i].birthTime = t;
        par[i].shapeType = random(0, 5);
    }
}

// Update and draw particles
void updateParticles() {
    for (int i = 0; i < numParticles; i++) {
        Parti  *p = &par[i];
        
        // Check if particle should respawn
        if (t - p->birthTime > p->life * 10) {
            p->speed = random(5, 100) / 1000.0f;
            p->offset = random(0, 628) / 100.0f;
            p->color = colorPalettes[currentCurve % numPalettes].colors[random(0, 3)];
            // p->color = palette256[ currentCurve % 256];
            p->size = random(1, 3); // Fixed size of 1 pixel
            p->life = random(200, 2500) / 1000.0f;
            p->birthTime = t;
            p->shapeType = random(0, 5);
        }
        
        // Calculate particle position
        float theta = (t * p->speed + p->offset) * TWO_PI;
        float radius = maxRadius * 1.0f;
        calculateCurvePoint(theta, radius, currentCurve, &p->x, &p->y);
        
        int screenX = (int)(CENTER_X + p->x);
        int screenY = (int)(CENTER_Y + p->y);
        
        // Draw particle
        if (screenX >= 0 && screenX < WIDTH && screenY >= 0 && screenY < HEIGHT) {
            // Fade based on life
            float alpha = map(t - p->birthTime, 0, p->life * 10, 1, 0.2);
            alpha = constrain(33+alpha, 0, 1);
            
            uint16_t color = getPlasmaColor(p->x, p->y, t);
            
            // Draw particle shape
            for (int dx = -p->size/2; dx <= p->size/2; dx++) {
                for (int dy = -p->size/2; dy <= p->size/2; dy++) {
                    int px = screenX + dx;
                    int py = screenY + dy;
                    if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                        setBufferPixel(px, py, color);
                    }
                }
            }
        }
    }
}
 

// Draw background effects
void drawBackground() {
    // Reduce cosmic noise significantly - only 5 pixels instead of 30
    for (int i = 0; i < 5; i++) {
        int x = random(0, WIDTH);
        int y = random(0, HEIGHT);
        float noise = sin(x * 0.1f + t) * cos(y * 0.1f + t);
        if (noise > 0.9f) { // Higher threshold - was 0.7f
            uint16_t color = hsvToRgb565((t * 50 + x + y), 0.2f, 0.3f); // Dimmer
            setBufferPixel(x, y, color);
        }
    }
}

// Main animation initialization
void initSupercharged() {
    initTextBuffer();
    initParticles();
    randomSeed(analogRead(0));
    clearTextBuffer();
    lastChangeTime = millis();
}


 
// Main animation update
void updateSupercharged() {
    unsigned long currentTime = millis();
    
    // Clear buffer
     clearTextBuffer();
    
    // Draw background effects
    drawBackground();
    
    // Handle curve transitions
    if (currentTime - lastChangeTime > 7000) {
        transitionProgress += 0.01f;
        if (transitionProgress >= 2.0f) {
            transitionProgress = 0.1f;
            currentCurve = nextCurve;
            nextCurve = (nextCurve + random(1, 6)) % 60;
            lastChangeTime = currentTime;
        }
    }
    
    // Draw curves with transition
    if (transitionProgress > 0) {
        drawCurve(currentCurve, 1.0f - transitionProgress);
        drawCurve(nextCurve, transitionProgress);
    } else {
        drawCurve(currentCurve, 1.0f);
    }
    
    // Draw additional curve layers for depth
    for (int layer = 0; layer < 3; layer++) {
        int layerCurve = (currentCurve + layer + 1) % 60;
        drawCurve(layerCurve, 0.3f / (layer + 1));
    }
    
    // Update and draw particles
    updateParticles();
    
    // Update time
    t += animationSpeed;
    if (t > 1000.0f) t = 0.0f; // Prevent overflow
    
    // Flip buffers to display
    pageFlip();
}


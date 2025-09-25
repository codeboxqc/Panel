// ESP32 Fast 3D Line Animations - 10 optimized cases
// High-performance 3D visualization for HUB75 displays

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <math.h>

#include "gfx.h"

// Fast 3D point transformation
void transform3D(float x, float y, float z, float rotX, float rotY, float rotZ, float* screenX, float* screenY) {
    // Optimized 3D rotation matrix
    float cosX = cos(rotX), sinX = sin(rotX);
    float cosY = cos(rotY), sinY = sin(rotY);
    float cosZ = cos(rotZ), sinZ = sin(rotZ);
    
    // Apply rotations
    float x1 = x;
    float y1 = y * cosX - z * sinX;
    float z1 = y * sinX + z * cosX;
    
    float x2 = x1 * cosY + z1 * sinY;
    float y2 = y1;
    float z2 = -x1 * sinY + z1 * cosY;
    
    float x3 = x2 * cosZ - y2 * sinZ;
    float y3 = x2 * sinZ + y2 * cosZ;
    float z3 = z2;
    
    // Perspective projection
    float perspective = 200.0f / (200.0f + z3);
    *screenX = x3 * perspective;
    *screenY = y3 * perspective;
}

 

// Case 0: 3D Rotating Cube with Trails
void draw3DCube(float centerX, float centerY, float scale, float rotX, float rotY, float rotZ, float t) {
    float vertices[8][3] = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
    };
    
    int edges[12][2] = {
        {0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6},
        {6,7}, {7,4}, {0,4}, {1,5}, {2,6}, {3,7}
    };
    
    for (int i = 0; i < 12; i++) {
        float x1, y1, x2, y2;
        transform3D(vertices[edges[i][0]][0] * scale, vertices[edges[i][0]][1] * scale, vertices[edges[i][0]][2] * scale, rotX, rotY, rotZ, &x1, &y1);
        transform3D(vertices[edges[i][1]][0] * scale, vertices[edges[i][1]][1] * scale, vertices[edges[i][1]][2] * scale, rotX, rotY, rotZ, &x2, &y2);
        
        uint16_t color = 0xFFE0;
        dma_display->drawLine(centerX + x1, centerY + y1, centerX + x2, centerY + y2, color);
    }
}

 

// Case 2: 3D Sphere Wireframe
void draw3DSphere(float centerX, float centerY, float scale, float rotX, float rotY, float t) {
    int meridians = 12, parallels = 8;
    
    // Draw meridians
    for (int m = 0; m < meridians; m++) {
        float longitude = m * TWO_PI / meridians;
        for (int i = 0; i < parallels; i++) {
            float lat1 = (i - parallels/2) * PI / parallels;
            float lat2 = (i + 1 - parallels/2) * PI / parallels;
            
            float x1 = cos(lat1) * cos(longitude) * scale;
            float y1 = cos(lat1) * sin(longitude) * scale;
            float z1 = sin(lat1) * scale;
            
            float x2 = cos(lat2) * cos(longitude) * scale;
            float y2 = cos(lat2) * sin(longitude) * scale;
            float z2 = sin(lat2) * scale;
            
            float sx1, sy1, sx2, sy2;
            transform3D(x1, y1, z1, rotX, rotY, 0, &sx1, &sy1);
            transform3D(x2, y2, z2, rotX, rotY, 0, &sx2, &sy2);
            
            uint16_t color = 0xF800;
            dma_display->drawLine(centerX + sx1, centerY + sy1, centerX + sx2, centerY + sy2, color);
        }
    }
}

 
 
 

// Continue with remaining cases (6-9)...
void draw3DOctahedron(float centerX, float centerY, float scale, float rotX, float rotY, float rotZ, float t) {
    float vertices[6][3] = {
        {0, 0, 1}, {0, 0, -1}, {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}
    };
    
    int edges[12][2] = {
        {0,2}, {0,3}, {0,4}, {0,5}, {1,2}, {1,3}, {1,4}, {1,5}, {2,4}, {3,5}, {2,5}, {3,4}
    };
    
    for (int i = 0; i < 12; i++) {
        float x1, y1, x2, y2;
        transform3D(vertices[edges[i][0]][0] * scale, vertices[edges[i][0]][1] * scale, vertices[edges[i][0]][2] * scale, rotX, rotY, rotZ, &x1, &y1);
        transform3D(vertices[edges[i][1]][0] * scale, vertices[edges[i][1]][1] * scale, vertices[edges[i][1]][2] * scale, rotX, rotY, rotZ, &x2, &y2);
        
        uint16_t color = 0x07E0;
        dma_display->drawLine(centerX + x1, centerY + y1, centerX + x2, centerY + y2, color);
    }
}

 
  


// Fast 3D line animation system
void draw3DLineAnimations(unsigned long time) {
    static unsigned long lastUpdate = 0;
    static float t = 0.0f;
    static int currentEffect = 0;
    static unsigned long lastEffectChange = 0;
    static float rotX = 0, rotY = 0, rotZ = 0;
    
    // Update every 40ms for smooth 25fps animation
    if (time - lastUpdate > 20) {
        lastUpdate = time;
        t += 0.05f;
        
        // Rotate 3D space
        rotX += 0.01f;
        rotY += 0.02f;
        rotZ += 0.03f;
        
        // Change effect every 5 seconds
        if (time - lastEffectChange > 5000*10) {
            currentEffect = random(0,3);
            lastEffectChange = time;
            rotX = rotY = rotZ = 0; // Reset rotation for new effect
        }
    }
    
    // Clear display
    dma_display->fillScreen(0x0000);
    
    // Center coordinates
    float centerX = WIDTH  / 2.0f;
    float centerY = HEIGHT / 2.0f;
    float scale = min(WIDTH , HEIGHT) * 0.25f;
    

   
    // Draw the current 3D effect
    switch (currentEffect) {

        case 0: // 3D Rotating Cube with Trails
            draw3DCube(centerX, centerY, scale, rotX, rotY, rotZ, t);
            break;
    
        case 1: // 3D Sphere Wireframe
            draw3DSphere(centerX, centerY, scale+2, rotX, rotY, t);
            break;
    
        case 2: // 3D Octahedron
            draw3DOctahedron(centerX, centerY, scale+2, rotX, rotY, rotZ, t);
            break;
            
     
    }
}

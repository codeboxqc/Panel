#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "gfx.h"

// Configuration de l'écran HUB75
 
 

 

// Structure pour les astéroïdes
struct Asteroid {
  float x, y;           // Position
  float vx, vy;         // Vélocité
  float size;           // Taille du triangle
  int collisions;       // Nombre de collisions
  bool active;          // Actif ou non
  uint16_t color;       // Couleur
  float rotation;       // Rotation du triangle
  float rotSpeed;       // Vitesse de rotation
};

// Structure pour les débris d'explosion
struct Debris {
  float x, y;           // Position
  float vx, vy;         // Vélocité
  float life;           // Durée de vie (0.0 à 1.0)
  uint16_t color;       // Couleur
  bool active;          // Actif ou non
};

const int MAX_ASTEROIDS = 5;
const int MAX_DEBRIS = 15;  // 5 astéroïdes * 3 débris chacun

Asteroid asteroids[MAX_ASTEROIDS];
Debris debris[MAX_DEBRIS];

// Constantes physiques
const float GRAVITY = 0.07f;
const float BOUNCE_DAMPING = 0.8f;
const float MIN_VELOCITY = 0.2f;
const float MAX_VELOCITY = 2.0f;
const float COLLISION_DISTANCE = 7.0f;
 
 
void updateAsteroids();
void respawnAsteroid(int index);
void explodeAsteroid(int index);
 int findFreeDebris();




void initAsteroids() {

 
  
  // Initialiser les débris
  for (int i = 0; i < MAX_DEBRIS; i++) {
    debris[i].active = false;
  }
  
  

  uint16_t colors[] = {
    dma_display->color565(255, 100, 100),  // Rouge
    dma_display->color565(100, 255, 100),  // Vert
    dma_display->color565(100, 100, 255),  // Bleu
    dma_display->color565(255, 255, 100),  // Jaune
    dma_display->color565(255, 100, 255)   // Magenta
  };


  
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    asteroids[i].x = random(10,WIDTH - 10);
    asteroids[i].y = random(10, HEIGHT - 10);
    asteroids[i].vx = random(-200, 200) / 100.0f;
    asteroids[i].vy = random(-200, 200) / 100.0f;
    asteroids[i].size = random(2, 3);
    asteroids[i].collisions = 0;
    asteroids[i].active = true;
    asteroids[i].color = colors[i];
    asteroids[i].rotation = 0;
    asteroids[i].rotSpeed = random(-10, 10) / 100.0f;
  }
}

void drawTriangle(float x, float y, float size, float rotation, uint16_t color) {
  // Calcul des points du triangle avec rotation
  float cos_r = cos(rotation);
  float sin_r = sin(rotation);
  
  // Points du triangle (équilatéral)
  float x1 = 0, y1 = -size;
  float x2 = -size * 0.866f, y2 = size * 0.5f;
  float x3 = size * 0.866f, y3 = size * 0.5f;
  
  // Rotation des points
  float rx1 = x1 * cos_r - y1 * sin_r;
  float ry1 = x1 * sin_r + y1 * cos_r;
  float rx2 = x2 * cos_r - y2 * sin_r;
  float ry2 = x2 * sin_r + y2 * cos_r;
  float rx3 = x3 * cos_r - y3 * sin_r;
  float ry3 = x3 * sin_r + y3 * cos_r;
  
  // Translation vers la position finale
  int px1 = (int)(x + rx1);
  int py1 = (int)(y + ry1);
  int px2 = (int)(x + rx2);
  int py2 = (int)(y + ry2);
  int px3 = (int)(x + rx3);
  int py3 = (int)(y + ry3);
  
  // Dessiner les lignes du triangle
  dma_display->drawLine(px1, py1, px2, py2, color);
  dma_display->drawLine(px2, py2, px3, py3, color);
  dma_display->drawLine(px3, py3, px1, py1, color);
}

float distance(float x1, float y1, float x2, float y2) {
  float dx = x2 - x1;
  float dy = y2 - y1;
  return sqrt(dx * dx + dy * dy);
}

void checkCollisions() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) continue;
    
    for (int j = i + 1; j < MAX_ASTEROIDS; j++) {
      if (!asteroids[j].active) continue;
      
      float dist = distance(asteroids[i].x, asteroids[i].y, 
                           asteroids[j].x, asteroids[j].y);
      
      if (dist < COLLISION_DISTANCE) {
        // Collision détectée
        asteroids[i].collisions++;
        asteroids[j].collisions++;
        
        // Effet de rebond avec échange de vélocité
        float tempVx = asteroids[i].vx;
        float tempVy = asteroids[i].vy;
        asteroids[i].vx = asteroids[j].vx * BOUNCE_DAMPING;
        asteroids[i].vy = asteroids[j].vy * BOUNCE_DAMPING;
        asteroids[j].vx = tempVx * BOUNCE_DAMPING;
        asteroids[j].vy = tempVy * BOUNCE_DAMPING;
        
        // Séparer les astéroïdes pour éviter les collisions multiples
        float dx = asteroids[j].x - asteroids[i].x;
        float dy = asteroids[j].y - asteroids[i].y;
        float len = sqrt(dx * dx + dy * dy);
        if (len > 0) {
          dx /= len;
          dy /= len;
          asteroids[i].x -= dx * 2;
          asteroids[i].y -= dy * 2;
          asteroids[j].x += dx * 2;
          asteroids[j].y += dy * 2;
        }
        
        // Explosion après 3 collisions
        if (asteroids[i].collisions >= 5) {
          explodeAsteroid(i);
        }
        if (asteroids[j].collisions >= 5) {
          explodeAsteroid(j);
        }
      }
    }
  }
}
 
void explodeAsteroid(int index) {
  if (!asteroids[index].active) return;
  
  // Créer 3 débris
  for (int i = 0; i < 3; i++) {
    int debrisIndex = findFreeDebris();
    if (debrisIndex != -1) {
      debris[debrisIndex].x = asteroids[index].x;
      debris[debrisIndex].y = asteroids[index].y;
      // Directions aléatoires pour les débris
      float angle = random(0, 628) / 100.0f; // 0 à 2π
      debris[debrisIndex].vx = cos(angle) * random(100, 300) / 100.0f;
      debris[debrisIndex].vy = sin(angle) * random(100, 300) / 100.0f;
      debris[debrisIndex].life = 1.0f;
      debris[debrisIndex].color = asteroids[index].color;
      debris[debrisIndex].active = true;
    }
  }
  
  // Réapparaître l'astéroïde à une position aléatoire
  respawnAsteroid(index);
}

int findFreeDebris() {
  for (int i = 0; i < MAX_DEBRIS; i++) {
    if (!debris[i].active) return i;
  }
  return -1;
}




void respawnAsteroid(int index) {
  asteroids[index].x = random(10, WIDTH - 10);
  asteroids[index].y = random(10, HEIGHT - 10);
  asteroids[index].vx = random(-200, 200) / 100.0f;
  asteroids[index].vy = random(-200, 200) / 100.0f;
  asteroids[index].collisions = 0;
  asteroids[index].rotation = 0;
  asteroids[index].size = random(2, 5);
}

void updateAsteroids() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (!asteroids[i].active) continue;

    // Attraction/gravity code stays the same
    for (int j = 0; j < MAX_ASTEROIDS; j++) {
      if (i == j || !asteroids[j].active) continue;

      float dx = asteroids[j].x - asteroids[i].x;
      float dy = asteroids[j].y - asteroids[i].y;
      float dist = sqrt(dx * dx + dy * dy);

      if (dist > 0 && dist < 30) {
        float force = GRAVITY / (dist * dist);
        asteroids[i].vx += (dx / dist) * force;
        asteroids[i].vy += (dy / dist) * force;
      }
    }

    // Clamp speed
    float speed = sqrt(asteroids[i].vx * asteroids[i].vx + asteroids[i].vy * asteroids[i].vy);
    if (speed > MAX_VELOCITY) {
      asteroids[i].vx = (asteroids[i].vx / speed) * MAX_VELOCITY;
      asteroids[i].vy = (asteroids[i].vy / speed) * MAX_VELOCITY;
    } else if (speed < MIN_VELOCITY && speed > 0) {
      asteroids[i].vx = (asteroids[i].vx / speed) * MIN_VELOCITY;
      asteroids[i].vy = (asteroids[i].vy / speed) * MIN_VELOCITY;
    }

    // Update position
    asteroids[i].x += asteroids[i].vx;
    asteroids[i].y += asteroids[i].vy;
    asteroids[i].rotation += asteroids[i].rotSpeed;

    // Wrap-around instead of bounce
    if (asteroids[i].x < 0) asteroids[i].x = WIDTH - 1;
    if (asteroids[i].x >= WIDTH) asteroids[i].x = 0;
    if (asteroids[i].y < 0) asteroids[i].y = HEIGHT - 1;
    if (asteroids[i].y >= HEIGHT) asteroids[i].y = 0;
  }
}


void updateDebris() {
  for (int i = 0; i < MAX_DEBRIS; i++) {
    if (!debris[i].active) continue;
    
    // Mettre à jour la position avec gravité
    debris[i].vy += GRAVITY * 0.5f; // Gravité vers le bas
    debris[i].x += debris[i].vx;
    debris[i].y += debris[i].vy;
    
    // Diminuer la durée de vie
    debris[i].life -= 0.02f;
    
    if (debris[i].life <= 0 || 
        debris[i].x < 0 || debris[i].x > WIDTH ||
        debris[i].y < 0 || debris[i].y > HEIGHT) {
      debris[i].active = false;
    }
  }
}

void drawDebris() {
  for (int i = 0; i < MAX_DEBRIS; i++) {
    if (!debris[i].active) continue;
    
    // Couleur qui fade avec la durée de vie
    uint8_t alpha = (uint8_t)(debris[i].life * 255);
    uint16_t fadedColor = dma_display->color565(
      (debris[i].color >> 11) * alpha / 255,
      ((debris[i].color >> 5) & 0x3F) * alpha / 63,
      (debris[i].color & 0x1F) * alpha / 31
    );
    
    // Dessiner une ligne courte comme débris
    int x = (int)debris[i].x;
    int y = (int)debris[i].y;
    dma_display->drawLine(x, y, x + 3, y + 3, fadedColor);
    delay(5);
  }
}

void asteroid() {
  dma_display->clearScreen();
  
  // Mettre à jour la physique
  updateAsteroids();
  updateDebris();
  checkCollisions();
  
  // Dessiner les astéroïdes
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (asteroids[i].active) {
      drawTriangle(asteroids[i].x, asteroids[i].y, asteroids[i].size, 
                   asteroids[i].rotation, asteroids[i].color);
    }
  }
  
  // Dessiner les débris
  drawDebris();
  
  delay(40); // ~20 FPS
}
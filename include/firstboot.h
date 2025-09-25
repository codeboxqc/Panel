#ifndef FIRSTBOOT_H
#define FIRSTBOOT_H

#include <Arduino.h>
//#include <WiFi.h>
// #include <WiFiClientSecure.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Panel configuration
 
 

// Configuration des pins par défaut pour la matrice LED HUB75
struct MatrixPins {
    int8_t R1_PIN = 17;
    int8_t G1_PIN = 18;
    int8_t B1_PIN = 8;
    int8_t R2_PIN = 3;
    int8_t G2_PIN = 2;
    int8_t B2_PIN = 10;
    int8_t A_PIN = 15;
    int8_t B_PIN = 11;
    int8_t C_PIN = 7;
    int8_t D_PIN = 4;
    int8_t E_PIN = 13;
    int8_t LAT_PIN = 6;
    int8_t OE_PIN = 12;
    int8_t CLK_PIN = 5;
    
    // Function to create HUB75 configuration
    HUB75_I2S_CFG::i2s_pins getPinsConfig() {
        return {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN,
               A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
    }
};



// Global variable declaration
extern MatrixPins pins;

// Function declarations
 
bool isConfigComplete();
MatrixPins getMatrixPins();
void handleFirstBoot();

// Other function declarations needed
bool checkFirstBoot();
void saveWiFiConfig(String ssid, String password);
void savePinConfig(MatrixPins& pins);
MatrixPins loadPinConfig();
void loadWiFiConfig(String& ssid, String& password);
void resetConfig();
void handleRoot();
void handleSave();
int extractPinValue(String body, String pinName);
void createAccessPoint();
 int  initFirstBoot();
void checkResetButton();
// Web page declaration
 

extern String ssidGlob;
extern String passwordGlob;



#endif
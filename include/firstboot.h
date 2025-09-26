#ifndef FIRSTBOOT_H
#define FIRSTBOOT_H

#include <Arduino.h>
//#include <WiFi.h>
// #include <WiFiClientSecure.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "gfx.h"
// Panel configuration
 
 
 
 
// Configuration des pins par défaut pour la matrice LED HUB75
struct MatrixPins {
    int8_t mR1_PIN = R1_PIN;
    int8_t mG1_PIN = G1_PIN;
    int8_t mB1_PIN = B1_PIN;
    int8_t mR2_PIN = R2_PIN;
    int8_t mG2_PIN = G2_PIN;
    int8_t mB2_PIN = B2_PIN;
    int8_t mA_PIN =  A_PIN;
    int8_t mB_PIN = B_PIN;
    int8_t mC_PIN = C_PIN;
    int8_t mD_PIN = D_PIN;
    int8_t mE_PIN = E_PIN;
    int8_t mLAT_PIN = LAT_PIN;
    int8_t mOE_PIN = OE_PIN;
    int8_t mCLK_PIN = CLK_PIN;
    
    // Function to create HUB75 configuration
    HUB75_I2S_CFG::i2s_pins getPinsConfig() {
        return {mR1_PIN, mG1_PIN, mB1_PIN, mR2_PIN, mG2_PIN, mB2_PIN,
               mA_PIN, mB_PIN, mC_PIN, mD_PIN, mE_PIN, mLAT_PIN, mOE_PIN, mCLK_PIN};
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
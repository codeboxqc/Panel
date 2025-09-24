#include "firstboot.h"


//http://192.168.4.1


// Configuration web page
const char  CONFIG_PAGE[] PROGMEM= R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 First Boot Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .form-group { margin: 15px 0; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type="text"], input[type="password"], input[type="number"] {
            width: 100%; padding: 8px; border: 1px solid #ccc; border-radius: 4px;
        }
        .pin-group { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
        .btn { background: #4CAF50; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; width: 100%; }
        .btn:hover { background: #45a049; }
        .status { margin: 10px 0; padding: 10px; border-radius: 4px; }
        .success { background: #d4edda; color: #155724; }
        .error { background: #f8d7da; color #721c24; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Configuration</h1>
        <p>Configure your WiFi and LED matrix pins</p>

        <form id="configForm">
            <h2>WiFi Configuration</h2>
            <div class="form-group">
                <label>WiFi Network Name (SSID):</label>
                <input type="text" id="ssid" name="ssid" required>
            </div>
            <div class="form-group">
                <label>WiFi Password:</label>
                <input type="password" id="password" name="password">
            </div>

            <h2>LED Matrix Pins Configuration (HUB75)</h2>
            <div class="pin-group">
                <div class="form-group">
                    <label>R1_PIN:</label>
                    <input type="number" id="r1_pin" name="r1_pin" value="17" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>G1_PIN:</label>
                    <input type="number" id="g1_pin" name="g1_pin" value="18" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>B1_PIN:</label>
                    <input type="number" id="b1_pin" name="b1_pin" value="8" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>R2_PIN:</label>
                    <input type="number" id="r2_pin" name="r2_pin" value="3" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>G2_PIN:</label>
                    <input type="number" id="g2_pin" name="g2_pin" value="2" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>B2_PIN:</label>
                    <input type="number" id="b2_pin" name="b2_pin" value="10" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>A_PIN:</label>
                    <input type="number" id="a_pin" name="a_pin" value="15" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>B_PIN:</label>
                    <input type="number" id="b_pin" name="b_pin" value="11" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>C_PIN:</label>
                    <input type="number" id="c_pin" name="c_pin" value="7" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>D_PIN:</label>
                    <input type="number" id="d_pin" name="d_pin" value="4" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>E_PIN:</label>
                    <input type="number" id="e_pin" name="e_pin" value="13" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>LAT_PIN:</label>
                    <input type="number" id="lat_pin" name="lat_pin" value="6" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>OE_PIN:</label>
                    <input type="number" id="oe_pin" name="oe_pin" value="12" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>CLK_PIN:</label>
                    <input type="number" id="clk_pin" name="clk_pin" value="5" min="0" max="39">
                </div>
            </div>

            <div class="form-group">
                <button type="button" class="btn" onclick="saveConfig() ">Save Configuration</button>
            </div>
        </form>

        <div id="status"></div>
    </div>

    <script>
        function saveConfig() {
            console.log('Save button clicked');
            const form = document.getElementById('configForm');
            const formData = new FormData(form);
            const data = {};

            for (let [key, value] of formData.entries()) {
                data[key] = value;
            }

            console.log('Sending data:', data);

            // Show immediate feedback
            const status = document.getElementById('status');
            status.innerHTML = '<div class="status">Saving configuration...</div>';

            fetch('/save', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data)
            })
            .then(response => {
                console.log('Response status:', response.status);
                return response.json();
            })
            .then(result => {
                console.log('Result:', result);
                if (result.success) {
                    status.innerHTML = '<div class="status success">Configuration saved! ESP32 will restart...</div>';
                    setTimeout(() => {
                        status.innerHTML = '<div class="status success">Restarting... Please wait...</div>';
                    }, 3000);
                } else {
                    status.innerHTML = '<div class="status error">Error: ' + result.message + '</div>';
                }
            })
            .catch(error => {
                console.log('Error:', error);
                document.getElementById('status').innerHTML = '<div class="status error">Connection error: ' + error.message + '</div>';
            });
        }
    </script>
</body>
</html>
)";





// Variables globales
WebServer *server = nullptr;
Preferences prefs;
MatrixPins pins; // Global instance
bool isFirstBoot = false;
bool configComplete = false;

// Bouton de reset (GPIO 0 - bouton BOOT sur ESP32)
#define RESET_BUTTON_PIN 0
 //#define RESET_BUTTON_PIN 5


// Vérifier si c'est le premier démarrage
bool checkFirstBoot() {
    prefs.begin("esp32_config", false);
    bool configured = prefs.getBool("configured", false);
    prefs.end();
    return !configured;
}

// Sauvegarder la configuration WiFi
void saveWiFiConfig(String ssid, String password) {
    prefs.begin("esp32_config", false);
    prefs.putString("wifi_ssid", ssid);
    prefs.putString("wifi_password", password);
    prefs.end();
    Serial.println("WiFi configuration saved");
}

// Sauvegarder la configuration des pins
void savePinConfig(MatrixPins& pins) {
    prefs.begin("esp32_config", false);


    prefs.putInt("r1_pin", pins.R1_PIN);
    prefs.putInt("b1_pin", pins.B1_PIN);
    prefs.putInt("r2_pin", pins.R2_PIN);
    prefs.putInt("b2_pin", pins.B2_PIN);
    prefs.putInt("a_pin", pins.A_PIN);
    prefs.putInt("c_pin", pins.C_PIN);
    prefs.putInt("clk_pin", pins.CLK_PIN);
    prefs.putInt("oe_pin", pins.OE_PIN);
    prefs.putInt("g1_pin", pins.G1_PIN);
    prefs.putInt("g2_pin", pins.G2_PIN);
    prefs.putInt("e_pin", pins.E_PIN);
    prefs.putInt("b_pin", pins.B_PIN);
    prefs.putInt("d_pin", pins.D_PIN);
    prefs.putInt("lat_pin", pins.LAT_PIN);


    prefs.putBool("configured", true);
    prefs.end();
    Serial.println("Pins configuration saved");
}

// Charger la configuration des pins
MatrixPins loadPinConfig() {
    MatrixPins loadedPins;
    prefs.begin("esp32_config", true);


    loadedPins.R1_PIN = prefs.getInt("r1_pin", 17);
    loadedPins.B1_PIN = prefs.getInt("b1_pin", 8);
    loadedPins.R2_PIN = prefs.getInt("r2_pin", 3);
    loadedPins.B2_PIN = prefs.getInt("b2_pin", 10);
    loadedPins.A_PIN = prefs.getInt("a_pin", 15);
    loadedPins.C_PIN = prefs.getInt("c_pin", 7);
    loadedPins.CLK_PIN = prefs.getInt("clk_pin", 5);
    loadedPins.OE_PIN = prefs.getInt("oe_pin", 12);
    loadedPins.G1_PIN = prefs.getInt("g1_pin", 18);
    loadedPins.G2_PIN = prefs.getInt("g2_pin", 2);
    loadedPins.E_PIN = prefs.getInt("e_pin", 13);
    loadedPins.B_PIN = prefs.getInt("b_pin", 11);
    loadedPins.D_PIN = prefs.getInt("d_pin", 4);
    loadedPins.LAT_PIN = prefs.getInt("lat_pin", 6);



    prefs.end();
    return loadedPins;
}

// Charger la configuration WiFi
void loadWiFiConfig(String& ssid, String& password) {
    prefs.begin("esp32_config", true);
    ssid = prefs.getString("wifi_ssid", "");
    password = prefs.getString("wifi_password", "");
    prefs.end();
}

// Reset de la configuration
void resetConfig() {
    prefs.begin("esp32_config", false);

    prefs.putInt("r1_pin", pins.R1_PIN);
    prefs.putInt("b1_pin", pins.B1_PIN);
    prefs.putInt("r2_pin", pins.R2_PIN);
    prefs.putInt("b2_pin", pins.B2_PIN);
    prefs.putInt("a_pin", pins.A_PIN);
    prefs.putInt("c_pin", pins.C_PIN);
    prefs.putInt("clk_pin", pins.CLK_PIN);
    prefs.putInt("oe_pin", pins.OE_PIN);
    prefs.putInt("g1_pin", pins.G1_PIN);
    prefs.putInt("g2_pin", pins.G2_PIN);
    prefs.putInt("e_pin", pins.E_PIN);
    prefs.putInt("b_pin", pins.B_PIN);
    prefs.putInt("d_pin", pins.D_PIN);
    prefs.putInt("lat_pin", pins.LAT_PIN);
    prefs.putBool("configured", false);

    prefs.end();
     Serial.println("Pins configuration saved");
    prefs.clear();
    prefs.end();
     Serial.println("Configuration erased - restarting...");
    delay(1000);
    ESP.restart();
}

// Fonction utilitaire pour extraire les valeurs des pins du JSON
int extractPinValue(String body, String pinName) {
    String searchStr = "\"" + pinName + "\":\"";
    int start = body.indexOf(searchStr) + searchStr.length();
    int end = body.indexOf("\"", start);
    if (start > searchStr.length() - 1 && end > start) {
        return body.substring(start, end).toInt();
    }
    return -1; // valeur par défaut en cas d'erreur
}

// Gestionnaires des requêtes web
void handleRoot() {
    server->send(200, "text/html", CONFIG_PAGE);
}

void handleSave() {
    Serial.println("Save request received");

    if (server->method() == HTTP_POST) {
        String body = server->arg("plain");
         Serial.println("POST Body: " + body);

        // Parse JSON simple
        String ssid = "";
        String password = "";

        int ssidStart = body.indexOf("\"ssid\":\"") + 8;
        int ssidEnd = body.indexOf("\"", ssidStart);
        if (ssidStart > 7 && ssidEnd > ssidStart) {
            ssid = body.substring(ssidStart, ssidEnd);
        }

        int passStart = body.indexOf("\"password\":\"") + 12;
        int passEnd = body.indexOf("\"", passStart);
        if (passStart > 11 && passEnd > passStart) {
            password = body.substring(passStart, passEnd);
        }

        //Serial.println("SSID: " + ssid);
        //Serial.println("Password: " + password);

        // Vérifier que les valeurs extraites sont valides
        int r1_val = extractPinValue(body, "r1_pin");
        int g1_val = extractPinValue(body, "g1_pin");

        if (r1_val != -1) pins.R1_PIN = r1_val;
        if (g1_val != -1) pins.G1_PIN = g1_val;

        // Sauvegarder tous les pins
        int b1_val = extractPinValue(body, "b1_pin");
        if (b1_val != -1) pins.B1_PIN = b1_val;

        int r2_val = extractPinValue(body, "r2_pin");
        if (r2_val != -1) pins.R2_PIN = r2_val;

        int g2_val = extractPinValue(body, "g2_pin");
        if (g2_val != -1) pins.G2_PIN = g2_val;

        int b2_val = extractPinValue(body, "b2_pin");
        if (b2_val != -1) pins.B2_PIN = b2_val;

        int a_val = extractPinValue(body, "a_pin");
        if (a_val != -1) pins.A_PIN = a_val;

        int b_val = extractPinValue(body, "b_pin");
        if (b_val != -1) pins.B_PIN = b_val;

        int c_val = extractPinValue(body, "c_pin");
        if (c_val != -1) pins.C_PIN = c_val;

        int d_val = extractPinValue(body, "d_pin");
        if (d_val != -1) pins.D_PIN = d_val;

        int e_val = extractPinValue(body, "e_pin");
        if (e_val != -1) pins.E_PIN = e_val;

        int lat_val = extractPinValue(body, "lat_pin");
        if (lat_val != -1) pins.LAT_PIN = lat_val;

        int oe_val = extractPinValue(body, "oe_pin");
        if (oe_val != -1) pins.OE_PIN = oe_val;

        int clk_val = extractPinValue(body, "clk_pin");
        if (clk_val != -1) pins.CLK_PIN = clk_val;

        if (ssid.length() > 0) {
            //Serial.println("Saving configuration...");
            saveWiFiConfig(ssid, password);
            savePinConfig(pins);

            server->send(200, "application/json", "{\"success\":true}");
             Serial.println("Configuration saved, restarting...");

            delete server;
            server = nullptr;

            delay(2000);
            ESP.restart();
        } else {
             Serial.println("SSID required");
            server->send(400, "application/json", "{\"success\":false,\"message\":\"SSID required\"}");
        }
    } else {
        Serial.println("Invalid method");
        server->send(405, "application/json", "{\"success\":false,\"message\":\"Method not allowed\"}");
    }
}


// Create WiFi access point
void createAccessPoint() {
    WiFi.softAP("ESP32Dev-Config", "");
     Serial.println("Access point created");
    Serial.print("IP Address: ");
     Serial.println(WiFi.softAPIP());

    server = new WebServer(80);
    server->on("/", handleRoot);
    server->on("/save", handleSave);
    server->begin();

     Serial.println("Web server started");
     Serial.println("Connect to 'ESP32-Config' network and go to http://192.168.4.1");
}


 static unsigned long lastPressTime = 0;
    static bool buttonPressed = false;
    static int pressCount = 0;
    const unsigned long debounceDelay = 50;
    const unsigned long doubleClickTimeout = 500; // Time window for double click
    const int requiredPresses = 2;

// Improved reset button check with proper debouncing
void checkResetButton2() {


    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        if (!buttonPressed && (millis() - lastPressTime > debounceDelay)) {
            buttonPressed = true;
            lastPressTime = millis();
            pressCount++;
            Serial.println("Button press detected. Count: " + String(pressCount));

            // Reset counter if too much time passed between presses
            if (pressCount > 1 && (millis() - lastPressTime) > doubleClickTimeout) {
                pressCount = 1;
            }

            // Check for double press
            if (pressCount >= requiredPresses) {
                //Serial.println("Double press detected - resetting configuration...");
                resetConfig();
                pressCount = 0;
            }
        }
    } else {
        buttonPressed = false;

        // Reset counter if timeout reached
        if (pressCount > 0 && (millis() - lastPressTime) > doubleClickTimeout) {
            pressCount = 0;
             Serial.println("Reset press counter (timeout)");
        }
    }
}

// Vérifier le bouton de reset
void checkResetButton() {
    static bool buttonPressed = false;

    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        if (!buttonPressed) {
            buttonPressed = true;
             Serial.println("Reset button pressed - erasing configuration...");
            resetConfig();
            delay(50); // Simple debounce delay
        }
    } else {
        buttonPressed = false;
    }
}

int initFirstBoot() {
    Serial.begin(115200);
     Serial.println("ESP32 Starting...");

    // Configurer le bouton de reset
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

    // Vérifier si c'est le premier démarrage
    isFirstBoot = checkFirstBoot();

    if (isFirstBoot) {
         Serial.println("First boot - configuration mode");
        createAccessPoint();
        return 0; // Configuration mode
    } else {
         Serial.println("Configuration found, loading...");
        pins = loadPinConfig();
        configComplete = true;
        return 1; // Skip configuration mode
    }
}

// Fonction principale à appeler dans la loop
void handleFirstBoot() {
    if (server != nullptr && !configComplete) {
        server->handleClient();
        checkResetButton();
        delay(10);
    }
}

// Fonction pour obtenir les pins configurés
MatrixPins getMatrixPins() {
    return pins;
}

// Fonction pour vérifier si la configuration est terminée
bool isConfigComplete() {
    return configComplete;
}
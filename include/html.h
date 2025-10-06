#include <pgmspace.h> 

// Configuration web page
const char CONFIG_PAGE[] PROGMEM = R"(
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
        .btn { background: #4CAF50; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; width: 100%; margin-bottom: 10px; }
        .btn:hover { background: #45a049; }
        .reset-btn { background: #ff9800; }
        .reset-btn:hover { background: #e68900; }
        .status { margin: 10px 0; padding: 10px; border-radius: 4px; }
        .success { background: #d4edda; color: #155724; }
        .error { background: #f8d7da; color: #721c24; }
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
            <div class="form-group">
                <button type="button" class="btn" onclick="saveWiFi() ">Save WiFi Configuration</button>
            </div>
            
            <h2>LED Matrix Pins Configuration (HUB75)</h2>
            <div class="pin-group">
                <div class="form-group">
                    <label>R1_PIN:</label>
                    <input type="number" id="r1_pin" name="r1_pin" value="%R1_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>G1_PIN:</label>
                    <input type="number" id="g1_pin" name="g1_pin" value="%G1_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>B1_PIN:</label>
                    <input type="number" id="b1_pin" name="b1_pin" value="%B1_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>R2_PIN:</label>
                    <input type="number" id="r2_pin" name="r2_pin" value="%R2_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>G2_PIN:</label>
                    <input type="number" id="g2_pin" name="g2_pin" value="%G2_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>B2_PIN:</label>
                    <input type="number" id="b2_pin" name="b2_pin" value="%B2_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>A_PIN:</label>
                    <input type="number" id="a_pin" name="a_pin" value="%A_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>B_PIN:</label>
                    <input type="number" id="b_pin" name="b_pin" value="%B_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>C_PIN:</label>
                    <input type="number" id="c_pin" name="c_pin" value="%C_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>D_PIN:</label>
                    <input type="number" id="d_pin" name="d_pin" value="%D_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>E_PIN:</label>
                    <input type="number" id="e_pin" name="e_pin" value="%E_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>LAT_PIN:</label>
                    <input type="number" id="lat_pin" name="lat_pin" value="%LAT_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>OE_PIN:</label>
                    <input type="number" id="oe_pin" name="oe_pin" value="%OE_PIN%" min="0" max="39">
                </div>
                <div class="form-group">
                    <label>CLK_PIN:</label>
                    <input type="number" id="clk_pin" name="clk_pin" value="%CLK_PIN%" min="0" max="39">
                </div>
            </div>
            
            <div class="form-group">
                <button type="button" class="btn" onclick="savePins() ">Save Pin Configuration</button>
                <button type="button" class="btn reset-btn" onclick="resetPins() ">Reset to Default Pins</button>
                <button type="button" class="btn" onclick="saveAll() ">Save All Configuration</button>
            </div>
        </form>
        
        <div id="status"></div>
    </div>

    <script>
        function saveWiFi() {
            console.log('Save WiFi button clicked');
            const form = document.getElementById('configForm');
            const formData = new FormData(form);
            const data = { ssid: formData.get('ssid'), password: formData.get('password') };
            
            console.log('Sending WiFi data:', data);
            
            const status = document.getElementById('status');
            status.innerHTML = '<div class="status">Saving WiFi configuration...</div>';
            
            fetch('/save_wifi', {
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
                    status.innerHTML = '<div class="status success">WiFi configuration saved! ESP32 will restart...</div>';
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

        function savePins() {
            console.log('Save Pins button clicked');
            const form = document.getElementById('configForm');
            const formData = new FormData(form);
            const data = {};
            
            for (let [key, value] of formData.entries()) {
                if (key !== 'ssid' && key !== 'password') {
                    data[key] = value;
                }
            }
            
            console.log('Sending Pins data:', data);
            
            const status = document.getElementById('status');
            status.innerHTML = '<div class="status">Saving pin configuration...</div>';
            
            fetch('/save_pins', {
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
                    status.innerHTML = '<div class="status success">Pin configuration saved! ESP32 will restart...</div>';
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

        function saveAll() {
            console.log('Save All button clicked');
            const form = document.getElementById('configForm');
            const formData = new FormData(form);
            const data = {};
            
            for (let [key, value] of formData.entries()) {
                data[key] = value;
            }
            
            console.log('Sending all data:', data);
            
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

        function resetPins() {
            console.log('Reset Pins button clicked');
            document.getElementById('r1_pin').value = '%R1_PIN%';
            document.getElementById('g1_pin').value = '%G1_PIN%';
            document.getElementById('b1_pin').value = '%B1_PIN%';
            document.getElementById('r2_pin').value = '%R2_PIN%';
            document.getElementById('g2_pin').value = '%G2_PIN%';
            document.getElementById('b2_pin').value = '%B2_PIN%';
            document.getElementById('a_pin').value = '%A_PIN%';
            document.getElementById('b_pin').value = '%B_PIN%';
            document.getElementById('c_pin').value = '%C_PIN%';
            document.getElementById('d_pin').value = '%D_PIN%';
            document.getElementById('e_pin').value = '%E_PIN%';
            document.getElementById('lat_pin').value = '%LAT_PIN%';
            document.getElementById('oe_pin').value = '%OE_PIN%';
            document.getElementById('clk_pin').value = '%CLK_PIN%';
            
            const status = document.getElementById('status');
            status.innerHTML = '<div class="status success">Pin values reset to defaults. Click "Save Pin Configuration" to apply.</div>';
        }
    </script>
</body>
</html>
)";











// Configuration web page
const char CONFIG_PAGE2[] PROGMEM = R"(
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
        .error { background: #f8d7da; color: #721c24; }
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
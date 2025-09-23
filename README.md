ESP32 LED Matrix Display Project README
This project is an ESP32-based firmware for driving HUB75 LED matrix panels (e.g., 64x64 RGB LED matrices). It includes features like:

First-boot configuration mode for WiFi and pin setup via a web interface.
Display effects (e.g., plasma, fire, rain, time display).
Time synchronization via NTP or HTTP.
Graphics rendering with palettes and animations.

The code uses the ESP32-HUB75-MatrixPanel-I2S-DMA library for efficient LED matrix control.
Hardware Requirements

ESP32 development board (e.g., ESP32-DevKitC or similar).
HUB75-compatible LED matrix panel (tested with 64x64 panels).
Jumper wires for connecting pins (default pins are defined in main.cpp).
USB cable for flashing and serial monitoring.
Optional: Button connected to GPIO 0 for reset (BOOT button on most ESP32 boards).

Default pin configuration (can be changed during first boot):

R1: 17, G1: 18, B1: 8
R2: 3, G2: 2, B2: 10
A: 15, B: 11, C: 7, D: 4, E: 13
LAT: 6, OE: 12, CLK: 5

Prerequisites for Development

Install Visual Studio Code (VS Code).
Install the PlatformIO extension in VS Code (search for "PlatformIO IDE" in extensions).
Python 3.x (for esptool if flashing manually).
Git (to clone the repository).

Option 1: Build and Flash from VS Code (Using PlatformIO) - Step by Step for Beginners
This method compiles the code and uploads it to the ESP32 directly from VS Code. No need for Arduino IDE.

Clone the Repository:

Open VS Code.
Go to the Terminal (View > Terminal or Ctrl+`).
Run: git clone[ https://github.com/yourusername/your-repo-name.git](https://github.com/codeboxqc/Panel.git)  
Open the cloned folder in VS Code (File > Open Folder).


Install Dependencies:

PlatformIO will automatically detect the platformio.ini file (if present; otherwise, create one with basic ESP32 config).
In the PlatformIO sidebar (ant icon on the left), click "Build" to compile and download libraries (e.g., ESP32-HUB75-MatrixPanel-I2S-DMA).


Configure PlatformIO.ini (if not already set):

Create or edit platformio.ini in the project root:
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
bodmer/ESP32-HUB75-MatrixPanel-I2S-DMA


Save and reload the project.


Connect ESP32:

Plug your ESP32 into your PC via USB.
Ensure drivers are installed (on Windows, you may need CP210x or CH340 drivers from the manufacturer's site).


Build the Project:

In PlatformIO sidebar, click the "Build" button (checkmark icon).
Wait for compilation to finish. Fix any errors (e.g., missing libraries).


Upload (Flash) the Firmware:

Put ESP32 in bootloader mode: Hold the BOOT button (GPIO 0) while pressing EN/RESET.
In PlatformIO, click "Upload" (right arrow icon).
Release BOOT button after upload starts.
Wait for "Success" message.


Monitor Serial Output:

In PlatformIO, click "Serial Monitor" (plug icon).
This shows logs like "ESP32 Starting..." or "First boot - configuration mode".





//////////////////////////////////////////////////////////////
Option 2: Flash Pre-Built Firmware (.bin) Without Programming - Step by Step for Beginners
If you don't want to build from source, use a pre-built firmware.bin (download from releases or build it yourself via Option 1, then find it in .pio/build/esp32dev/firmware.bin).
This uses esptool.py (no IDE needed).
//////////////////////////////////////////////////////////////////////


Install esptool:

Install Python 3.x from python.org.
Open Command Prompt (Windows) or Terminal (Mac/Linux).
Run: pip install esptool.


Download Firmware:

Get firmware.bin from GitHub releases or build it.
Note the file path (e.g., C:\Downloads\firmware.bin).


Connect ESP32:

Plug ESP32 into PC via USB.


Find COM Port:

Windows: Check Device Manager > Ports (COM & LPT) for "USB-Serial" or similar (e.g., COM3).
Mac/Linux: Run ls /dev/tty* (e.g., /dev/ttyUSB0).


Put ESP32 in Bootloader Mode:

Hold BOOT button (GPIO 0).
Press and release EN/RESET button.
Release BOOT button.


Flash the Firmware:

In Command Prompt/Terminal, run:   esptool.py --chip esp32 --port COM3 --baud 921600 --before default_reset --after hard_reset write_flash -z 0x10000 path/to/firmware.bin



Replace COM3 with your port (e.g., /dev/ttyUSB0).
Replace path/to/firmware.bin with the actual file path.


Wait for "Hash of data verified" and "Hard resetting".


Verify:

Reset ESP32 (press EN/RESET).
Use a serial monitor like PuTTY or CoolTerm (baud 115200) to check logs.



Alternative Tool: Use the ESP32 Web Flasher in Chrome browser for a GUI-based flash (connect via Web Serial).
After Flashing: Configure WiFi and Pins (First Boot Mode) - Step by Step
After flashing, the ESP32 enters "First Boot" mode if no config is saved. It creates a WiFi Access Point (AP) for setup.

Power On and Check Serial:

Connect ESP32 to power/USB.
Open serial monitor (115200 baud).
Look for: "First boot - configuration mode" and "Access point created" with IP (usually 192.168.4.1).


Connect to ESP32 WiFi from PC/Phone:

On your PC or phone, scan for WiFi networks.
Connect to "ESP32Dev-Config" (no password).
If it doesn't connect automatically, manually join (open network).


Open Web Configuration Page:

Open a web browser (Chrome/Firefox).
Go to: http://192.168.4.1
You should see the "ESP32 Configuration" page with WiFi and Pin fields.

 ![11](https://github.com/user-attachments/assets/8cd0688f-fd29-48e3-bb45-1cc8998d98a2)


Enter Configuration:

WiFi: Enter your home WiFi SSID and Password.
Pins: Adjust HUB75 pins if needed (defaults are pre-filled; ensure they match your wiring to avoid damage).
Click "Save Configuration".
Page shows "Configuration saved! ESP32 will restart...".


Verify and Restart:

ESP32 restarts and connects to your WiFi.
Serial log: "Configuration found, loading..." and WiFi connection attempts.
If successful, it displays time/effects on the LED matrix.


Reset Configuration (If Needed):

Double-press the BOOT button (GPIO 0) quickly to reset and re-enter config mode.
Serial shows: "Double press detected - resetting configuration...".



Troubleshooting

No AP Visible: Check serial for errors; re-flash if needed.
Web Page Not Loading: Ensure you're connected to "ESP32Dev-Config"; try incognito mode.
LED Matrix Issues: Verify pins/wiring; brightness too low? Adjust in code (dma_display->setBrightness8(100);).
WiFi Not Connecting: Check SSID/password; ESP32 may retry.
Errors in Build: Ensure libraries are installed; clean build in PlatformIO.

Contributing

Fork the repo, make changes, submit a Pull Request.
Issues: Report bugs or feature requests.

License
MIT License - Free to use/modify.


![2460a20d70e4de6d3298214ffb9f6a54](https://github.com/user-attachments/assets/46db8025-7766-4377-9773-d78f9dca8e93)
![1757968882935](https://github.com/user-attachments/assets/42c1f722-4faa-447a-8d5f-3ceb506a8543)
![Sda4bee1fa1d64fe7be8f3cd4c757da96f](https://github.com/user-attachments/assets/44a9d76b-e05e-421d-a626-35b0df0d00b7)


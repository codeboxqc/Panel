#include <Arduino.h>
#include <math.h>
#include <string.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "gfx.h"

//rle image

const char image[2000] PROGMEM= 
"h648o1g4h1m1g3p1h1o1g3p1h2p1g3h5i1a2i1h1a3h1n1a2h1l1a2h1a2i1h16g1o4h1o5h1o5p2o5h4i1a2i1h1a3h1i1a2h1i1a2h1p1a1i1h16g1o1h4g1p1h1o1g1h1o2h1p1g1p2g1p1h1o1g1h4i4h1i2n1h1i2n1h1i2a1h1i1a1i1h16g1o1h4g1p2h3o2h1p1g1p1h4o1g1h40o1g3p1h1p1g1o1p1h2o2h1p1g1p1h4o2h4l1p2l1h1p3h1p1l1p1h1p3h1p3h16g1o3h3p1g2p1h1o1g3o1j1h2j1o1g1h5l1a1l2h1a1l1p1h1l1a1p1h1l2p1h1p1a1l1h16g1o1h7p1g1h1o2p2j1h3o1g1h6l1p1l2h1l1p1l1h1p1l1p1h1p1l2h1l3h16g1o1h4o1p1h1p1g1h1o2h4p1o2h43g4o1h1o1g3o1h1o2h4p1g1o1g3h4l1d2l1h1d3h1p1d2h1p1d2h1p1d1l1h16p3o1p1h2o1p1o1h2p2h4p1o1p3o1h4l1d2l1h1d3h1d3h1d3h1d3h44l1d2l1h1d3h1d3h1d3h1d3h108l4h1l3h1l3h1l3h1l3h19p4h1p10h1p4h5l1d2l1h1d3h1l1d2h1l1d2h1l1d1l1h18p2h1p2h1p10h1j1p1h1p2h4l1d2l1h1d3h1l1d2h1l1d2h1l1d1l1h18p2h1m1p1h1p1o3p1o1p1o2p1h1m1p1h2p1h4l4h1l3h1l3h1l3h1l3h18p5h1p1o4p5h1k1p4h42p5h1p1o3p2o1p3h1j1p2d1p1h4k1p3h1p3h1p3h1p3h1p3h18p2h1j1p1h1p10h1j1p2l1p1h4k1p2k1h1p3h1p3h1p3h1p3h18p5h1p1o3p6h1i1p4h4k1p2k1h1p3h1p1k1p1h1m1p2h1p2m1h18p2h2p1h1p1o3p6h1j1d1p1d1p1h42p2h2p1h1p10j1m1p4h4m1e2m1h1e3h1m1e2h1m1e2h1p1e1m1h18p5h6j1h3j1h2d1p3h4m1e2m1h1e3h1m1e2h1m1e2h1m1e1m1h18p6h1l1h1l1h1l1h1l1h1l1h1p3d1p1h4m1e2m1h1e3h1m1e2h1m1e2h1e2m1h18p6j1p1h1p1k1p5m1p5h42p2h2p16d1p1h4j1m2j1h1m3h1j1m2h1j1m2h1m2j1h18p2h2p2j2p2j1m1p2j1p5d1p1h4m4h1m3h1m3h1m3h1m3h18p6h2p2h2p1h3p6h4m4h1m3h1m3h1m3h1m3h18p2h1j1p2h2p2h2p2h1j1p3h2p1h5j2h2j3h1j3h1j3h1j3h18j1p1h1p16h1p2h9j1h1j1h7j1h23p5h9j1p5h5n1p2n1h1p3h1p3h1p3h1p3h26p1h4p1h12n1p2n1h1p3h1p3h1p3h1p3h26p1h1p2h1p1j1h11n1p2n1h1p3h1p3h1p3h1p3h25p2h1p2h1p2h178g1p1h2g3p1h2p1g2o1h7g1o1g2p1h2o2h4g1o1h2p1g2h23g1p1h1g1o3g1h1p1g1o2g1p1h6g1o3g1h2o1g1o1h2o1g1o1h2p1g2h23g1p1h1g1p1h1p1g1h1p1g1h2o1p1h6g1p1h1p1o2h1o1g2p2g2o1h2g1p2g1h22g1p1h4p1g1h1o1g1p1h9g1p1h2p1g1h1o2p1o1g1l1g1o1h2g1p2g1h22g1p1h3p1g1p1h2o1g2p1h2g3p1h1g1p1h2p1g1h1o2h1o2h1g1o1h1p1g1h1p1g1h22g1p1h2p1g1p1h4p1o1g1p1h1o3p1h1g1p1h2p1g1h1o2h1p2h1g1o1h1g1o1p2g1o1h21g1p1h1p1g1p1h3p2h2g1o1h6g1p1h2o1g1h1o2h4g1o1h1g5o1h21g1p1h1g1p4h1o1g1p2g1o1h6g1o1p2g1p1h1o2h4g1o1h1g1o1p2g1o1h21g1p1h1g5p1h1o1g2o1h7g4p1h2g1o1h4g1o1h1g1o1h2g1o1h586";



// 16-color palette with letters
struct Color {
    uint8_t r, g, b;
    char letter;
    uint16_t rgb565;
};

const Color pal16[16]  = {
    {255,   0,   0, 'a', 0}, // Red
    {  0,   0, 255, 'b', 0}, // Blue
    {  0, 255,   0, 'c', 0}, // Green
    {255, 255,   0, 'd', 0}, // Yellow
    {  0, 255, 255, 'e', 0}, // Cyan
    {255,   0, 255, 'f', 0}, // Magenta
    {255, 255, 255, 'g', 0}, // White
    {  0,   0,   0, 'h', 0}, // Black
    {128,   0,   0, 'i', 0}, // Dark Red
    {  0,   0, 128, 'j', 0}, // Dark Blue
    {  0, 128,   0, 'k', 0}, // Dark Green
    {128, 128,   0, 'l', 0}, // Olive
    {  0, 128, 128, 'm', 0}, // Teal
    {128,   0, 128, 'n', 0}, // Purple
    {192, 192, 192, 'o', 0}, // Light Gray
    {128, 128, 128, 'p', 0}  // Gray
};


 

// Initialize palette with RGB565 values
void initPalette() {
    for (int i = 0; i < 16; i++) {
        const_cast<Color&>(pal16[i]).rgb565 = fastRGB565(pal16[i].r, pal16[i].g, pal16[i].b);
    }
}

// Decode RLE string to back buffer
void drawRLEImage(const char* rle) {
    int x = 0, y = 0;
    std::string number;
    char color = 0;

    clearTextBuffer(); // Clear the back buffer before drawing

    for (int i = 0; rle[i] != '\0'; i++) {
        if (std::isalpha(rle[i])) {
            if (!number.empty()) {
                int count = std::stoi(number);
                // Find the color in the palette
                uint16_t rgb565 = 0;
                for (int j = 0; j < 16; j++) {
                    if (pal16[j].letter == color) {
                        rgb565 = pal16[j].rgb565;
                        break;
                    }
                }
                // Set pixels for the run
                for (int j = 0; j < count; j++) {
                    if (x >= WIDTH) {
                        x = 0;
                        y++;
                    }
                    if (y < HEIGHT) {
                        setBufferPixel(x, y, rgb565);
                    }
                    x++;
                }
                number.clear();
            }
            color = rle[i];
        } else if (std::isdigit(rle[i])) {
            number += rle[i];
        }
    }
    // Handle the last run
    if (!number.empty()) {
        int count = std::stoi(number);
        uint16_t rgb565 = 0;
        for (int j = 0; j < 16; j++) {
            if (pal16[j].letter == color) {
                rgb565 = pal16[j].rgb565;
                break;
            }
        }
        for (int j = 0; j < count; j++) {
            if (x >= WIDTH) {
                x = 0;
                y++;
            }
            if (y < HEIGHT) {
                setBufferPixel(x, y, rgb565);
            }
            x++;
        }
    }
}



void logo()
{

    initPalette();
    drawRLEImage(image);
    pageFlip();
    delay(2000);


}



/*
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

// Define a simple 16-color palette (RGB values)
struct Color {
    unsigned char r, g, b;
    char letter;
};

const Color palette[16] = {
    {255,   0,   0, 'a'}, // Red
    {  0,   0, 255, 'b'}, // Blue
    {  0, 255,   0, 'c'}, // Green
    {255, 255,   0, 'd'}, // Yellow
    {  0, 255, 255, 'e'}, // Cyan
    {255,   0, 255, 'f'}, // Magenta
    {255, 255, 255, 'g'}, // White
    {  0,   0,   0, 'h'}, // Black
    {128,   0,   0, 'i'}, // Dark Red
    {  0,   0, 128, 'j'}, // Dark Blue
    {  0, 128,   0, 'k'}, // Dark Green
    {128, 128,   0, 'l'}, // Olive
    {  0, 128, 128, 'm'}, // Teal
    {128,   0, 128, 'n'}, // Purple
    {192, 192, 192, 'o'}, // Light Gray
    {128, 128, 128, 'p'}  // Gray
};

 

// Find the closest color in the palette
char getClosestColor(unsigned char r, unsigned char g, unsigned char b) {
    int minDist = INT_MAX;
    char closestLetter = 'h'; // Default to black
    for (const auto& color : palette) {
        int dist = (r - color.r) * (r - color.r) +
            (g - color.g) * (g - color.g) +
            (b - color.b) * (b - color.b);
        if (dist < minDist) {
            minDist = dist;
            closestLetter = color.letter;
        }
    }
    return closestLetter;
}

////////////////////////////
// Apply Run-Length Encoding (RLE)
std::string applyRLE(const std::vector<char>& colors) {
    std::string output;
    if (colors.empty()) return output;

    char currentColor = colors[0];
    int count = 1;

    for (size_t i = 1; i < colors.size(); ++i) {
        if (colors[i] == currentColor && count < 9999) { // Limit count to avoid overflow in string
            count++;
        }
        else {
            output += currentColor + std::to_string(count);
            currentColor = colors[i];
            count = 1;
        }
    }
    output += currentColor + std::to_string(count); // Append the last run

    return output;
}

 

void rle() {
    // Load PNG image
    int width, height, channels;
    unsigned char* img = stbi_load("input.png", &width, &height, &channels, 3); // Force RGB
    if (!img) {
        std::cerr << "Failed to load image!" << std::endl;
        return ;
    }

    // Convert image to 16-color sequence
    std::vector<char> colors;
    for (int i = 0; i < width * height; ++i) {
        unsigned char r = img[i * 3];
        unsigned char g = img[i * 3 + 1];
        unsigned char b = img[i * 3 + 2];
        colors.push_back(getClosestColor(r, g, b));
    }

    // Apply RLE compression
    std::string output = applyRLE(colors);

    // Save as char array format
    std::ofstream outFile("output_rle_image.h");
    outFile << "char image[] = \"" << output << "\";\n";
    outFile.close();

    // Free image memory
    stbi_image_free(img);

    std::cout << "Image processed and saved as output_image.h" << std::endl;
     
}
/// ////////////////////////////////////////////////
/// 
/// </summary>
/// <returns></returns>

int main() {
    // Load PNG image
    int width, height, channels;
    unsigned char* img = stbi_load("input.png", &width, &height, &channels, 3); // Force RGB
    if (!img) {
        std::cerr << "Failed to load image!" << std::endl;
        return 1;
    }

    // Convert image to 16-color string
    std::string output;
    for (int i = 0; i < width * height; ++i) {
        unsigned char r = img[i * 3];
        unsigned char g = img[i * 3 + 1];
        unsigned char b = img[i * 3 + 2];
        char colorLetter = getClosestColor(r, g, b);
        output += colorLetter + std::to_string(i);
    }

    // Save as char array format
    std::ofstream outFile("output_image.h");
    outFile << "char image[] = \"" << output << "\";\n";
    outFile.close();


    rle();

    // Free image memory
    stbi_image_free(img);

    std::cout << "Image processed and saved as output_image.h" << std::endl;
    return 0;
}
*/
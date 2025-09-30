#include <Arduino.h>
#include <math.h>
#include <string.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "gfx.h"

//rle image

const char image[] = "h285l1p1l1k1l2h53p2o3g6o2p2h46p2o1g15o1p1h43p1o1g19o1p1h40o1g20o2g1o1p1h36p1o1g22o3g1o1h34p1o1g24o5h32p1o1g26o5h30p1o1g27o5p1h28p1o1g28o6p1h27p1g29o4p1o2h26p1o1g30o3p2o1p1h24p1o1g30o4p2o2h24p1o1g30o4p3o1p1h23p1g31o4p3o2h22l1o1g31o4p3o2p1h21p1o1g30o4p1o1p3o1p1h21p1g29o7p4o2h21p1o1g6o4g17o8p5o1h21o1g6p1h4i1o1g13o3p1h4p1o2p4o1p1h20o2g4p1h7p1g9o4p1h8o1p4o1p1h20o2g3p1h4p1k1h3o1g1o1g2o8h9p1o1p3o1p1h20o2g2o1h4m4h2p1g1o10p1h3m3k1h3o1p3o1p1h20o2g2p1h3m5k1h2o11h3p1m5h2p1o1p4h20o2g2p1h2k1m5k1h2o11h2m7h2p1o1p4h20o3g1p1h2m6k1h2o4g2o5h2p1m7h1p1o1p4h20p1o2g1o1h2m6k1h2g1o3p2l1o4h3m6h2p1o1p4h20p1o2g2h3m5h2p1g1o1g1p1h3p1o3i1h2p1m4k1h2g1o1p4h21o2g2p1h3k3h3o4h5o3p1h3m2k1m1h2p1g1p3o1p1h21o3g2p1h7p1g1o3h5o4p1h3k1h3p1g1o1p3o1p1h21p1o3g2p1h5o1g1o3g1h5o5p1h5p1g1o1p5h22p1o4g3o3g2o5h5o5g1o1p3o1g1o1p4o1p1h23o5g6o6p1o2p2o7g3o1p5o1p1h24p1g1o15g1o1g1o11p6o2p1h25p1g1o25p8o2p1h27p1o25p6o3p1h29p2o11g1o18p2h33p6o4p2o3h1j1o5p7h40p1o4h1p1o3h2o5p1h46p1g1o3h1p1g1o2h2o5h48o3p1h1p1o1p2h2p2o1p2h48j1h14p1h60j2p2h49m1p1h9j1p3h48p3j1h8m1p4h46p4m1j8p6h44m1p4m1j8p6h44p1o1p3m1j7h1p7h42p1g2p4m1p5m1j1p5o1p2h40i1g1o3p10h1p1g2o3p2h40p1o6p9o7p2h39o7p1h6p2o1p1o2p1o2p2h38p1o7p1h7p1o2p1o1p2o1p2h38p1o7p2h6p1o3p1o1p1o1p2h39p1o6p2h6p2o6p2h40p7h8o1p7h43p4h229";


// 16-color palette with letters
struct Color {
    uint8_t r, g, b;
    char letter;
    uint16_t rgb565;
};

const Color pal16[16] PROGMEM= {
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
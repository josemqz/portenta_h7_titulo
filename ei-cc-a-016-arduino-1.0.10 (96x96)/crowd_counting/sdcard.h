#ifndef SDCARD_H
#define SDCARD_H

#include "SDMMCBlockDevice.h" // Multi Media Card APIs
#include "FATFileSystem.h"    // API to run operations on a FAT file system
//#include "camera.h" // Arduino Mbed Core Camera APIs
//#include "himax.h"  // API to read from the Himax camera found on the Portenta Vision Shield

// Settings for our setup
#define IMAGE_HEIGHT (unsigned int)96
#define IMAGE_WIDTH (unsigned int)96
//#define IMAGE_MODE CAMERA_GRAYSCALE
#define BITS_PER_PIXEL (unsigned int)8
#define PALETTE_COLORS_AMOUNT (unsigned int)(pow(2, BITS_PER_PIXEL))
#define PALETTE_SIZE  (unsigned int)(PALETTE_COLORS_AMOUNT * 4) // 4 bytes = 32bit per color (3 bytes RGB and 1 byte 0x00)

// Headers info
#define BITMAP_FILE_HEADER_SIZE (unsigned int)14 // For storing general information about the bitmap image file
#define DIB_HEADER_SIZE (unsigned int)40 // For storing information about the image and define the pixel format
#define HEADER_SIZE (BITMAP_FILE_HEADER_SIZE + DIB_HEADER_SIZE)

boolean mountSDCard(mbed::FATFileSystem *fs, SDMMCBlockDevice *blockDevice);
void setFileHeaders(unsigned char *bitmapFileHeader, unsigned char *bitmapDIBHeader, int fileSize);
void setColorMap(unsigned char *colorMap);
void saveImage(uint8_t *imageData, char* filename, int imageNum);
void countDownBlink();

#endif SDCARD_H

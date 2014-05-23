//Camera.h by RorschachUK September 2011
#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "svt.h"

//Used in quantizing, must be divisors of 320 and 240
#define BLOCKSIZE_X 10
#define BLOCKSIZE_Y 10

//Takes a picture and stores it as a BMP format in the given filename
void TakePicture(char *filename);

//Stores a table of average pixel colors for blocks BLOCKSIZE_X by BLOCKSIZE_Y.
//Buffer must be declared as Color buffer[(320 * 240 / (BLOCKSIZE_X * BLOCKSIZE_Y))]
void QuantizeImage(Color *buffer);

//Compares two buffers of quantized pixel block color averages, and
//returns true if they are substantially the same, false if different
int CompareQuantizedImages(Color *bufferReference, Color *bufferComparison, int threshold, int maxdifferences);

#endif

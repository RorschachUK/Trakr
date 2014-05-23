//Camera.c by RorschachUK September 2011
#include "svt.h"
#include "camera.h"

//Simply ensure value is between 0 and 255
int clip(int x) {
	if (x < 0)
		return 0;
	else if (x > 255)
		return 255;
	else
		return x;
}

//Convert four bytes of YUV data to two RGB pixels represented by Color structs
void ConvertPixels(unsigned char Y0, unsigned char U, unsigned char Y1, unsigned char V, Color *pixel1, Color *pixel2) {
	int C, D, E;
	//Do the YUV calculation voodoo
	C = Y0 - 16;
	D = U - 128;
	E = V - 128;

	pixel1->R = clip(( 298 * C + 409 * E + 128) >> 8);
	pixel1->G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8);
	pixel1->B = clip((298 * C + 516 * D + 128) >> 8);
	pixel1->Transparent=0;

	//Repeat with the Y1 value
	C = Y1 - 16;

	pixel2->R = clip(( 298 * C + 409 * E + 128) >> 8);
	pixel2->G = clip(( 298 * C - 100 * D - 208 * E + 128) >> 8);
	pixel2->B = clip((298 * C + 516 * D + 128) >> 8);
	pixel2->Transparent=0;
}

//TakePicture - write the camera buffer to file
void TakePicture(char *filename) {
	unsigned char *imageBuffer;
	imageBuffer = (unsigned char *) (*((uint32 *) 0xFFF0700C));  //Jolomika's magic number
	int Y0, Y1, U, V;
	int x,y, bytepos, writepos;
	uint32 out[13];
	unsigned char outC;
	unsigned char pix[320*3]; // a line of RGB data
	Color pixel1, pixel2;

	DeleteFile(filename);
	CreateFile(filename);
	File f = OpenFile(filename);

	if (f != -1) {
		//File type identifier - BM identifies a bitmap file
		outC='B';
		WriteFile(f, &outC, 1);
		outC='M';
		WriteFile(f, &outC, 1);

		//Now the bmp header
		out[0] = 54 + 3 * 320 * 240;	//file size
		out[1] = 0;						//app specific
		out[2] = 54;					//offset to pixel data

		//Now the DIB header
		out[3] = 40;					//length of DIB header
		out[4] = 320;					//width
		out[5] = 240;					//height
		out[6] = 0x180001; 				//1 plane, 24 bits per plane
		out[7] = 0;						//compression type - none
		out[8] = 3 * 320 * 240;			//pixel data size
		out[9] = 0x0B13;				//horizontal resolution
		out[10] = 0x0B13;				//vertical resolution
		out[11] = 0;					//no indexed colours
		out[12] = 0;					//no important colours
		WriteFile(f, &out[0], 52);

		//Translate YUV to RGB and write the RGB data
		for (y=240; y>0; y--) { //Scan upwards through lines from bottom up
			writepos=0;
			for (x=0; x<640; x+=4) { //Scan across lines
				//Extract YUV data
				bytepos=y*640+x;
				Y0 = imageBuffer[bytepos];
				U = imageBuffer[bytepos + 1];
				Y1 = imageBuffer[bytepos + 2];
				V = imageBuffer[bytepos + 3];

				//Convert to RGB
				ConvertPixels(Y0, U, Y1, V, &pixel1, &pixel2);

				//Save RGB to file
				pix[writepos++]=pixel1.B;
				pix[writepos++]=pixel1.G;
				pix[writepos++]=pixel1.R;
				pix[writepos++]=pixel2.B;
				pix[writepos++]=pixel2.G;
				pix[writepos++]=pixel2.R;
			}
			WriteFile(f, &pix[0], 320*3);
		}
		CloseFile(f);
	}
	FlushFile(f);
}

//Calculate the average colour of a block BLOCKSIZE_X x BLOCKSIZE_Y from the start pos bufferpos
Color QuantizeBlock(unsigned char *bufferpos) {
	int x, y, r, g, b, Y0, Y1, U, V, bytepos;
	Color pixel1, pixel2, ret;
	r=0; g=0; b=0;
	for (y=0; y< BLOCKSIZE_Y; y++) {
		for (x=0; x<BLOCKSIZE_X / 2; x++) {
			bytepos = y * 320 * 2 + x * 4;
			//Log("QB: x=%d, y=%d, bp=%d", x, y, bytepos);
			Y0=bufferpos[bytepos];
			U=bufferpos[bytepos+1];
			Y1=bufferpos[bytepos+2];
			V=bufferpos[bytepos+3];

			//Convert YUV to RGB
			ConvertPixels(Y0, U, Y1, V, &pixel1, &pixel2);

			r += pixel1.R + pixel2.R;
			g += pixel1.G + pixel2.G;
			b += pixel1.B + pixel2.B;
		}
	}
	ret.R = r / (BLOCKSIZE_X * BLOCKSIZE_Y);
	ret.G = g / (BLOCKSIZE_X * BLOCKSIZE_Y);
	ret.B = b / (BLOCKSIZE_X * BLOCKSIZE_Y);
	ret.Transparent=0;
	return ret;
}

//Calculate a quantized average of the current camera image and put it in the specified buffer
void QuantizeImage(Color *buffer) {
	unsigned char *imageBuffer;
	imageBuffer = (unsigned char *) (*((uint32 *) 0xFFF0700C));
	int x, y, bytepos;
	Color average;
	for (y = 0; y * BLOCKSIZE_Y < 240; y++) {
		for (x = 0; x * BLOCKSIZE_X < 320; x++) {
			bytepos = (y * BLOCKSIZE_Y) * 320 * 2 + (x * BLOCKSIZE_X);
			average = QuantizeBlock(&imageBuffer[bytepos]);
			buffer[(320/BLOCKSIZE_X) * y + x] = average;
		}
	}
}

//abs value of an int
int absint(int inp) {
	if (inp<0)
		return -inp;
	else
		return inp;
}

//Compare two colours to return an absolute difference between the R, G and B components
int CompareColor(Color *pixel1, Color *pixel2) {
	return absint(pixel1->R - pixel2->R) + absint(pixel1->G - pixel2->G) + absint(pixel1->B - pixel2->B);
}

//
int CompareQuantizedImages(Color *bufferReference, Color *bufferComparison, int threshold, int maxdifferences) {
	int x, y, bytepos, hitcount;
	hitcount=0;
	//Scan through blocks comparing averaged values between reference and comparison
	for (y = 0; y * BLOCKSIZE_Y < 240; y++) {
		for (x = 0; x * BLOCKSIZE_X < 320; x++) {
			bytepos = ((y * 320) / BLOCKSIZE_Y) + x;
			//Check for a change over the supplied threshold
			if (CompareColor(&bufferReference[bytepos], &bufferComparison[bytepos]) > threshold) {
				hitcount++;
				//if we reach a significant number of changes, give up
				if (hitcount > maxdifferences)
					return maxdifferences;
			}
		}
	}
	return hitcount;
}

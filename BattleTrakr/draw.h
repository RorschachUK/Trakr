//Draw.h by RorschachUK
//A simple line-drawing and 2D vector graphics library

#ifndef __DRAW_H__
#define __DRAW_H__

//Convert RGB values (and transparency flag) into a Color struct
Color RGBColor(unsigned char R, unsigned char G, unsigned char B, unsigned char T);

//Some standard colours
#define WHITE RGBColor(255,255,255,0)
#define BLACK RGBColor(0,0,0,0)
#define RED RGBColor(255,0,0,0)
#define GREEN RGBColor(0,255,0,0)
#define BLUE RGBColor(0,0,255,0)
#define CYAN RGBColor(0,255,255,0)
#define YELLOW RGBColor(255,255,0,0)
#define MAGENTA RGBColor(255,0,255,0)
#define GREY RGBColor(128,128,128,0)
#define DARK_RED RGBColor(128,0,0,0)
#define DARK_GREEN RGBColor(0,128,0,0)
#define DARK_BLUE RGBColor(0,0,128,0)
#define CLEAR RGBColor(255,255,255,255)

//General line drawing routines

//Plot a point on the screen in a given colour
void Plot(int x, int y, Color c);

//Draw a line connecting two points, in a given colour
void DrawLine(int x0, int y0, int x1, int y1, Color c);

//Draw a circle with a given centre, radius and colour
void DrawCircle(int xCenter, int yCenter, int radius, Color c);


//2D Vector stuff

//For polygons we'll use an array of points
typedef struct {
	int x;
	int y;
} iPoint;

//Draw a polygon with a given set of points, in a given colour
void DrawPoly(int numPoints, iPoint points[], Color c);

//Rotate a point a given angle around an origin
void RotatePoint(iPoint *point, int origX, int origY, float radAngle);

//Rotate an entire polygon about an origin point
void RotatePoly(int numPoints, iPoint polygon[], int origX, int origY, float radAngle);

//Shift a polygon in 2D space
void MovePoly(int numPoints, iPoint polygon[], int shiftX, int shiftY);

//Rescale a point with respect to an origin and scale factor
void ScalePoint(iPoint *point, int origX, int origY, float scaleFactor);

//Rescale the dimensions of a polygon
void ScalePoly(int numPoints, iPoint polygon[], int origX, int origY, float scaleFactor);

//Math approximations - quick & dirty for faster graphics but NOT accurate

#define PI 3.14159265

//Quick square root
float qsqrt(float number);

//Quick sine
float qsin(float number);

//Quick cosine
float qcos(float number);

//Quick arctangent
double qatan(double x);

#endif

#include "svt.h"
#include "draw.h"

//Draw.c by RorschachUK

//Almost everything in here has been found in public domain on the internet
//(There's nothing new under the sun).  I tried a few different ways to
//get points plotted but this was the only one I found to be relatively
//stable - it's pretty naive though (1x1 rectangles).  The Trakr's
//architecture sends graphics as a series of tokenised commands and
//parameters from the vehicle to the remote control, where they are
//decoded and drawn - there are no commands covering points, lines or
//circles so I have to fall back on this method of creating a succession
//of pixel-width thin rectangles which end up being squirted through
//the radio link and drawn back on the remote.  In other words, it is
//never going to be quick and efficient until there are more drawing
//primitives supported by the tokenising scheme and the remote's
//firmware - and so I didn't bother adding fill routines because they
//would be painfully slow.

//I've included fast approximation functions for some maths calculations
//which should be good enough for graphics purposes but are not accurate
//and repeated application will lead to cumulative errors.

//Convert RGB values (and transparency flag) into a Color struct
Color RGBColor(unsigned char R, unsigned char G, unsigned char B, unsigned char T) {
	Color ret;
	ret.R=R;
	ret.G=G;
	ret.B=B;
	ret.Transparent=T;
	return ret;
}

//Plot a point on the screen in a given colour by drawing a rectangle from x,y to x,y
void Plot(int x, int y, Color c) {
	//Check screen bounds
	if(x>=0 && x <=159 && y >= 0 && y <= 119) {
		//Check if the colour is transparent (i,e. unplot the point)
		if (c.Transparent==0)
			DrawRectangle(x,y,x,y,c); 	//Plot with the colour
		else
			ClearRectangle(x,y,x,y);	//Or, unplot to be clear
	}
}

//Used in DrawLine to draw a thin pixel-width rectangle
void DrawVerticalLine(int x, int y, int inc, Color c) {
	//Check screen bounds
	if(x>=0 && x <= 159) { //x is onscreen
		if (!((y > 119 && y + inc > 119) || (y < 0 && y + inc < 0))) { //the line is onscreen
			//line may need clipping
			if ((y + inc) > 119)
				inc = 119 - y;
			else if ((y + inc) < 0)
				inc = -y;
			if (y > 119) {
				inc += y-119;
				y=119;
			} else if (y < 0) {
				inc += y;
				y=0;
			}
			//Check if the colour is transparent (i,e. unplot the point)
			if (c.Transparent==0)
				DrawRectangle(x,y,x,y+inc,c); 	//Plot with the colour
			else
				ClearRectangle(x,y,x,y+inc);	//Or, unplot to be clear
		}
	}
}

//Used in DrawLine to draw a thin pixel-height rectangle
void DrawHorizontalLine(int x, int y, int inc, Color c) {
	//Check screen bounds
	if(y >= 0 && y <= 119) { //y is onscreen
		if (!((x > 159 && x + inc > 159) || (x < 0 && x + inc < 0))) { //the line is onscreen
			//line may need clipping
			if ((x + inc) > 159)
				inc = 159 - x;
			else if ((x + inc) < 0)
				inc = -x;
			if (x > 159) {
				inc += x-159;
				x=159;
			} else if (x < 0) {
				inc += x;
				x=0;
			}
			//Check if the colour is transparent (i,e. unplot the point)
			if (c.Transparent==0)
				DrawRectangle(x,y,x+inc,y,c); 	//Plot with the colour
			else
				ClearRectangle(x,y,x+inc,y+inc);	//Or, unplot to be clear
		}
	}
}

//Draw a line connecting two points, in a given colour.  This routine works
//out an optimised series of points to plot via Bresenham's algorithm, found
//online here: http://groups.csail.mit.edu/graphics/classes/6.837/F01/Lecture04/Slide20.html
//further optimised for the Trakr architecture by drawing horizontal or
//vertical segments as single pixel-width rectangles
void DrawLine(int x0, int y0, int x1, int y1, Color col) {
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy, inc;

	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;
	dx <<= 1;

	Plot(x0, y0, col);
	inc=0;
	if (dx > dy) {
		//wider than tall - optimise to draw horizontal rectangles 1 pixel high
		int fraction = dy - (dx >> 1);
		while (x0 + inc != x1) {
			if (fraction >= 0) {
				//changing y - draw rectangle
				DrawHorizontalLine(x0,y0,inc,col);
				x0+=inc;
				inc=0;
				y0 += stepy;
				fraction -= dx;
			}
			inc += stepx;
			fraction += dy;
		}
		DrawHorizontalLine(x0,y0,inc,col);
	} else {
		//taller than wide - optimise to draw vertical rectangles 1 pizel wide
		int fraction = dx - (dy >> 1);
		while (y0 + inc != y1) {
			if (fraction >= 0) {
				//changing x - draw rectangle
				DrawVerticalLine(x0,y0,inc,col);
				y0+=inc;
				inc=0;
				x0 += stepx;
				fraction -= dy;
			}
			inc += stepy;
			fraction += dx;
		}
		DrawVerticalLine(x0,y0,inc,col);
	}
}

//Fast sin approximation from a polynopmial parabola segment
//Found online here: http://lab.polygonal.de/2009/01/21/benchmarking-gotchas/
float qsin(float number) {
	//always wrap input angle to -PI..PI
	while (number < -PI)
		number += PI * 2.0;
	while (number >  PI)
		number -= PI * 2.0;

	//compute sine
	if (number < 0)
		return 1.27323954 * number + .405284735 * number * number;
	else
		return 1.27323954 * number - 0.405284735 * number * number;
}

//fast cos approximation, just shifts the input by pi/2 to make it sin
float qcos(float number) {
	number += 1.57079632;
	return qsin(number);
}

//arctan approximation
//found online here: http://www.ganssle.com/approx/approx.pdf
double qatan(double x) {
	const double tantwelfthpi=0.26794919243112271;
	const double tansixthpi=0.57735026918962576;
	const double c1=1.6867629106;
	const double c2=0.4378497304;
	const double c3=1.6867633134;
	double y;				// return from atan__s function
	double x2;
	bool complement = false;// true if arg was >1
	bool region = false;	// true depending on region arg is in
	bool sign = false;		// true if arg was < 0

	if (x <0 ){
		x=-x;
		sign=true;			// arctan(-x)=-arctan(x)
	}
	if (x > 1.0){
		x=1.0/x;			// keep arg between 0 and 1
		complement=true;
	}
	if (x > tantwelfthpi){
		x = (x-tansixthpi)/(1+tansixthpi*x); // reduce arg to under tan(pi/12)
		region=true;
	}

	x2=x * x;
	y = (x*(c1 + x2*c2)/(c3 + x2));

	if (region) y += PI / 6.0;
	if (complement) y = PI / 2.0 - y;
	if (sign) y = -y;
	return (y);
}

//A square root approximation, It's from John Carmack's fast inverse
//square root from the Quake III source code.  Trakr seemed confused
//about whether it supported sqrt - there's a <math.h>, but if I use it
//the function's not linked.  OTOH if I call this routine sqrt instead
//of qsqrt, the compiler complains I'm redefining an existing function!
float qsqrt(float number) {
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 ); // <- ??? might as well be magic
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );	// Couple of iterations of Newton's Method
    y  = y * ( f - ( x * y * y ) );	// to improve the approximation
    return number * y;
}

//Draw a circle with a given centre, radius and colour.  Uses
//symmetry so it only needs to figure out one eighth of it and
//mirrors those points to the other seven eighths.
void DrawCircle(int xCenter, int yCenter, int radius, Color c) {
	int x, y, r2;

	r2 = radius * radius;
	Plot(xCenter, yCenter + radius, c);
	Plot(xCenter, yCenter - radius, c);
	Plot(xCenter + radius, yCenter, c);
	Plot(xCenter - radius, yCenter, c);

	y = radius;
	x = 1;
	y = (int) (qsqrt(r2 - 1) + 0.5);
	while (x < y) {
		Plot(xCenter + x, yCenter + y, c);
		Plot(xCenter + x, yCenter - y, c);
		Plot(xCenter - x, yCenter + y, c);
		Plot(xCenter - x, yCenter - y, c);
		Plot(xCenter + y, yCenter + x, c);
		Plot(xCenter + y, yCenter - x, c);
		Plot(xCenter - y, yCenter + x, c);
		Plot(xCenter - y, yCenter - x, c);
		x += 1;
		y = (int) (qsqrt(r2 - x*x) + 0.5);
	}
	if (x == y) {
		Plot(xCenter + x, yCenter + y, c);
		Plot(xCenter + x, yCenter - y, c);
		Plot(xCenter - x, yCenter + y, c);
		Plot(xCenter - x, yCenter - y, c);
	}
}

//Draw a set of points as a polygon.  Join up the first
//and last points to close the poly.
void DrawPoly(int numPoints, iPoint points[], Color c) {
	//quit if not at least two points
	if (numPoints<2)
		return;
	//loop through each line to draw
	int i, x0, y0, x1, y1;
	for (i=1;i<numPoints;i++) {
		x0=points[i-1].x;
		y0=points[i-1].y;
		x1=points[i].x;
		y1=points[i].y;
		DrawLine(x0, y0, x1, y1, c);
	}
	//final line to join it back up
	if (numPoints>2) {
		x1=points[0].x;
		y1=points[0].y;
		x0=points[numPoints-1].x;
		y0=points[numPoints-1].y;
		DrawLine(x0, y0, x1, y1, c);
	}
}

//Rotate a point a given angle around an origin
void RotatePoint(iPoint *point, int origX, int origY, float radAngle) {
	int x =  origX + (int) ((point->x - origX) * qcos(radAngle) - (point->y - origY) * qsin(radAngle));
	int y = origY + (int) ((point->x - origX) * qsin(radAngle) + (point->y - origY) * qcos(radAngle));
	point->x=x;
	point->y=y;
}

//Rotate an entire polygon about an origin point
void RotatePoly(int numPoints, iPoint polygon[], int origX, int origY, float radAngle) {
	int i;
	for(i=0; i<numPoints;i++)
		RotatePoint(&polygon[i], origX, origY, radAngle);
}

//Shift a polygon in 2D space
void MovePoly(int numPoints, iPoint polygon[], int shiftX, int shiftY) {
	int i;
	for(i=0; i<numPoints; i++) {
		polygon[i].x += shiftX;
		polygon[i].y += shiftY;
	}
}

//Rescale a point with respect to an origin and scale factor
void ScalePoint(iPoint *point, int origX, int origY, float scaleFactor) {
	int x = origX + (int) ((point->x - origX) * scaleFactor);
	int y = origY + (int) ((point->y - origY) * scaleFactor);
	point->x=x;
	point->y=y;
}

//Rescale the dimensions of a polygon
void ScalePoly(int numPoints, iPoint polygon[], int origX, int origY, float scaleFactor) {
	int i;
	for(i=0; i<numPoints;i++)
		ScalePoint(&polygon[i], origX, origY, scaleFactor);
}

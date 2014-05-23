/* Vector.h by RorschachUK
A simple 3d wireframe vector graphics library

A 'world' is composed of a number of 'objects',
Each object is composed of a number of 'faces',
Each face is composed of a number of 'edges',
Each edge connects two 'points'.

A camera in the world has a position and a direction.
The reference orientation for this camera (angle 0,0,0)
is to look forward along the z axis, with the y axis
being straight up from the origin and the x axis
extending straight to the right.
*/

#ifndef __VECTOR_H__
#define __VECTOR_H__

//3d vector stuff

/*
I really want to do this with linked lists, using
malloc for dynamic allocation - doesn't seem to
work though, so I'm left with fixed arrays of
structs.
*/
#define MAXEDGESINFACE 10
#define MAXOBJECTSINWORLD 20
#define MAXFACESINOBJECT 20
#define MAXPOINTSINOBJECT 50
#define MAXEDGESINOBJECT 50

//A point in 3d space
typedef struct {
	double x;
	double y;
	double z;
} Point3d;

//An edge - connects two points, whose index positions are given
typedef struct {
	int start;
	int end;
	bool behind;
} Edge3d;

//A face - which has a given number of edges, whose index positions are given in the array
typedef struct {
	int numEdges;
	bool behind;
	int edges[MAXEDGESINFACE];
} Face3d;

//An object has a collection of points, edges connecting those points,
//faces joining those edges, a heading, a centre and a colour
typedef struct {
	Color colour;
	Point3d centre;
	Point3d heading;
	int numPoints;
	int numEdges;
	int numFaces;
	Point3d points[MAXPOINTSINOBJECT];
	Edge3d edges[MAXEDGESINOBJECT];
	Face3d faces[MAXFACESINOBJECT];
} Object3d;

//A world has a collection of objects
typedef struct {
	int numObjects;
	Object3d objects[MAXOBJECTSINWORLD];
} World3d;

//Create an point from x, y, z
Point3d CreatePoint(double x, double y, double z);

//Add a point to another
Point3d AddPoint(Point3d point1, Point3d point2);

//Subtract one point from another
Point3d SubtractPoint(Point3d point1, Point3d point2);

//Create an edge
Edge3d CreateEdge(int start, int end);

/* Create an object from a colour and sequential poly data
Format: after specifying the colour, the number of points
in the 3d object, the number of edges and the number of
faces, the rest of the parameters to CreateObject supply
the point data (three doubles per point for x, y and z),
the edge data (pairs of zero-based index integers for
the start and end points, referring to the index positions
in the list of points data already given), and the face
data which consists, for each face, of an integer
specifying the number of edges in the face, then that
number of integers giving the index positions of the
edge data already given.  Examples may help clarify this.
*/
Object3d CreateObject(Color colour, Point3d centre, int numPoints, int numEdges, int numFaces, ...);

//Create a world from sequential poly data
World3d CreateNewWorld();

//Add an object to the world, returns the index position within the world
int AddObjectToWorld(World3d *world, Object3d object);

//Constrain angle to 0..2PI
double ClipAngle(double angle);

//Rotate an object in 3d space about the X axis centred on a given point
void RotateObjectXAxis(Object3d *object, float radAngle);

//Rotate an object in 3d space about the Y axis centred on a given point
void RotateObjectYAxis(Object3d *object, float radAngle);

//Rotate an object in 3d space about the Z axis centred on a given point
void RotateObjectZAxis(Object3d *object, float radAngle);

//Move an object in 3d space
void MoveObject(Object3d *object, Point3d shiftVector);

//Scale an object by a scale factor with respect to their centre
void ScaleObject(Object3d *object, float scaleFactor);

//Transform an individual point with regards to camera position and direction
Point3d TransformPoint(Point3d point, Point3d cameraPoint, Point3d cameraDirection);

//Draw the world from the perspective of a given camera
void DrawWorld(World3d *world, Point3d cameraPoint, Point3d cameraDirection);

//Erase the drawn world from the perspective of a given camera
void EraseWorld(World3d *world, Point3d cameraPoint, Point3d cameraDirection);

#endif

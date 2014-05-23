#include "svt.h"
#include "vector.h"
#include "draw.h"
#include <stdarg.h>

/* Vector.c by RorschachUK
A simple 3d wireframe vector graphics library

A 'world' is composed of a number of 'objects',
Each object is composed of a number of 'faces',
Each face is composed of a number of 'edges',
Each edge connects two 'points'.

A camera in the world has a position and a direction.
The reference orientation for this camer (angle 0,0,0)
is to look forward along the z axis, with the y axis
being straight up from the origin and the x axis
extending straight to the right.
*/

Point3d transformedObjectPoints[MAXPOINTSINOBJECT]; //store the transformed points for normals calculation
Point3d projected[MAXPOINTSINOBJECT];	//Array of points projected onto viewing surface
Point3d transformedCentres[MAXOBJECTSINWORLD];	//Centre points
int sortedObjects[MAXOBJECTSINWORLD];	//Sorted list of objects, so we can draw from the back forward.
static Point3d cosCamera, sinCamera;

//Create an point from x, y, z
Point3d CreatePoint(double x, double y, double z) {
	Point3d newPoint;
	newPoint.x=x;
	newPoint.y=y;
	newPoint.z=z;
	return newPoint;
}

//Add one point to another
Point3d AddPoint(Point3d point1, Point3d point2) {
	return CreatePoint(point1.x + point2.x, point1.y + point2.y, point1.z + point2.z);
}

//Subtract one point from another
Point3d SubtractPoint(Point3d point1, Point3d point2) {
	return CreatePoint(point1.x - point2.x, point1.y - point2.y, point1.z - point2.z);
}

//Create an edge from start and end point index positions
Edge3d CreateEdge(int start, int end) {
	Edge3d newEdge;
	newEdge.start=start;
	newEdge.end=end;
	return newEdge;
}

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
Object definition adapted from source found here:
http://www.cs.umd.edu/~mount/Indep/Connie_Peng/wireframe/proj.html
*/
Object3d CreateObject(Color colour, Point3d centre, int numPoints, int numEdges, int numFaces, ...) {
	Object3d obj;
	va_list ap;
	obj.colour=colour;
	obj.numPoints=numPoints;
	obj.numEdges=numEdges;
	obj.numFaces=numFaces;
	obj.centre=centre;
	obj.heading=CreatePoint(0,0,0);
	va_start(ap, numFaces);
	int i, j, numEdgesThisFace,thisEdge;
	Face3d face;
	//Point data - numPoints lots of 3 doubles
	for(i=0;i<numPoints;i++) {
		obj.points[i]=CreatePoint(va_arg(ap,double),va_arg(ap,double),va_arg(ap,double));
	}
	//Edge data - numEdges lots of 2 ints
	for (i=0; i<numEdges;i++) {
		obj.edges[i]=CreateEdge(va_arg(ap, int), va_arg(ap, int));
	}
	//Face data - numFaces lots of face data, each of which is an int for the number of edges, and int data for edge indices
	for (i=0; i<numFaces;i++) {
		numEdgesThisFace=va_arg(ap,int);
		for (j=0;j<numEdgesThisFace;j++) {
			face.numEdges=numEdgesThisFace;
			face.behind=false;
			thisEdge=va_arg(ap, int);
			face.edges[j]=thisEdge;
		}
		obj.faces[i]=face;
	}

	va_end(ap);

	return obj;
}

//Create an empty world
World3d CreateNewWorld() {
	World3d world;
	world.numObjects=0;
	return world;
}

//Add an object to the world (Would much rather do this dynamically)
int AddObjectToWorld(World3d *world, Object3d object) {
	world->numObjects++;
	world->objects[world->numObjects]=object;
	return world->numObjects;
}

//Constrain angle to 0..2PI
double ClipAngle(double angle) {
	double ret=angle;
	while (ret<0)
		ret += PI * 2.0;
	while (ret>PI * 2.0)
		ret -= PI * 2.0;
	return ret;
}

//Rotate a point in 3d space about a point in the direction aroiund the X axis
void RotatePoint3dXAxis(Point3d *point, Point3d origin, float radAngle) {
	radAngle=ClipAngle(radAngle);
	double y =  origin.y + (double) ((point->y - origin.y) * qcos(radAngle) - (point->z - origin.z) * qsin(radAngle));
	double z = origin.z + (double) ((point->y - origin.y) * qsin(radAngle) + (point->z - origin.z) * qcos(radAngle));
	point->y=y;
	point->z=z;
}

//Rotate a point in 3d space about a point in the direction aroiund the Y axis
void RotatePoint3dYAxis(Point3d *point, Point3d origin, float radAngle) {
	radAngle=ClipAngle(radAngle);
	double x =  origin.x + (double) ((point->x - origin.x) * qcos(radAngle) + (point->z - origin.z) * qsin(radAngle));
	double z = origin.z + (double) ((point->x - origin.x) * -1 * qsin(radAngle) + (point->z - origin.z) * qcos(radAngle));
	point->x=x;
	point->z=z;
}

//Rotate a point in 3d space about a point in the direction aroiund the Z axis
void RotatePoint3dZAxis(Point3d *point, Point3d origin, float radAngle) {
	radAngle=ClipAngle(radAngle);
	double x =  origin.x + (double) ((point->x - origin.x) * qcos(radAngle) - (point->y - origin.y) * qsin(radAngle));
	double y = origin.y + (double) ((point->x - origin.x) * qsin(radAngle) + (point->y - origin.y) * qcos(radAngle));
	point->x=x;
	point->y=y;
}

//Rotate an object in 3d space about the X axis
void RotateObjectXAxis(Object3d *object, float radAngle) {
	int i;
	radAngle=ClipAngle(radAngle);
	object->heading.x = ClipAngle(object->heading.x + radAngle);
	for (i=0;i<object->numPoints;i++)
		RotatePoint3dXAxis(&(object->points[i]), object->centre, radAngle);
}

//Rotate an object in 3d space about the Y axis
void RotateObjectYAxis(Object3d *object, float radAngle) {
	int i;
	radAngle=ClipAngle(radAngle);
	object->heading.y = ClipAngle(object->heading.y + radAngle);
	for (i=0;i<object->numPoints;i++)
		RotatePoint3dYAxis(&(object->points[i]), object->centre, radAngle);
}

//Rotate an object in 3d space about the Z axis
void RotateObjectZAxis(Object3d *object, float radAngle) {
	int i;
	radAngle=ClipAngle(radAngle);
	object->heading.z = ClipAngle(object->heading.z + radAngle);
	for (i=0;i<object->numPoints;i++)
		RotatePoint3dZAxis(&(object->points[i]), object->centre, radAngle);
}

//Move an object in 3d space
void MoveObject(Object3d *object, Point3d shiftVector) {
	int i;
	object->centre.x += shiftVector.x;
	object->centre.y += shiftVector.y;
	object->centre.z += shiftVector.z;

	for (i=0;i<object->numPoints;i++) {
		object->points[i].x += shiftVector.x;
		object->points[i].y += shiftVector.y;
		object->points[i].z += shiftVector.z;
	}
}

//Rescale a point with respect to an origin and scale factor
void ScalePoint3d(Point3d *point, Point3d origin, float scaleFactor) {
	double x = origin.x + ((point->x - origin.x) * scaleFactor);
	double y = origin.y + ((point->y - origin.y) * scaleFactor);
	double z = origin.z + ((point->z - origin.z) * scaleFactor);
	point->x=x;
	point->y=y;
	point->z=z;
}

//Scale an object by a scale factor with respect to an origin
void ScaleObject(Object3d *object, float scaleFactor) {
	int i;
	for (i=0;i<object->numPoints;i++)
		ScalePoint3d(&(object->points[i]), object->centre, scaleFactor);
}

//Transform a point with respect to a camera position and angle
//Calculations from Wikipedia here: http://en.wikipedia.org/wiki/3D_projection
Point3d TransformPoint(Point3d point, Point3d cameraPoint, Point3d cameraDirection) {
	Point3d transformed;
	transformed.x = cosCamera.y * (sinCamera.z * (point.y - cameraPoint.y)
		+ cosCamera.z * (point.x - cameraPoint.x)) - sinCamera.y
		* (point.z - cameraPoint.z);
	transformed.y = sinCamera.x * (cosCamera.y * (point.z - cameraPoint.z)
		+ sinCamera.y * sinCamera.z * (point.y - cameraPoint.y)
		+ cosCamera.z * (point.x - cameraPoint.x)) + cosCamera.x
		* (cosCamera.z * (point.y - cameraPoint.y) - sinCamera.z
		* (point.x - cameraPoint.x));
	transformed.z = cosCamera.x * (cosCamera.y * (point.z - cameraPoint.z)
		+ sinCamera.y * (sinCamera.z * (point.y - cameraPoint.y)
		+ cosCamera.z * (point.x - cameraPoint.x))) - sinCamera.x
		* (cosCamera.z * (point.y - cameraPoint.y) - sinCamera.z
		* (point.x - cameraPoint.x));
	return transformed;
}

//Draw the world from the perspective of a given camera - also called by EraseWorld
void DrawWorldExt(World3d *world, Point3d cameraPoint, Point3d cameraDirection, bool erase) {
	Point3d transformed; //A point transformed to eliminate camera position and rotation
	int i,j, k;	//Loop indices
	int swap;	//used in sorting

	//For performance - precalculate sin and cos of camera directions
	sinCamera.x = qsin(cameraDirection.x);
	sinCamera.y = qsin(cameraDirection.y);
	sinCamera.z = qsin(cameraDirection.z);
	cosCamera.x = qcos(cameraDirection.x);
	cosCamera.y = qcos(cameraDirection.y);
	cosCamera.z = qcos(cameraDirection.z);

	//transform centres first so we can sort
	for (i=1; i<=world->numObjects;i++) {
		sortedObjects[i]=i;
		transformed=TransformPoint(world->objects[i].centre, cameraPoint, cameraDirection);
		transformedCentres[i]=transformed;
	}

	//sort centres so we can determine draw order, back to front so objects
	//appear 'behind' each other correctly
	for (i=world->numObjects; i>1;i--) {
		for (j=1; j<i; j++) {
			//compare z plane values with the 'next' entry to see if we're behind it
			if (transformedCentres[sortedObjects[j]].z < transformedCentres[sortedObjects[j+1]].z) {
				swap=sortedObjects[j];
				sortedObjects[j]=sortedObjects[j+1];
				sortedObjects[j+1]=swap;
			}
		}
	}

	//Now cycle through objects in sorted order and draw
	for (i=1; i<=world->numObjects;i++) { //Loop through objects
		//Transform points
		for (j=0; j<world->objects[sortedObjects[i]].numPoints; j++) { //Loop through points in object
			transformed=TransformPoint(world->objects[sortedObjects[i]].points[j], cameraPoint, cameraDirection);
			transformedObjectPoints[j]=transformed;
			//Project transformed points onto 2D surface a presumed 2 units away
			projected[j].x=transformed.x * (transformed.z == 0 ? 0 : 2.0/transformed.z);
			projected[j].y=transformed.y * (transformed.z == 0 ? 0 : 2.0/transformed.z);
			projected[j].z=(transformed.z>=0 ? true : false);
		}

		//Treat all edges as 'behind' until we know they're on a face that's not
		for (j=0; j < world->objects[sortedObjects[i]].numEdges;j++) {
			world->objects[sortedObjects[i]].edges[j].behind=true;
		}

		//Loop through faces and calculate normals to see if they're 'behind'
		int edge0, edge1, v1, v2, v3, v4;
		Point3d vector0, vector1, normal;
		for (j=1; j < world->objects[sortedObjects[i]].numFaces; j++) {
			//pick the first two edges in the face
			edge0 = world->objects[sortedObjects[i]].faces[j].edges[0];
			edge1 = world->objects[sortedObjects[i]].faces[j].edges[1];
			//they should be defined as A->B then B->C going clockwise, so we can
			//find the right points for our edges
			if (edge0 < 0) { // minus means reverse the order - also, subtract 1 for zero-based array
				v1 = world->objects[sortedObjects[i]].edges[-edge0-1].end;
				v2 = world->objects[sortedObjects[i]].edges[-edge0-1].start;
			} else {
				v1 = world->objects[sortedObjects[i]].edges[edge0-1].start;
				v2 = world->objects[sortedObjects[i]].edges[edge0-1].end;
			}
			if (edge1 < 0) {
				v3 = world->objects[sortedObjects[i]].edges[-edge1-1].start;
				v4 = world->objects[sortedObjects[i]].edges[-edge1-1].end;
			} else {
				v3 = world->objects[sortedObjects[i]].edges[edge1-1].end;
				v4 = world->objects[sortedObjects[i]].edges[edge1-1].start;
			}
			//Now calculate a cross product to get the normal
			vector0 = SubtractPoint(transformedObjectPoints[v1], transformedObjectPoints[v2]);
			vector1 = SubtractPoint(transformedObjectPoints[v3], transformedObjectPoints[v4]);
			normal.x = vector0.y * vector1.z - vector0.z * vector1.y;
			normal.y = vector0.z * vector1.x - vector0.x * vector1.z;
			normal.z = vector0.x * vector1.y - vector0.y * vector1.x;
			//If the normal is pointing away from the camera, treat the face as 'behind' the object
			double dotProduct;
			transformed = transformedObjectPoints[v2];
			dotProduct = normal.x * transformed.x + normal.y * transformed.y + normal.z * transformed.z;
			if (dotProduct <= 0) {
				world->objects[sortedObjects[i]].faces[j].behind=true;
			} else {
				world->objects[sortedObjects[i]].faces[j].behind=false;
				for (k=0; k < world->objects[sortedObjects[i]].faces[j].numEdges; k++) {
					edge0 = world->objects[sortedObjects[i]].faces[j].edges[k];
					if (edge0<0)
						edge1 = -edge0-1;
					else
						edge1 = edge0 -1;
					world->objects[sortedObjects[i]].edges[edge1].behind = false;
				}
			}
		}
		//Draw 'behind' at half-strength colouring.
		Color full, faded;
		full = world->objects[sortedObjects[i]].colour;
		faded.R = full.R / 2;
		faded.G = full.G / 2;
		faded.B = full.B / 2;
		faded.Transparent = 0;
		//Loop through edges & draw corresponding lines twice - behind then in front
		for (k=0; k<=1; k++) {
			for (j=0; j < world->objects[sortedObjects[i]].numEdges;j++) {
				//Are both points in front of us?
				if (projected[world->objects[sortedObjects[i]].edges[j].start].z && projected[world->objects[sortedObjects[i]].edges[j].end].z) {
					//I'm sidestepping the clipping problem of one point in front but another behind us
					if ((k==0 && world->objects[sortedObjects[i]].edges[j].behind) || (k==1 && !(world->objects[sortedObjects[i]].edges[j].behind))) {
						DrawLine(projected[world->objects[sortedObjects[i]].edges[j].start].x * 160.0 + 80.0,
							60.0 - projected[world->objects[sortedObjects[i]].edges[j].start].y * 160.0,
							projected[world->objects[sortedObjects[i]].edges[j].end].x * 160.0 + 80.0,
							60.0 - projected[world->objects[sortedObjects[i]].edges[j].end].y * 160.0,(erase ? CLEAR : (world->objects[sortedObjects[i]].edges[j].behind ? faded : full)));
					}
				}
			}
		}
	}
}

//Draw the world from the perspective of the camera
void DrawWorld(World3d *world, Point3d cameraPoint, Point3d cameraDirection) {
	DrawWorldExt(world, cameraPoint, cameraDirection, false);
}

//Erase the world - Draw it in clear pixels
void EraseWorld(World3d *world, Point3d cameraPoint, Point3d cameraDirection) {
	DrawWorldExt(world, cameraPoint, cameraDirection, true);
}

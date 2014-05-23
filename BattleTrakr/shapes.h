#ifndef __SHAPE_H__
#define __SHAPES_H__

#include "vector.h"
#include "svt.h"

//Some sample object shapes for the vector.h library

//When defining a face - pick edges in sequence rotating clockwise,
//we're going to assume that to calculate a normal from the face

//Define a cube
Object3d CreateCube(Color colour, Point3d position, float scale) {
	Object3d cube;
	cube=CreateObject(colour, CreatePoint(0,0,0), 8, 12, 6,	//8 points, 12 edges, 6 faces
		//8 points (numbered from zero)
		-1.0, 0.0, -1.0,	//0 - back bottom left
		1.0, 0.0, -1.0,		//1 - back bottom right
		-1.0, 2.0, -1.0,	//2 - back top left
		1.0, 2.0, -1.0,		//3 - back top right
		-1.0, 0.0, 1.0,		//4 - front bottom left
		1.0, 0.0, 1.0,		//5 - front bottom right
		-1.0, 2.0, 1.0,		//6 - front top left
		1.0, 2.0, 1.0,		//7 - front top right
		//12 edges (pairs of points, edges numbered from 1 rather than zero
		//so that face data can use sign to indicate orientation)
		0,1, 0,2, 0,4, 1,5,	//1-4
		1,3, 2,6, 2,3, 3,7,	//5-8
		4,6, 4,5, 5,7, 6,7,	//9-12
		//6 faces (first number says how many edges, then there are that many
		//integers referencing edges in the face, a minus sign means read the
		//points in the edge the opposite way round to how they're listed in
		//the edge definitions)
		4, 12, -8, -7, 6,	//top
		4, 7, -5, -1, 2,  	//back
		4, -6, -2, 3, 9,	//left
		4, 10, 11, -12, -9,	//front
		4, 1, 4, -10, -3,	//bottom
		4, 5, 8, -11, -4);	//right
	ScaleObject(&cube, scale);
	MoveObject(&cube, position);
	return cube;
}

//Define a pyramid object
Object3d CreatePyramid(Color colour, Point3d position, float scale) {
	Object3d pyramid;
	pyramid=CreateObject(colour, CreatePoint(0,0,0), 5, 8, 5,
		//5 points
		-1.0,0.0, -1.0,	//0 - base back left
		1.0, 0.0, -1.0,	//1 - base back right
		1.0, 0.0, 1.0,	//2 - base front right
		-1.0, 0.0, 1.0,	//3 - base front left
		0.0, 2.0, 0.0,	//4 - top centre point
		//8 edges
		0,1, 0,3, 0,4, 1,2,
		1,4, 2,3, 2,4, 3,4,
		//5 faces
		4, 1, 4, 6, -2,
		3, 3, -5, -1,
		3, 5, -7, -4,
		3, 7, -8, -6,
		3, 8, -3, 2);
	ScaleObject(&pyramid, scale);
	MoveObject(&pyramid, position);
	return pyramid;
}

//Define a cylinder object
Object3d CreateCylinder(Color colour, Point3d position, float scale) {
	Object3d cylinder;
	cylinder=CreateObject(colour, CreatePoint(0,0,0), 12, 18, 8,
		1.0, 0.0, 0.0,		//12 points
		0.5, 0.0, 0.866,
		-0.5, 0.0, 0.866,
		-1.0, 0.0, 0.0,
		-0.5, 0.0, -0.866,
		0.5, 0.0, -0.866,
		1.0, 2.0, 0.0,
		0.5, 2.0, 0.866,
		-0.5, 2.0, 0.866,
		-1.0, 2.0, 0.0,
		-0.5, 2.0, -0.866,
		0.5, 2.0, -0.866,
		//18 edges
		0,1, 1,2, 2,3, 3,4,
		4,5, 5,0, 0,6, 1,7,
		2,8, 3,9, 4,10, 5,11,
		6,7, 7,8, 8,9, 9,10,
		10,11, 11,6,
		//8 faces
		4, 7, 13, -8, -1,
		4, 8, 14, -9, -2,
		4, 9, 15, -10, -3,
		4, 10, 16, -11, -4,
		4, 11, 17, -12, -5,
		4, 12, 18, -7, -6,
		6, 1, 2, 3, 4, 5, 6,
		6, -18, -17, -16, -15, -14, -13);
	ScaleObject(&cylinder, scale);
	MoveObject(&cylinder, position);
	return cylinder;
}

//define a chessboard object
Object3d CreateChessboard(Color colour, Point3d position, float scale) {
	Object3d board;
	board=CreateObject(colour, CreatePoint(0,0,0), 32, 18, 1,
		//32 points (around the edges)
		-1.0, 0.0, -1.0,
		-1.0, 0.0, 0.75,
		-1.0, 0.0, -0.5,
		-1.0, 0.0, -0.25,
		-1.0, 0.0, 0.0,
		-1.0, 0.0, 0.25,
		-1.0, 0.0, 0.5,
		-1.0, 0.0, 0.75,
		-1.0, 0.0, 1.0,
		1.0, 0.0, -1.0,
		1.0, 0.0, 0.75,
		1.0, 0.0, -0.5,
		1.0, 0.0, -0.25,
		1.0, 0.0, 0.0,
		1.0, 0.0, 0.25,
		1.0, 0.0, 0.5,
		1.0, 0.0, 0.75,
		1.0, 0.0, 1.0,
		-0.75, 0.0, -1.0,
		-0.5, 0.0, -1.0,
		-0.25, 0.0, -1.0,
		0.0, 0.0, -1.0,
		0.25, 0.0, -1.0,
		0.5, 0.0, -1.0,
		0.75, 0.0, -1.0,
		-0.75, 0.0, 1.0,
		-0.5, 0.0, 1.0,
		-0.25, 0.0, 1.0,
		0.0, 0.0, 1.0,
		0.25, 0.0, 1.0,
		0.5, 0.0, 1.0,
		0.75, 0.0, 1.0,
		//18 edges
		0,9, 1,10, 2,11, 3,12,
		4,13, 5,14, 6,15, 7,16,
		8,17, 18,25, 19,26, 20,27,
		21,28, 22,29, 23,30, 24,31,
		0,8, 9,17,
		//only one face - the whole board
		4, 1, 9, 10, 18);
	ScaleObject(&board, scale);
	MoveObject(&board, position);
	return board;
}

/* Create a tank object
	 ___
 ___/   \=====
/		    \
\___________/
*/
Object3d CreateTank(Color colour, Point3d position, float scale) {
	Object3d tank;
	tank=CreateObject(colour, CreatePoint(0.0,0.0,0), 28, 44, 19,
		//28 points
		//base - rests on the y=0 ground plane
		-1.0, 0.0, -1.5,	//0 - base back left
		1.0, 0.0, -1.5,		//1 - base back right
		-1.0, 0.0, 1.5,		//2 - base front left
		1.0, 0.0, 1.5,		//3 - base front right
		//track ridge midpoint
		-1.5, 0.5, -2.0,	//4 - ridge back left
		1.5, 0.5, -2.0,		//5 - ridge back right
		-1.5, 0.5, 2.0,		//6 - ridge front left
		1.5, 0.5, 2.0,		//7 - ridge front right
		//body top
		-1.0, 1.0, -1.5,	//8 - body top back left
		1.0, 1.0, -1.5,		//9 - body top right
		-1.0, 1.0, 1.5,		//10 - body top front left
		1.0, 1.0, 1.5,		//11 - body top front right
		//turret base
		-0.75, 1.0, -1.0,	//12 - turret base back left
		0.75, 1.0, -1.0,	//13 - turret base back right
		-0.75, 1.0, 0.75,	//14 - turret base front left
		0.75, 1.0, 0.75,	//15 - turret base front right
		//turret top
		-0.5, 1.5, -0.5,	//16 - turret top back left
		0.5, 1.5, -0.5,		//17 - turret top back right
		-0.5, 1.5, 0.5,		//18 - turret top front left
		0.5, 1.5, 0.5,		//19 - turret top front right
		//barrel start
		-0.1, 1.2, 0.7,		//20 - barrel start bottom left
		0.1, 1.2, 0.7,		//21 - barrel start bottom right
		-0.1, 1.4, 0.6,		//22 - barrel start top left
		0.1, 1.4, 0.6,		//23 - barrel start top right
		//barrel end
		-0.1, 1.2, 2.2,		//24 - barrel end bottom left
		0.1, 1.2, 2.2,		//25 - barrel end bottom right
		-0.1, 1.4, 2.2,		//26 - barrel end top left
		0.1, 1.4, 2.2,		//27 - barrel end top right
		//44 lines
		0, 1, 1, 3, 3, 2, 2, 0,			//1-4: base
		0, 4, 1, 5, 2, 6, 3, 7,			//5-8: base to track ridge midpoint
		4, 5, 5, 7, 7, 6, 6, 4,			//9-12: track ridge midpoint
		4, 8, 5, 9, 6, 10, 7, 11,		//13-16: ridge to body top
		8, 9, 9, 11, 11, 10, 10, 8,		//17-20: body top
		12, 13, 13, 15, 15, 14, 14, 12,	//21-24: turret base
		12, 16, 13, 17, 14, 18, 15, 19,	//25-28: turret base to top
		16, 17, 17, 19, 19, 18, 18, 16,	//29-32: turret top
		20, 21, 21, 23, 23, 22, 22, 20,	//33-36: barrel start
		20, 24, 21, 25, 22, 26, 23, 27,	//37-40: barrel
		24, 25, 25, 27, 27, 26, 26, 24,	//41-44: barrel end
		//19 faces
		4, 3, 4, 1, 2,			//base
		4, 17, 18, 19, 20,	    //top
		4, 9, -6, -1, 5,		//lower back
		4, 17, -14, -9, 13,	    //upper back
		4, 29, -26, -21, 25,	//turret back
		4, 11, -7, -3, 8,		//lower front
		4, 16, 19, -15, -11,    //upper front
		4, 31, -27, -22, 28,	//turret front
		4, 10, -8, -2, 6,		//lower right
		4, 18, -16, -10, 14,	//upper right
		4, 30, -28, -22, 26,	//turret right
		4, 12, -5, -4, 7,		//lower left
		4, 20, -13, -12, 15,	//upper left
		4, 32, -25, -20, 27,	//turret left
		4, -43, -40, 35, -39,	//barrel top
		4, 33, 38, -41, -37,	//barrel bottom
		4, 40, -42, -38, 34,	//barrel left
		4, -39, 36, 37, -44,	//barrel right
		4, 41, 42, 43, 44);	//barrel end
	ScaleObject(&tank, scale);
	MoveObject(&tank, position);
	return tank;
}

#endif

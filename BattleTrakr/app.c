/*
BattleTrakr v1.0 by RorschachUK April 2011
A vector graphics proof of concept demo.

Inspired by a classic Atari coin-op arcade game from 1980

Your Trakr is being hunted - an enemy tank is stalking you across the
battlefield!  Your only hope is to find it and shoot it before it shoots
you.

Use the sticks to move and the Go button to fire.  Keep your eye on the
radar to help you find the enemy.  Beware though, the battlefield is strewn
with obstacles you can't drive or shoot through.
*/
#include "svt.h"
#include "draw.h"
#include "vector.h"
#include "shapes.h"
#include <stdlib.h>

#define SWEEPSTEPS 20
#define ROTATESLEEP 3000
#define FORWARDSLEEP 600
#define FILE_PARAMS "A:\\Bigtrakr_Sounds\\Bigtrakr_Params.tmp"
#define SOUND_BEEP "A:\\Battletrakr_Sounds\\Beep.wav"
#define SOUND_EXPLOSION "A:\\Battletrakr_Sounds\\Explosion.wav"
#define SOUND_HIT "A:\\Battletrakr_Sounds\\Hit.wav"
#define SOUND_SHOOT "A:\\Battletrakr_Sounds\\Shoot.wav"

//bitmaps
extern char _binary_Images_battle_bmp_end[];
extern char _binary_Images_battle_bmp_start[];

extern char _binary_Images_rorschach_bmp_end[];
extern char _binary_Images_rorschach_bmp_start[];

extern char _binary_Images_tank_bmp_end[];
extern char _binary_Images_tank_bmp_start[];

int score;			//Points!
int sweepStepCount;	//Position of radar sweep arm in the HUD
iPoint arrow[4];	//Polygon of an arrowhead directional pointer in the HUD
Point3d baselinePos;//Baseline reference position for camera when keystate changed
float arrowRefAngle;//Baseline reference angle for camera when keystate changed
float sweepAngle;	//Direction of radar sweep arm
iPoint sweepArm;	//Point at tip of radar sweep arm
int arrowX, arrowY;	//Centre of arrow
int radarX, radarY;	//Centre of radar sweep arm
int rotateSleep;	//time to rotate
int keystate;		//store keystate for interpretation
int oldKeystate;	//to determine if keystate's changed
bool blocked;		//false=not blocked, true=blocked
int mode;			//control mode (0=splash, 1=game, 2=instructions)
int lives;			//game lives
int logoImage;		//handle for BattleTrakr logo image
int signatureImage;	//Rorschach logo
int tankImage;		//tank icon for lives
int tankObjectIndex;//tank index position in world
int refreshCount;	//Countdown number of sleeps till we do a full redraw
int resetCount;		//Countdown till we reset the model to prevent cumulative errors
int behaviourCount;	//Countdown ticks how long the tank maintains its present behaviour
int behaviourType;	//0=move forward, 1=move backward, 2=rotate left, 3=rotate right
World3d world;		//The virtual world
Object3d obj;		//placeholder for an object
Point3d cameraPos;	//Position of camera in the world
Point3d cameraAngle;//Angle of camera in the world
Object3d tank;		//Reference tank

//Resets polygon for arrow to avoid cumulative rounding errors
void ResetArrow() {
	//define a triangle polygon
	arrow[0].x=80;
	arrow[0].y=20;
	arrow[1].x=50;
	arrow[1].y=90;
	arrow[2].x=80;
	arrow[2].y=70;
	arrow[3].x=110;
	arrow[3].y=90;
	//I could define it small but I
	//wanted to test these routines!
	ScalePoly(4, arrow, 80, 60, 0.3);
	MovePoly(4, arrow, -60, -40);
}

//Draws arrow at new angle
void DrawArrow(float radAngle) {
	//Erase old copy
	DrawPoly(4, arrow, BLACK);
	//Recalculate new rotated angle
	ResetArrow();
	RotatePoly(4, arrow, arrowX, arrowY, radAngle);
	//Draw new arrow
	DrawPoly(4, arrow, YELLOW);
}

//Draws radar arm at new angle
void DrawRadarArm(float radAngle) {
	//Erase old line
	DrawLine(radarX, radarY, sweepArm.x, sweepArm.y, BLACK);
	//define 'north' tip position of sweep arm
	sweepArm.x=radarX;
	sweepArm.y=radarY-16;
	//Rotate point about centre
	RotatePoint(&sweepArm, radarX, radarY, radAngle);
	//Draw new arm
	DrawLine(radarX,radarY, sweepArm.x, sweepArm.y, GREEN);
}

//Draw radar dots on HUD
void DrawRadarDots(bool clear) {
	int i;
	int x, y;
	Point3d transformed;
	SetLineWidth(3); //only the tank is largest
	for(i=1; i<=world.numObjects; i++) {
		transformed=TransformPoint(world.objects[i].centre, cameraPos, cameraAngle);
		if (i==2) //rest of objects a little smaller
			SetLineWidth(2);
		//draw transformed point on radar
		x = (int)(transformed.x / 200.0 * 20.0);
		y = (int)(transformed.z / 200.0 * 20.0);
		if (qsqrt(x * x + y * y) < 20) { //if within circle
			Plot(x + radarX, radarY - y, (clear ? BLACK : world.objects[i].colour));
		}
	}
	SetLineWidth(1);
}

double Random(double min, double max) {
	return (double) rand() / RAND_MAX * (max-min) + min;
}

//Draw an explosion in the centre where your shell hit
void DrawExplosion() {
	iPoint boom[16];
	int i;
	double angle, distance;
	//Define a poly star with some random peaks
	for (i=0; i<16; i++) {
		if (i % 2 == 1)
			distance = 8;
		else
			//Random star points to stop all explosions being identical
			distance = Random(12.0, 24.0);
		angle = PI * 2 * i / 16.0;
		boom[i].x = 80.0 - distance * qsin(angle);
		boom[i].y = 60 + distance * qcos(angle);
	}
	DrawPoly(16, boom, YELLOW);
}

//Helper function for recursing procedural cracked screen lines
void DrawCrackedScreenExt(int x0, int y0, int x1, int y1, int remaining) {
	//basically, draw this line, then if remaining>0, draw two more lines branching off from it
	Color col = RGBColor(255 * (remaining + 6) / 8,255 * (remaining + 6) / 8,255 * (remaining + 6) / 8,0);
	DrawLine(x0, y0, x1, y1, col);
	if (remaining>0) {
		int i, x2, y2;
		float angle, distance;
		//two lines
		for (i=0; i<2; i++) {
			if (x0==x1) {
				if (y0<y1) {
					angle = PI;
				} else {
					angle= - PI;
				}
			} else {
				//get angle via arctan
				angle = qatan((float)(y0-y1)/(float)(x1-x0));
				//add or subtract up to a quarter turn depending on first or second line
				angle += Random(0, PI/2.0) * (i==0?1.0:-1.0);
				if (x0 > x1) {
					//correct for quadrant
					angle += PI;
				}
			}
			//pick a distance
			distance = Random(10,40);
			//get new positions
			x2 = x1 + distance * qcos(angle);
			y2 = y1 - distance * qsin(angle);
			//recurse into new set of lines
			DrawCrackedScreenExt(x1, y1, x2, y2, remaining-1);
		}
	}
}

//Draw a cracked screen when you've been hit
void DrawCrackedScreen() {
	int i, x0, y0, x1, y1;
	float angle, distance;
	//Pick a centre reasonably close to screen centre
	x0 = Random(50, 110);
	y0 = Random(40, 80);
	//Cycle through 5 angles approximately spread out
	for (i=0; i<5; i++) {
		//Pick an angle and a distance and recurse some more lines out from there
		angle = i * 2.0 * PI / 5 + Random(-0.5, 0.5);
		distance = Random(10, 50);
		x1 = x0 + distance * qcos(angle);
		y1 = y0 - distance * qsin(angle);
		//begin recursion, each of these five lines will fork a couple more times
		DrawCrackedScreenExt(x0, y0, x1, y1, 2);
	}
}

//Borrowed from BigTrakr - get calibration data to improve rotation reckoning
void LoadCalibration() {
	File f = OpenFile(FILE_PARAMS);
	int buffer;
	long read;
	if (f != -1) {
		read = ReadFile(f, &buffer, 2);
		CloseFile(f);
	}
	if (read==2) {
		rotateSleep=buffer*24; //BigTrakr stores time to rotate 15 degrees not 360
	} else {
		rotateSleep=ROTATESLEEP;
	}
}

//test for line-of-sight - face intersection is hard, so cheat slightly
//and look along the y=0 base plane for a line intersection.
//Returns index of first object hit.
int LineHitsObject(Point3d gunPoint, Point3d gunAngle) {
	int i, j, ret;
	float nearest, distance;
	Point3d start, end;
	ret=0;
	nearest=0.0;
	//Cycle through objects
	for (i=1; i<=world.numObjects; i++) {
		//cycle through edges
		for (j=0; j < world.objects[i].numEdges;j++) {
			//transform with respect to the gun position and orientation
			start=TransformPoint(world.objects[i].points[world.objects[i].edges[j].start], gunPoint, gunAngle);
			end=TransformPoint(world.objects[i].points[world.objects[i].edges[j].end], gunPoint, gunAngle);
			//ignore height - if the edge's start and end points are left and right of the gun, and in front, it's a hit
			if (((start.x <=0 && end.x >= 0) || (start.x >=0 && end.x <= 0)) && start.z >0 && end.z > 0) {
				//Hit!  Is it nearer than the last one?
				distance = start.z;
				if (start.x != end.x)
					distance += start.x * (end.z - start.z) / (end.x - start.x);
				if (ret==0 || distance < nearest) {
					ret=i;
					nearest = distance;
					break; //jump out of for loop (for j)
				}
			}
		}
	}
	return ret; //will be 0 if none found - our objects start at 1 anyway
}

//Backdrop headup display graphics - instrument panels & crosshairs
void DrawHUD() {
	//Black out side panels
	SetLineWidth(5);
	int i, j;
	for (i=0; i<8; i++)
		for (j=0; j<8; j++) {
			Plot(i*5,j*5,BLACK);
			Plot(120+i*5,j*5,BLACK);
		}
	SetLineWidth(1);

	//Some concentric circles
	DrawCircle(arrowX, arrowY, 19, RED);
	DrawCircle(radarX, radarY, 19, GREEN);

	//Crosshairs
	Color crosshair;
	if (LineHitsObject(cameraPos, cameraAngle) == tankObjectIndex) {
		crosshair = RED;
	} else {
		crosshair = WHITE;
	}
	DrawLine(80, 40, 80, 80, crosshair);
	DrawLine(60, 60, 100, 60, crosshair);
	DrawCircle(80, 60, 10, crosshair);

	//lives
	for (i = 0; i< lives; i++)
		DrawImage(tankImage, 45 + i * 25, 6, WHITE);

	//score
	SetTextColor(YELLOW);
	DrawText(45, 15, "%d", score);
}

void PlaySound(char *filename) {
	//CloseGraphics();// free up our processor to do audio digitizing
	StartAudioPlayback(filename); // begin playing file
	// wait for audio to finish
	while( IsAudioPlaying() )
	{
		Sleep( 50 );
	}
	//OpenGraphics();// open the display again
}

//Help screen
void ShowHelp() {
	CloseGraphics();
	OpenGraphics();

	//DrawImage(logoImage, 5,5,RGBColor(253,255,252,0));
	SetTextColor(WHITE);
	DrawText(6, 7, "Find the enemy &");
	DrawText(6, 22, "shoot him before");
	DrawText(6, 37, "he shoots you!");
	SetTextColor(GREEN);
	DrawText(6, 52, "Press GO to fire,");
	SetTextColor(RED);
	DrawText(6, 67, "beware of the red");
	DrawText(6, 82, "exploding barrels!");
	SetTextColor(CYAN);
	DrawText(120,100, "OK");
	Show();
	mode=2;
}

//Splash screen
void Splash() {
	ResetTimer(); //we're going to use it for random seed
	//Setup a world with a tank and a pyramid in specific places
	CloseGraphics();
	OpenGraphics();

	Point3d centre;
	world=CreateNewWorld();

	//A tank pointing straight at us
	centre=CreatePoint(0.0,0.0,45.0);
	obj=CreateTank(GREEN, centre, 4.5);
	RotateObjectYAxis(&obj, 2.0 * PI / 360.0 * 195);
	AddObjectToWorld(&world, obj);

	//A yellow pyramid behind the tank and to the right
	centre=CreatePoint(10.0, 0.0, 70.0);
	obj=CreatePyramid(YELLOW, centre, 5.0);
	RotateObjectYAxis(&obj, 3.0*PI/5.0);
	AddObjectToWorld(&world, obj);

	//A blue cube behind the tank and to the left
	centre=CreatePoint(-10.0, 0.0, 60.0);
	obj=CreateCube(BLUE, centre, 5.0);
	RotateObjectYAxis(&obj, 195.0*PI/180.0);
	AddObjectToWorld(&world,obj);

	//Draw world, add splash graphics, prompt to start
	cameraPos=CreatePoint(0.0,5.0,0.0);
	cameraAngle=CreatePoint(0.0,0.0,0.0);
	DrawWorld(&world, cameraPos, cameraAngle);
	SetTextColor(GREEN);
	DrawText(5,25, "by RorschachUK");

	SetTextColor(CYAN);
	DrawText(5,100, "Help");
	DrawText(110,100,"Start");
	DrawImage(logoImage, 5,5,RGBColor(253,255,252,0));
	DrawImage(signatureImage, 135,24,BLACK);

	Show();
	Sleep(100);
	mode=0;
}

//Place the tank somewhere suitable - not too easy, not too close, not too far away, not straight ahead
void PlaceTank(Point3d cameraPosition, Point3d cameraAngle) {
	world.objects[tankObjectIndex]=tank;
	RotateObjectYAxis(&(world.objects[tankObjectIndex]), Random(-PI/2.0, PI/2.0));
	double distance = Random(50.0, 100.0);	//At least 50 from us, at most 100
	double angle = Random(45.0 / 360.0 * 2 * PI, 315.0 / 360.0 * 2.0 * PI);	//45-315 degrees
	Point3d tankPos = CreatePoint(cameraPosition.x + distance * qsin(angle + cameraAngle.y), 0, cameraPosition.z + distance * qcos(angle + cameraAngle.y));
	MoveObject(&(world.objects[tankObjectIndex]), tankPos);
	if (world.objects[tankObjectIndex].centre.z<0) {
		//behind us - face away
		RotateObjectYAxis(&(world.objects[tankObjectIndex]),PI);
	}
}

//Setup world
void InitialiseWorld() {
	world=CreateNewWorld();
	Point3d centre;
	//Camera starting point - raised 5cm off ground.
	cameraPos=CreatePoint(0.0,5.0,0.0);
	cameraAngle=CreatePoint(0.0,0.0,0.0);
	baselinePos=cameraPos;
	arrowRefAngle=0;

	//The tank
	tank=CreateTank(GREEN, CreatePoint(0,0,0), 4.5);
	tankObjectIndex=AddObjectToWorld(&world, tank);
	PlaceTank(cameraPos, cameraAngle);

	//Populate with a few shapes
	int i;
	for (i=0; i<4; i++) {
		//Random blue cubes
		centre=CreatePoint(Random(-100.0, 100.0),0.0,Random(-100.0, 100.0));
		obj=CreateCube(BLUE, centre, 4.0);
		RotateObjectYAxis(&obj, Random(0.0, 2 * PI));
		AddObjectToWorld(&world,obj);

		//Random yellow pyramids
		centre=CreatePoint(Random(-100.0, 100.0),0.0,Random(-100.0, 100.0));
		obj=CreatePyramid(YELLOW, centre, 4.0);
		RotateObjectYAxis(&obj, Random(0.0, 2 * PI));
		AddObjectToWorld(&world,obj);

		//Random red barrels
		centre=CreatePoint(Random(-100.0, 100.0),0.0,Random(-100.0, 100.0));
		obj=CreateCylinder(RED,centre, 4.0);
		RotateObjectYAxis(&obj, Random(0.0, 2 * PI));
		AddObjectToWorld(&world,obj);
	}
}

//start the game
void InitialiseGame() {
	srand(ReadTimer());
	mode=1;
	score=0;
	InitialiseWorld();
	refreshCount=0;
	resetCount=50;
	behaviourCount=0;
	ResetArrow();
	arrowRefAngle=0;
	cameraPos=CreatePoint(0.0,5.0,0.0);
	cameraAngle=CreatePoint(0.0,0.0,0.0);
	blocked=0;
	lives=3;

	DrawHUD();
}

//Pick a random behaviour for the tank
void ChooseBehaviour() {
	behaviourType = (rand() % 4);
	behaviourCount = (rand() % 10) + 5;
}

void LoseLife(char *reason) {
	//Draw cracked screen - but draw all the rest of the world & hud first
	CloseGraphics();
	OpenGraphics();
	DrawWorld(&world, cameraPos, cameraAngle);
	DrawHUD();
	DrawArrow(cameraAngle.y);
	DrawRadarDots(true);
	DrawRadarArm(sweepAngle * sweepStepCount++);
	DrawRadarDots(false);
	//finally!
	DrawCrackedScreen();
	SetTextColor(RED);
	DrawText(25,40,reason);
	if (--lives<=0) {
		SetTextColor(GREEN);
		DrawText(30,55,"GAME OVER!");
	}
	Show();
	PlaySound(SOUND_EXPLOSION);
	//dead?
	if (lives<=0) {
		Sleep(2000);
		//Reset back to splash screen
		ResetTimer();
		mode=0;

		Splash();
	} else {
		// Hit but still going
		InitialiseWorld();
	}
}

//Handle tank movement
void MoveTank() {
	Point3d playerOffset = SubtractPoint(cameraPos, world.objects[tankObjectIndex].centre);
	double angleToPlayer;
	if (playerOffset.x == 0.0) {
		angleToPlayer = ((playerOffset.z < 0.0) ? PI / 2.0 : 0.0);
	} else {
		angleToPlayer = qatan((double)playerOffset.x / (double)playerOffset.z);
		if (playerOffset.x < 0.0)
			angleToPlayer += PI;
	}
	angleToPlayer = ClipAngle(angleToPlayer);

	double distanceToPlayer = qsqrt(playerOffset.x * playerOffset.x + playerOffset.z * playerOffset.z);
	double angleOffset = ClipAngle(world.objects[tankObjectIndex].heading.y - angleToPlayer);

/*
	DrawText(5,35,"X:%d, Z:%d",(int)(playerOffset.x),(int)(playerOffset.z));
	DrawText(5,50,"AO:%d, AP:%d",(int)(180.0/PI*angleOffset),(int)(180.0/PI*angleToPlayer));
	DrawText(5,65,"DI:%d, Atan:%d",(int)(distanceToPlayer),(int)(1000.0 * qatan((double)playerOffset.x / (double)playerOffset.z)));
*/
	switch(behaviourType) {
		case 0: { //move forward
			MoveObject(&(world.objects[tankObjectIndex]), CreatePoint(2.0 * qsin(world.objects[tankObjectIndex].heading.y), 0.0, 2.0 * qcos(world.objects[tankObjectIndex].heading.y)));
		}
		break;
		case 1: { //rotate right
			RotateObjectYAxis(&(world.objects[tankObjectIndex]), 5.0 * PI * 2.0 /360.0);
		}
		break;
		case 2: { //rotate left
			RotateObjectYAxis(&(world.objects[tankObjectIndex]), -5.0 * PI * 2.0 /360.0);
		}
		break;
		case 3: { //hunt!
			if (angleOffset > PI/4.0 && angleOffset < 7.0 * PI / 4.0) {
				//if not pointing roughly at player, turn towards player
				RotateObjectYAxis(&(world.objects[tankObjectIndex]), PI / 180.0 * (angleOffset < PI ? -5.0 : 5.0));
			} else if (distanceToPlayer > 75.0) {
				//if pointing roughly at player but far away, move forward
				MoveObject(&(world.objects[tankObjectIndex]), CreatePoint(2.0 * qsin(world.objects[tankObjectIndex].heading.y), 0.0, 2.0 * qcos(world.objects[tankObjectIndex].heading.y)));
			} else if (angleOffset < PI / 180.0 * 5.0 || angleOffset > PI / 180.0 * 355.0) {
				//turn more finely
				RotateObjectYAxis(&(world.objects[tankObjectIndex]), PI / 180.0 * (angleOffset < PI ? -1.0 : 1.0));
			} else {
				//if pointing roughly at player and close, turn towards player
				RotateObjectYAxis(&(world.objects[tankObjectIndex]), PI / 180.0 * (angleOffset < PI ? -5.0 : 5.0));
			}
		}
	}
	playerOffset = SubtractPoint(cameraPos, world.objects[tankObjectIndex].centre);
	if (playerOffset.x == 0.0) {
		angleToPlayer = ((playerOffset.z < 0.0) ? PI / 2.0 : 0.0);
	} else {
		angleToPlayer = qatan((double)playerOffset.x / (double)playerOffset.z);
		if (playerOffset.x < 0.0)
			angleToPlayer += PI;
	}
	angleToPlayer = ClipAngle(angleToPlayer);

	distanceToPlayer = qsqrt(playerOffset.x * playerOffset.x + playerOffset.z * playerOffset.z);
	angleOffset = ClipAngle(world.objects[tankObjectIndex].heading.y - angleToPlayer);
	if (angleOffset < PI / 180.0 * 5.0 || angleOffset > PI / 180.0 * 355.0) {
		//if pointing at player, fire
		PlaySound(SOUND_SHOOT);
		//Hit if close enough angle
		if (angleOffset < PI / 180.0 * 2.0 || angleOffset > PI / 180.0 * 358.0) {
			LoseLife("You got shot!");
		} else {
			//Missed - pick a new behaviour
			ChooseBehaviour();
		}
	}
}

//Calculate new angle and position given starting angle and position when timer started, elapsed time and keystate when last changed.
void MoveCamera(Point3d *cameraPos, Point3d *cameraAngle, Point3d baselinePos, float baselineAngle, unsigned int elapsed, int keys) {
	float distance;
	if(blocked)
		SetMotors(0,0);

	switch (keys & (KEY_RIGHT_BACK+KEY_RIGHT_FORWARD+KEY_LEFT_BACK+KEY_LEFT_FORWARD)) {
		case KEY_RIGHT_BACK+KEY_LEFT_FORWARD: { //rotate right
			if (blocked) {
				blocked=false;
				CloseMotors();
			}
			cameraAngle->y=baselineAngle + (PI * 2.0 * elapsed) / rotateSleep;
		}
		break;
		case KEY_RIGHT_FORWARD+KEY_LEFT_BACK: { //rotate left
			if (blocked) {
				blocked=false;
				CloseMotors();
			}
			cameraAngle->y=baselineAngle - (PI * 2.0 * elapsed) / rotateSleep;
		}
		break;
		case KEY_RIGHT_FORWARD: { //one wheel rotate left
			if (blocked) {
				blocked=false;
				CloseMotors();
			}
			cameraAngle->y=baselineAngle - (PI * elapsed) / rotateSleep;
		}
		break;
		case KEY_LEFT_FORWARD: { //one wheel rotate right
			if (blocked) {
				blocked=false;
				CloseMotors();
			}
			cameraAngle->y=baselineAngle + (PI * elapsed) / rotateSleep;
		}
		break;
		case KEY_RIGHT_BACK: { //one wheel rotate left
			if (blocked) {
				blocked=false;
				CloseMotors();
			}
			cameraAngle->y=baselineAngle + (PI * elapsed) / rotateSleep;
		}
		break;
		case KEY_LEFT_BACK: { //one wheel rotate left
			if (blocked) {
				blocked=false;
				CloseMotors();
			}
			cameraAngle->y=baselineAngle - (PI * elapsed) / rotateSleep;
		}
		break;
		case KEY_RIGHT_FORWARD+KEY_LEFT_FORWARD: { //forward
			//First, check if anything is in our way
			int ob = LineHitsObject(*cameraPos, *cameraAngle);
			if (ob != 0) {
				//something's in the way, calculate the distance to it
				Point3d offset = SubtractPoint(world.objects[ob].centre, *cameraPos);
				distance = qsqrt(offset.x * offset.x + offset.z * offset.z);
				//if we're too close, stop engines and refuse to proceed
				if (distance < 30 && !blocked) {
					if (world.objects[ob].colour.R==255 && world.objects[ob].colour.G==0 && world.objects[ob].colour.B==0) {
						//it's a barrel - red barrels always explode in FPSs!
						LoseLife("Beware barrels!");
					} else {
						OpenMotors();
						SetMotors(0,0);
						PlaySound(SOUND_HIT);
						blocked=true;
					}
					break;
				}
			}
			if (blocked) {
				blocked=false;
				CloseMotors();
			}
			distance=(20.0 * elapsed)/FORWARDSLEEP;
			cameraPos->x=baselinePos.x + (distance * qsin(baselineAngle));
			cameraPos->z=baselinePos.z + (distance * qcos(baselineAngle));
		};
		break;
		case KEY_RIGHT_BACK+KEY_LEFT_BACK: { //backwards
			//First, check if anything is in our way
			Point3d dir=*cameraAngle;
			dir.y=ClipAngle(dir.y+PI);
			int ob = LineHitsObject(*cameraPos, dir);
			if (ob != 0) {
				//something's in the way, calculate the distance to it
				Point3d offset = SubtractPoint(world.objects[ob].centre, *cameraPos);
				distance = qsqrt(offset.x * offset.x + offset.z * offset.z);
				//if we're too close, stop engines and refuse to proceed
				if (distance < 30 && !blocked) {
					if (world.objects[ob].colour.R==255 && world.objects[ob].colour.G==0 && world.objects[ob].colour.B==0) {
						//it's a barrel - red barrels always explode in FPSs!
						LoseLife("Beware barrel!s");
					} else {
						OpenMotors();
						SetMotors(0,0);
						PlaySound(SOUND_HIT);
						blocked=true;
					}
					break;
				}
			}
			if (blocked) {
				blocked=false;
				CloseMotors();
			}
			distance=(20.0 * elapsed)/FORWARDSLEEP;
			cameraPos->x=baselinePos.x - (distance * qsin(baselineAngle));
			cameraPos->z=baselinePos.z - (distance * qcos(baselineAngle));
		};
	}
	//normalise ret to 0..2PI
	cameraAngle->y = ClipAngle(cameraAngle->y);
}

//Setup
void Start() {
	OpenFileSystem();
	OpenGraphics();
	SetLineWidth(1); //needed otherwise we draw huge thick points!

	//Register bitmaps
	OpenImageRegister();
	logoImage = RegisterImage(_binary_Images_battle_bmp_start,_binary_Images_battle_bmp_end - _binary_Images_battle_bmp_start);
	signatureImage = RegisterImage(_binary_Images_rorschach_bmp_start,_binary_Images_rorschach_bmp_end - _binary_Images_rorschach_bmp_start);
	tankImage = RegisterImage(_binary_Images_tank_bmp_start,_binary_Images_tank_bmp_end - _binary_Images_tank_bmp_start);
	CloseImageRegister();

	sweepStepCount=0;

	ResetTimer();
	arrowRefAngle=0;
	sweepAngle = 2 * PI / SWEEPSTEPS;

	arrowX=20;
	arrowY=20;
	radarX=140;
	radarY=20;

	sweepArm.x=140;
	sweepArm.y=20;
	oldKeystate=0;

	refreshCount=0;

  	//Load calibration data, if any exists
  	rotateSleep = ROTATESLEEP;
  	LoadCalibration();

	Splash();
}

//Main program loop - Run gets called until it returns false
bool Run() {
  	//Main loop - get keypresses and interpret them
  	keystate=GetRemoteKeys();
	if(keystate & KEY_HOME) {
		return false;
	}
	switch(mode) {
		case 0: {	//Splash screen
			if (keystate & KEY_INPUT2) { //Button B
				//Start game
				InitialiseGame();
				PlaySound(SOUND_BEEP);
			}

			if (keystate & KEY_INPUT1) { //Button A
				//show help
				ShowHelp();
				PlaySound(SOUND_BEEP);
			}
		}
		break;
		case 2: {	//Help - wait for 'OK' press
			if (keystate & KEY_INPUT2) { //Button B
				//Reset back to splash screen
				ResetTimer();
				mode=0;

				Splash();
				PlaySound(SOUND_BEEP);
			}
		}
		break;
		case 1: {	//Game
			if (keystate & KEY_INPUT1) { //Button A
				//Reset back to splash screen
				ResetTimer();
				mode=0;

				Splash();
			}
			if (keystate & KEY_RUN) { //Go button
				//Fire!
				PlaySound(SOUND_SHOOT);
				refreshCount = 0;
				int hit;
				hit = LineHitsObject(cameraPos, cameraAngle);
				if (hit == tankObjectIndex) {
					//we hit the tank!
					score += 100;
					CloseGraphics();
					OpenGraphics();
					DrawWorld(&world, cameraPos, cameraAngle);
					DrawHUD();
					DrawExplosion();
					DrawArrow(cameraAngle.y);
					DrawRadarDots(true);
					DrawRadarArm(sweepAngle * sweepStepCount++);
					DrawRadarDots(false);

					SetTextColor(CYAN);
					DrawText(6,100,"Quit");
					Show();
					PlaySound(SOUND_EXPLOSION);
					PlaceTank(cameraPos, cameraAngle);
				}
			}

			//Have the movement sticks changed? This bit doesn't care about buttons
			if((keystate & (KEY_RIGHT_BACK+KEY_RIGHT_FORWARD+KEY_LEFT_BACK+KEY_LEFT_FORWARD)) != (oldKeystate & (KEY_RIGHT_BACK+KEY_RIGHT_FORWARD+KEY_LEFT_BACK+KEY_LEFT_FORWARD))) {

				MoveCamera(&cameraPos, &cameraAngle, baselinePos, arrowRefAngle, ReadTimer(), oldKeystate);
				arrowRefAngle=cameraAngle.y;
				baselinePos=cameraPos;

				ResetTimer();
				oldKeystate = keystate;
				if (keystate==0) //we've just stopped moving
					refreshCount=0;
			}

			if (mode==1) {
				if (sweepStepCount == SWEEPSTEPS)
					sweepStepCount=0;

				if (--behaviourCount<0)
					ChooseBehaviour();

				MoveTank();

				MoveCamera(&cameraPos, &cameraAngle, baselinePos, arrowRefAngle, ReadTimer(), oldKeystate);
				//is it time to reset the tank model?  Otherwise it gradually gets distorted
				if (--resetCount<0) {
					//Refresh the tank model which is prone to getting distorted
					//due to cumulative inaccuracies of sin/cos approximations
					Point3d angle=world.objects[tankObjectIndex].heading;
					Point3d tankPos=world.objects[tankObjectIndex].centre;
					world.objects[tankObjectIndex]=tank;
					MoveObject(&(world.objects[tankObjectIndex]), tankPos);
					RotateObjectYAxis(&(world.objects[tankObjectIndex]), angle.y);
					resetCount=50;
				}

				//Is it time to redraw the world?
				if (--refreshCount<0) {
					//Seems a bit brutal to have to close graphics but it's
					//the only way to clear the screen that'll let us draw
					//on it properly again - ClearScreen or ClearRectangle mess up
					CloseGraphics();
					OpenGraphics();

					DrawWorld(&world, cameraPos, cameraAngle);

					DrawHUD();
					refreshCount=2;
				}
				DrawArrow(cameraAngle.y);
				DrawRadarDots(true);
				DrawRadarArm(sweepAngle * sweepStepCount++);
				DrawRadarDots(false);

				SetTextColor(CYAN);
				DrawText(6,100,"Quit");
				Show();

				//just in case we changed mode...
				if (mode==0)
					Splash();
			}
		}
	}
	Sleep(50);
	return true;
}

//Cleanup
void End() {
	CloseGraphics();
	CloseFileSystem();
}

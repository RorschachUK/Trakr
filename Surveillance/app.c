//Surveillance by RorschachUK, September 2011

#include "svt.h"
#include "camera.h"

//File paths on SD
#define BMP_EVIDENCE "A:\\Surveillance\\Evidence.bmp"
#define WAV_ALARM "A:\\Surveillance\\Alarm.wav"
#define WAV_EVIDENCE "A:\\Surveillance\\Evidence.wav"

//bitmaps
extern char _binary_Images_camw_bmp_start[];
extern char _binary_Images_camw_bmp_end[];

extern char _binary_Images_camb_bmp_start[];
extern char _binary_Images_camb_bmp_end[];

extern char _binary_Images_rorschach_bmp_start[];
extern char _binary_Images_rorschach_bmp_end[];

//Variables
int imgCamW, imgCamB, imgRorschach; //handles for bitmaps
Color red, blue, green, white, black, grey, yellow, cyan, magenta; //some palette colours
int keyState, oldKeyState;	//Used for tracking whether key presses have changed
bool pushButton, oldPushButton;	//button on the front of the Trakr
Color referenceBuffer[(320 * 240 / (BLOCKSIZE_X * BLOCKSIZE_Y))];	//Baseline to compare against
Color compareBuffer[(320 * 240 / (BLOCKSIZE_X * BLOCKSIZE_Y))];		//quantized buffer to compare with reference
int threshold, maxdifferences; //used in determining degree of difference between images
int compare, oldCompare;
int mode; //0=Move, 1=Guard, 2=Record, 3=Splash

//RGBColor - returns a colour struct
Color RGBColor(unsigned char R, unsigned char G, unsigned char B) {
	Color ret;
	ret.R=R;
	ret.G=G;
	ret.B=B;
	ret.Transparent=0;
	return ret;
}

//Play a recorded audio sound
void PlaySound(char *filename) {
	StartAudioPlayback(filename); // begin playing file
	while(IsAudioPlaying()) {
		Sleep(50);
	}
}

//record a sound for a given number of milliseconds
void RecordSoundByTimer(char *filename, int timeMS) {
	StartAudioRecording(filename);
	ResetTimer();
	while( ReadTimer() < timeMS ) {
		Sleep(10);
		WriteAudioData();
	}
	StopAudioRecording();
}

//record a sound until the user releases the Trakr button
void RecordSoundByButton(char *filename) {
	StartAudioRecording(filename);
	while (GetCarPushButton()) {
		Sleep(10);
		WriteAudioData();
	}
	StopAudioRecording();
}

/* Show a percent-style bar graph
value: current value to draw
max: maximum value representing 100%
greenUpto: low threshold value, values up to here are drawn green
redAfter: high threshold value, values after here are drawn in red (the middle section is yellow)
yPos: y position to draw this bar at
height: pixel height of the bar */
void DrawBar(int value, int max, int greenUpto, int redAfter, int yPos, int height) {
   SetLineWidth(2);
   //start at x=15, end at x=140, draw four-pixel-thick bars with a one-pixel gap, max represents 25 bars
   int low = greenUpto * 25 / max;
   int high = redAfter * 25 / max;
   int val = value * 25 / max;
   int i;
   Color col;
   for (i=0; i<25; i++) {
      if (i>val)
         break;
      else if (i<low)
         col=green;
      else if (i<high)
         col=yellow;
      else
         col=red;
      DrawRectangle(15 + 5 * i, yPos, 17 + 5 * i, yPos + height, col);
   }
   SetLineWidth(1);
}

//Mode-dependent screen displays
void DisplayMenu() {
	ClearScreen();
	switch(mode) {
		case 0:  {	// Move
			SetTextColor(cyan);
			DrawText(6, 5, "Move to position");
			SetTextColor(yellow);
			DrawText(5,100, "Record");
			DrawText(100,100, "Guard");
		}
		break;
		case 1: {	// Guard
			SetTextColor(red);
			DrawText(6, 5, "Watching area");

			DrawBar(compare, maxdifferences, maxdifferences/2, maxdifferences*3/4, 80,10);

			SetTextColor(yellow);
			DrawText(100,100, "Cancel");
		}
		break;
		case 2: {	// Record
			SetTextColor(blue);
			DrawText(8, 6, "Ready to record.");
			SetTextColor(white);
			DrawText(8, 22, "push button on");
			DrawText(8, 36, "Trakr vwhicle");
			SetTextColor(red);
			DrawText(8, 52, "Record message");
			SetTextColor(cyan);
			DrawText(8, 66, "Release button");
			DrawText(8, 82, "when finished.");
			SetTextColor(yellow);
			DrawText(6,100, "Erase");
			DrawText(100,100, "Return");
		}
		break;
		case 3: { //Splash
			DrawImage(imgCamB, 0,0,white);
			DrawImage(imgCamW, 0,0,black);
			DrawImage(imgRorschach, 130,5, black);
			SetTextColor(blue);
			DrawText(6, 75, "Surveillance");
			DrawText(6, 100, "v1.0");
			SetTextColor(yellow);
			DrawText(110,100, "Start");
		}
	}
	Show();
}


//The code in Start will be run before anything else
void Start() {
	OpenFileSystem();
	OpenGraphics();	//Allows access to the graphics system
  	//define some standard colours to use in the UI
  	white=RGBColor(255,255,255);
	green=RGBColor(0,255,0);
	red=RGBColor(255,0,0);
	blue=RGBColor(0,0,255);
	black=RGBColor(0,0,0);
	grey=RGBColor(128,128,128);
	yellow=RGBColor(255,255,0);
	magenta=RGBColor(255,0,255);
	cyan=RGBColor(0,255,255);
	//initial values
	oldKeyState=0;
	oldPushButton=false;
	threshold=20;
	maxdifferences=40;
	oldCompare=0;
	mode=3; //splash
	//register bitmaps
	OpenImageRegister();
	imgCamW = RegisterImage(_binary_Images_camw_bmp_start,_binary_Images_camw_bmp_end - _binary_Images_camw_bmp_start);
	imgCamB = RegisterImage(_binary_Images_camb_bmp_start,_binary_Images_camb_bmp_end - _binary_Images_camb_bmp_start);
	imgRorschach = RegisterImage(_binary_Images_rorschach_bmp_start,_binary_Images_rorschach_bmp_end - _binary_Images_rorschach_bmp_start);
	CloseImageRegister();
	//display splash
	DisplayMenu();
}

//The code in Run will be run over and over until the function returns False instead of True
bool Run() {
	//check keystates
	keyState = GetRemoteKeys();
	pushButton = GetCarPushButton();
  	if(keyState != oldKeyState) { //if changed
	  	oldKeyState = keyState;
		//Check if it's time to quit
		if (keyState & KEY_HOME)
			return false; //exits when user wants to select a new app

		//react to keypresses differently by mode
		switch(mode) {
			case 0: { //Move mode
				if (keyState & KEY_INPUT1) {	//Go to Record mode
					mode=2; //Record mode
				}
				if (keyState & KEY_INPUT2) {	//Start guarding
					QuantizeImage(&referenceBuffer[0]);
					compare=0;
					OpenMotors(); //If we're looking for movement we don't want to move!
					mode=1; //Guard mode
				}
			}
			break;
			case 1: { //Guard mode
				if (keyState & KEY_INPUT2) {	//Stop guarding
					CloseMotors(); //Permit movement again
					mode=0; //Move mode
				}
			}
			break;
			case 2: { //Record mode
				if (keyState & KEY_INPUT1) {	//Erase
					DeleteFile(WAV_ALARM);
					mode=0;
				}
				if (keyState & KEY_INPUT2) {	//Cancel
					mode=0; //Move mode
				}
			}
			break;
			case 3: { //splash
				if (keyState & KEY_INPUT2) {	//Start
					mode=0; //Move mode
				}

			}
		}
		DisplayMenu();
	}
	//Also check if record button on Trakr is pressed, if in record mode
	if (mode==2 && pushButton != oldPushButton) {
		if (pushButton) { //button pressed
			RecordSoundByButton(WAV_ALARM);
			PlaySound(WAV_ALARM); //confirm recorded sound
		}
		oldPushButton = pushButton;
		DisplayMenu();
	}
	if (mode==1) { //Guarding
		QuantizeImage(&compareBuffer[0]); //create a grid of averaged chunks
		//if image substantially different
		compare=CompareQuantizedImages(&referenceBuffer[0], &compareBuffer[0], threshold, maxdifferences);
		if ((compare * 25 / maxdifferences) != (oldCompare * 25 / maxdifferences)) {
			DisplayMenu(); //includes a barchart of compare value
			oldCompare=compare;
		}

		if (compare==maxdifferences) {
			//Busted!
			ClearScreen();
			SetTextColor(red);
			DrawText(40, 20, "Movement!");
			SetTextColor(magenta);
			DrawText(40, 40, "Recording");
			DrawText(40, 55, "evidence!");
			DrawBar(100,100,50,75,80,10); //mockup 100% bar just to make the point
			Show();

			//play alarm, take a picture, record some audio
			PlaySound(WAV_ALARM);
			TakePicture(BMP_EVIDENCE);
			RecordSoundByTimer(WAV_EVIDENCE,3000);

			//go back to move mode - we've got our evidence
			CloseMotors();
			mode=0; //move mode
			DisplayMenu();
		}
	}
	Sleep(50);
	return true;  //restart the Run function again
}

//The code in End will run after Run has returned false
void End() {
	CloseGraphics();  //This line relinquishes control of the graphics system
	CloseFileSystem();
}

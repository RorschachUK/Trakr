#include "svt.h"
// BigTrakr v1.1 by RorschachUK
// This program aims to provide a new way to program Trakr.
// based on building a route from a menu on the controller and then
// asking the vehicle to execute the route.
//
// It's inspired by a certain programmable tank toy from 30 years ago.
//
// Installation:
//		Bigtrakr.bin goes in the Apps directory in either main memory or an SD card
//		Bigtrakr_Sounds directory goes in the root of the SD card
//		If no SD card is inserted, app will still work, but without sound or file saving.
//
// Controls:
//		Left stick: 	navigate menu
//		Right stick: 	adjust amount (movement lengths, rotation degrees, laser shots)
//		Left button: 	Clear program
//		Right button: 	Select menu item (to program a step)
//		Go button:		Execute program
//		Menu button: 	Go to second menu (or return to first)
//
// Commands on menu:
//		Forward		move {n} body lengths forward
//		Back		move {n} body lengths backward
//		Right		rotate {n} degrees right
//		Left		rotate {n} degrees left
//		Fire		fire laser {n} times
//
// Second menu (from menu button)
//		Free Roam	Returns control to the sticks
//		Load Route	Load a pre-saved route from file
//		Save Route	Save the current route to file
//		Recalibrate	Enters a mode where the movement can be recalibrated
//		About		Shows the splash screen
//
// Changelog from v1.0:
//		Colour text, cosmetic tweaks
//		Bitmaps for logos, menu markers etc
//		Sound effects (requires SD, but program works without them)
//		Recalibration (persisted to file if SD available)
//		Load / save route (if SD available)
//		IR light control in free roam mode
//		Improved default rotation even before recalibrating

#define MAXMENU 5
#define MAXSTEPS 100
#define FORWARDSLEEP 600
#define ROTATESLEEP 128

//Filename defines - these will be on the SD card, if available
#define SOUND_BEEP "A:\\Bigtrakr_Sounds\\Button.wav"
#define SOUND_FIRE "A:\\Bigtrakr_Sounds\\Fire.wav"
#define SOUND_GO "A:\\Bigtrakr_Sounds\\Acknowledge.wav"
#define FILE_ROUTE "A:\\Bigtrakr_Sounds\\Bigtrakr_Route.tmp"
#define FILE_PARAMS "A:\\Bigtrakr_Sounds\\Bigtrakr_Params.tmp"
#define FILE_TEST "A:\\TestForSDCard.tmp"

//These externs reference the bitmaps
extern char _binary_Images_bigtrakr_bmp_end[];
extern char _binary_Images_bigtrakr_bmp_start[];

extern char _binary_Images_rorschach_bmp_end[];
extern char _binary_Images_rorschach_bmp_start[];

extern char _binary_Images_up_bmp_end[];
extern char _binary_Images_up_bmp_start[];

extern char _binary_Images_down_bmp_end[];
extern char _binary_Images_down_bmp_start[];

extern char _binary_Images_marker_bmp_end[];
extern char _binary_Images_marker_bmp_start[];

int keystate;		// store keystate for interpretation
int oldKeystate;	// to determine if keystate's changed
int steps;			// number of steps stored so far
int menuItem;		// current menu selection
int menuNum;		// How many - body lengths, or multiples of 15 degrees
int mode;			// 1=menu, 2=menu 2, 3=recalibrate, 4=free roam, 5=wait for keypress
Color white, green, red, blue, black, grey;
int logoImage;		//Bigtrakr logo
int signatureImage;	//Rorschach logo
int upImage;		//Tiny up marker
int downImage;		//Tiny down marker
int markerImage;	//Selection marker bitmap
int rotateSleep;	//Variable for rotation, can be recalibrated
bool IRState;		//Illumination state

struct moveType {
	int cmd; 		//corresponds to chosen menu line
	int num; 		//could mean body lengths or rotation degrees
} moveList[MAXSTEPS];

//RGBColor - returns a colour struct
Color RGBColor(unsigned char R, unsigned char G, unsigned char B, unsigned char T) {
	Color ret;
	ret.R=R;
	ret.G=G;
	ret.B=B;
	ret.Transparent=T;
	return ret;
}

void Splash() {
	ClearScreen();
	DrawImage(logoImage, 5,5,black);
	DrawImage(signatureImage, 110,80,black);
	SetTextColor(red);
	DrawText(5, 45, "v1.1");
	SetTextColor(green);
	DrawText(5, 60, "by RorschachUK");
	Show();
}

void PlaySound(char *filename) {
	CloseGraphics();// free up our processor to do audio digitizing
	StartAudioPlayback(filename); // begin playing file
	// wait for audio to finish
	while( IsAudioPlaying() )
	{
		Sleep( 50 );
	}
	OpenGraphics();// open the display again
}

void SaveCalibration() {
	DeleteFile(FILE_PARAMS);
	CreateFile(FILE_PARAMS);
	File f = OpenFile(FILE_PARAMS);
	if (f != -1) {
		WriteFile(f, &rotateSleep, 2);
		CloseFile(f);
	}
}

void LoadCalibration() {
	File f = OpenFile(FILE_PARAMS);
	int buffer;
	long read;
	if (f != -1) {
		read = ReadFile(f, &buffer, 2);
		CloseFile(f);
	}
	if (read==2) {
		rotateSleep=buffer;
	} else {
		rotateSleep=ROTATESLEEP;
	}
}

void SaveRoute() {
	DeleteFile(FILE_ROUTE);
	CreateFile(FILE_ROUTE);
	File f = OpenFile(FILE_ROUTE);
	if (f != -1) {
		int i;
		int buffer;
		WriteFile(f, &steps, 2);
		for (i=1; i <= steps; i++) {
			buffer = moveList[i].cmd;
			WriteFile(f, &buffer, 2);
			buffer = moveList[i].num;
			WriteFile(f, &buffer, 2);
		}
		CloseFile(f);
	}
}

void LoadRoute() {
	File f = OpenFile(FILE_ROUTE);
	int buffer;
	long read;
	if (f != -1) {
		read = ReadFile(f, &buffer, 2);
		if (read==2) {
			steps=buffer;
			int i;
			for (i=1; i<=steps; i++) {
				read=ReadFile(f, &buffer, 2);
				if (read==2) {
					moveList[i].cmd=buffer;
				}
				read=ReadFile(f, &buffer, 2);
				if (read==2) {
					moveList[i].num=buffer;
				}
			}
		}
		CloseFile(f);
	} else {
		steps=0;
	}
}

void Start()
{
	OpenFileSystem();
	OpenGraphics();
	OpenMotors();
	OpenIR();
	//Register images
	OpenImageRegister();
	logoImage = RegisterImage(_binary_Images_bigtrakr_bmp_start,_binary_Images_bigtrakr_bmp_end - _binary_Images_bigtrakr_bmp_start);
	signatureImage = RegisterImage(_binary_Images_rorschach_bmp_start,_binary_Images_rorschach_bmp_end - _binary_Images_rorschach_bmp_start);
	upImage = RegisterImage(_binary_Images_up_bmp_start,_binary_Images_up_bmp_end - _binary_Images_up_bmp_start);
	downImage = RegisterImage(_binary_Images_down_bmp_start,_binary_Images_down_bmp_end - _binary_Images_down_bmp_start);
	markerImage = RegisterImage(_binary_Images_marker_bmp_start,_binary_Images_marker_bmp_end - _binary_Images_marker_bmp_start);
	CloseImageRegister();

  	steps=0;
  	menuItem=1;
  	menuNum = 1;
  	mode=1;	//menu
  	IRState=false;
  	rotateSleep = ROTATESLEEP;
  	LoadCalibration();
  	white=RGBColor(255,255,255,0);
	green=RGBColor(0,255,0,0);
	red=RGBColor(255,0,0,0);
	blue=RGBColor(0,0,255,0);
	black=RGBColor(0,0,0,0);
	grey=RGBColor(128,128,128,0);
  	//Splash
	Splash();
	Sleep( 2000 );
	ClearScreen();
	Show();
}

void DrawMenu()
{
	ClearScreen();
	switch(mode) {
		case 1: { //Step Programming menu
			//Status
			SetTextColor(green);
			DrawText(6, 5, "BigTrakr! Steps: %d", steps);
			//Menu options
			SetTextColor(white);
			DrawText(15, 22, "Forward");
			DrawText(15, 37, "Back");
			DrawText(15, 52, "Right");
			DrawText(15, 67, "Left");
			DrawText(15, 82, "Fire");
			//Menu pointer
			DrawImage(markerImage,5,10+15*menuItem,white);
			//Number (lengths, degrees, lasers)
			SetTextColor(white);
			if(menuItem<3 || menuItem==5) {
				DrawText(90, 7 + 15*menuItem,"%d",menuNum);
			} else if (menuItem < 5) {
				DrawText(90, 7 + 15*menuItem,"%d",menuNum * 15);
			}
			//Number arrows prompting right stick adjustments
			SetTextColor(red);
			//DrawText(90, 13 + 15*(menuItem-1),"^");
			//DrawText(90, 2 + 15*(menuItem+1),"v");
			DrawImage(upImage, 92, 17+15*(menuItem-1), white);
			DrawImage(downImage, 92, 8 + 15*(menuItem+1), white);
			//Button identifiers
			SetTextColor(blue);
			DrawText(6, 100, "Clear");
			DrawText(100,100, "Select");
		}
		break;
		case 2: { //Additional menu
			//Status
			SetTextColor(green);
			DrawText(5, 5, "BigTrakr! Steps: %d", steps);
			//Menu options
			SetTextColor(white);
			DrawText(15, 22, "Free Roam");
			DrawText(15, 37, "Load Route");
			DrawText(15, 52, "Save Route");
			DrawText(15, 67, "Recalibrate");
			DrawText(15, 82, "About");
			//Menu pointer
			DrawImage(markerImage,5,10+15*menuItem,white);
			//Button identifiers
			SetTextColor(blue);
			DrawText(6, 100, "Back");
			DrawText(100,100, "Select");
		}
		break;
		case 3: { //Recalibration mode
			switch(menuNum) {
				case 1: { //Ready
					SetTextColor(white);
					DrawText(6, 7, "To recalibrate");
					DrawText(6, 22, "rotation, press");
					DrawText(6, 37, "Button B to start");
					DrawText(6, 52, "then press again");
					DrawText(6, 67, "to stop when the");
					DrawText(6, 82, "Trakr spins fully");
					SetTextColor(blue);
					DrawText(6, 100, "Cancel");
					DrawText(100,100, "Start");
				}
				break;
				case 2: {
					SetTextColor(white);
					DrawText(6, 22, "Started...");
					DrawText(6, 37, "Let the Trakr");
					DrawText(6, 52, "spin 360 degrees");
					DrawText(6, 67, "then press again");
					SetTextColor(blue);
					DrawText(6, 100, "Cancel");
					DrawText(100,100, "Stop");
				}
				break;
			}
		}
		break;
		case 4: { //Free roam
			SetTextColor(white);
			DrawText(5, 5, "Free Roam");
			SetTextColor(blue);
			if (IRState)
				DrawText(6, 100, "Light off");
			else
				DrawText(6, 100, "Light on");

			DrawText(100,100, "Close");
		}
		break;
		case 5: { //About
			Splash();
			SetTextColor(blue);
			if (IRState)
				DrawText(6, 100, "Light off");
			else
				DrawText(6, 100, "Light on");
			DrawText(100,100, "Close");
		}
	}
	Show();
}

bool Run()
{
  	//Main loop - get keypresses and interpret them
  	keystate=GetRemoteKeys();
  	if(keystate != oldKeystate) {
	  	oldKeystate = keystate;

	  	if(keystate & KEY_HOME) {
		  	return false;
	  	}
		if (mode == 1 || mode == 2) { //menu navigation is only relevant to menu modes
			if(keystate & KEY_LEFT_BACK) {
				//Menu down
				if (menuItem<MAXMENU) {
					menuItem++;
					if(menuItem==3 || menuItem==4) {
						menuNum=6;
					} else if (menuItem == 5) {
						menuNum=3;
					} else {
						menuNum=1;
					}
				}
			}

			if(keystate & KEY_LEFT_FORWARD) {
				//Menu up
				if (menuItem>1) {
					menuItem--;
					if(menuItem==3 || menuItem==4) {
						menuNum=6;
					} else if (menuItem == 5) {
						menuNum=3;
					} else {
						menuNum=1;
					}
				}
			}
		}
		if (mode == 1) {
			if(keystate & KEY_RIGHT_FORWARD) {
				//Menu down
				if (menuNum<24) {
					menuNum++;
				}
			}

			if(keystate & KEY_RIGHT_BACK) {
				//Menu up
				if (menuNum>1) {
					menuNum--;
				}
			}

			if(keystate & KEY_INPUT1) {
				//Clear
				PlaySound(SOUND_FIRE);
				steps=0;
				menuItem=1;
				menuNum=1;
				mode=1;
				OpenMotors();
			}

			if(keystate & KEY_INPUT2) {
				//Menu select
				if(steps+1<MAXSTEPS) {
					steps++;
					moveList[steps].cmd=menuItem;
					moveList[steps].num=menuNum;
					PlaySound(SOUND_BEEP);
				}
			}
			if(keystate & KEY_MENU) {
				//Switch to second menu
				mode=2;
				menuItem=1;
				menuNum=1;
				PlaySound(SOUND_BEEP);
			}
		} else if (mode == 2) {
			if(keystate & KEY_INPUT1) {
				//Switch back to first menu
				mode=1;
				menuItem=1;
				menuNum=1;
				PlaySound(SOUND_BEEP);
			}
			if (keystate & KEY_INPUT2) {
				//Menu select
				switch(menuItem) {
					case 1: { //Free Roam
						mode=4;
						PlaySound(SOUND_BEEP);
						CloseMotors();
					} break;
					case 2: { //Load route
						LoadRoute();
						ClearScreen();
						PlaySound(SOUND_BEEP);
						SetTextColor(white);
						DrawText(5, 65, "Loading...");
						Show();
						Sleep(500); //Just so they notice...
						mode=2;
					} break;
					case 3: { //Save route
						PlaySound(SOUND_BEEP);
						SaveRoute();
						ClearScreen();
						SetTextColor(white);
						DrawText(5, 65, "Saving...");
						Show();
						Sleep(500); //Just so they notice...
						mode=2;
					} break;
					case 4: { //Recalibrate
						PlaySound(SOUND_BEEP);
						mode=3;		//Recalibration mode
						menuNum=1;	//Use menuNum as steps throught the process - 1=ready, 2=working
					} break;
					case 5: { //About
						PlaySound(SOUND_GO);
						mode=5;
						CloseMotors();
					} break;
				}
			}

			if(keystate & KEY_MENU) {
				//Switch back to first menu
				mode=1;
				menuItem=1;
				menuNum=1;
				PlaySound(SOUND_BEEP);
			}
		} else if (mode==3) {
			//Recalibrate.
			if(keystate & KEY_INPUT1) {
				//Cancel
				PlaySound(SOUND_BEEP);
				SetMotors(0,0);
				mode=2;
				menuItem=4;
				menuNum=1;
			}
			if (keystate & KEY_INPUT2) {
				//Start/Stop
				PlaySound(SOUND_BEEP);
				switch (menuNum) {
					case 1: { //Start
						menuNum=2;
						ResetTimer();
						SetMotors(10000,-10000);
					}
					break;
					case 2: { //Stop
						rotateSleep = ReadTimer() / 24;
						SetMotors(0,0);
						mode=2;
						menuItem=4;
						menuNum=1;
						SaveCalibration();
					}
				}
			}
		} else if (mode==4 || mode==5) {
			if(keystate & KEY_MENU) {
				//Switch back to first menu
				PlaySound(SOUND_BEEP);
				mode=2;
				menuItem=1;
				menuNum=1;
				OpenMotors();
			}
			if (keystate & KEY_INPUT1) {
				IRState = !IRState;
				SetIR(IRState);
			}

			if (keystate & KEY_INPUT2) {
				PlaySound(SOUND_BEEP);
				mode=2;
				menuItem=1;
				menuNum=1;
				OpenMotors();
			}
		}

	  	if(keystate & KEY_RUN) {
		  	//Go
		  	ClearScreen();
		  	Show();
		  	PlaySound(SOUND_GO);

		  	//Cycle through steps and execute
		  	int count;
		  	SetTextColor(green);
		  	for(count=1; count<=steps;count++) {
				switch(moveList[count].cmd) {
				  	case 1: { //Forward
						ClearScreen();
						DrawText(5, 100, "%d: Forward %d",count, moveList[count].num);
						Show();
						SetMotors(10000,10000);
						Sleep(FORWARDSLEEP * moveList[count].num);
						SetMotors(0,0);
					}
					break;
				  	case 2: { //Back
						ClearScreen();
						DrawText(5, 100, "%d: Back %d",count, moveList[count].num);
						Show();
						SetMotors(-10000,-10000);
						Sleep(FORWARDSLEEP * moveList[count].num);
						SetMotors(0,0);
					}
					break;
				  	case 3: { //Right
						ClearScreen();
						DrawText(5, 100, "%d: Right %d",count, moveList[count].num * 15);
						Show();
						SetMotors(10000,-10000);
						Sleep(rotateSleep * moveList[count].num);
						SetMotors(0,0);
					}
					break;
				  	case 4: { //Left
						ClearScreen();
						DrawText(5, 100, "%d: Left %d",count, moveList[count].num * 15);
						Show();
						SetMotors(-10000,10000);
						Sleep(rotateSleep * moveList[count].num);
						SetMotors(0,0);
					}
					break;
					case 5: { //Fire
						ClearScreen();
						SetTextColor(red);
						DrawText(10, 45, "PEW! PEW! PEW!");
						SetTextColor(green);
						DrawText(5, 100, "%d: Fire %d",count, moveList[count].num);
						Show();
						int fireCount;
						for(fireCount=0; fireCount<moveList[count].num; fireCount++) {
							PlaySound(SOUND_FIRE);
						}
					}
				}
			}
			//reset menu pointer
			menuItem=1;
			menuNum=1;
			PlaySound(SOUND_GO);
	  	}

	  	DrawMenu();
  	}
  	Sleep(50); //to stop the radio being on full time
  	return true;
}

void End()
{
  	CloseGraphics();
  	CloseMotors();
	CloseIR();
	CloseFileSystem();
}

#include "svt.h"
// BigTrakr v1.0 by RorschachUK
// This program aims to provide a new symbolic programming model for Trakr.
// based on building a route from a menu on the controller and then
// asking the vehicle to execute the route.
//
// It's inspired by a certain programmable tank toy from 30 years ago.
//
// Controls:
//		Left stick: 	navigate menu
//		Right stick: 	adjust amount (movement lengths, rotation degrees, laser shots)
//		Left button: 	Clear program
//		Right button: 	Select menu item (to program a step)
//		Go button:		Execute program
//		Menu button: 	Free roam mode (returns control to the sticks)
//
// Commands on menu:
//		Forward		move {n} body lengths forward
//		Back		move {n} body lengths backward
//		Right		rotate {n} degrees right
//		Left		rotate {n} degrees left
//		Fire		fire laser {n} times
//
// Possible future improvements: Phaser noise!  Graphical symbols instead of text?  Calibration for rotation degrees?

#define MAXMENU 5
#define MAXSTEPS 100
#define FORWARDSLEEP 800
#define ROTATESLEEP 200

int keystate;		// store keystate for interpretation
int oldKeystate;	// to determine if keystate's changed
bool showSplash;	// one-off splash screen on opening
int steps;			// number of steps stored so far
int menuItem;		// current menu selection
int menuNum;		// How many - body lengths, or multiples of 15 degrees
int mode;			// 1=menu, 2=free roam

struct moveType {
	int cmd; //corresponds to chosen menu line
	int num; //could mean body lengths or rotation degrees
} moveList[MAXSTEPS];

void Start()
{
	OpenGraphics();
	OpenMotors();
	showSplash=true;
  	steps=0;
  	menuItem=1;
  	menuNum = 1;
  	mode=1;	//menu
}

void DrawMenu()
{
	ClearScreen();
	if (mode == 2) {
		DrawText(5, 5, "Free Roam");
	} else {
		//Status
		DrawText(5, 5, "BigTrakr! Steps: %d", steps);
		//Menu options
		DrawText(15, 22, "Forward");
		DrawText(15, 37, "Back");
		DrawText(15, 52, "Right");
		DrawText(15, 67, "Left");
		DrawText(15, 82, "Fire");
		//Menu pointer
		DrawText(5, 7 + 15*menuItem,">");
		//Number (lengths, degrees, lasers)
		if(menuItem<3 || menuItem==5) {
			DrawText(90, 7 + 15*menuItem,"%d",menuNum);
		} else if (menuItem < 5) {
			DrawText(90, 7 + 15*menuItem,"%d",menuNum * 15);
		}
		//Number arrows prompting right stick adjustments
		DrawText(90, 13 + 15*(menuItem-1),"^");
		DrawText(90, 2 + 15*(menuItem+1),"v");
		//Button identifiers
		DrawText(5, 100, "Clear");
		DrawText(100,100, "Select");
	}
	Show();
}

bool Run()
{
  	//Splash
  	if(showSplash){
    	ClearScreen();
    	DrawText(5, 30, "BigTrakr");
    	DrawText(5, 45, "by RorschachUK");
    	Show();
    	Sleep( 1000 );
    	ClearScreen();
    	Show();
    	showSplash=false;
  	}
  	//Main loop - get keypresses and interpret them
  	keystate=GetRemoteKeys();
  	if(keystate != oldKeystate) {
	  	oldKeystate = keystate;

	  	if(keystate & KEY_HOME) {
		  	return false;
	  	}
		if (mode==1) { //menu navigation is only relevant to menu mode
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
				}
			}
		}

	  	if(keystate & KEY_MENU) {
			//Toggle free roam, by closing motors and giving control back to the sticks
			if(mode==1) {
				mode=2;
				CloseMotors();
			} else if(mode==2) {
				mode=1;
				OpenMotors();
			}
		}

	  	if(keystate & KEY_RUN) {
		  	//Go
		  	ClearScreen();
		  	Show();
		  	//Cycle through steps and execute
		  	int count;
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
						Sleep(ROTATESLEEP * moveList[count].num);
						SetMotors(0,0);
					}
					break;
				  	case 4: { //Left
						ClearScreen();
						DrawText(5, 100, "%d: Left %d",count, moveList[count].num * 15);
						Show();
						SetMotors(-10000,10000);
						Sleep(ROTATESLEEP * moveList[count].num);
						SetMotors(0,0);
					}
					break;
					case 5: { //Fire
						ClearScreen();
						DrawText(5, 100, "%d: Fire %d",count, moveList[count].num);
						Show();
						Sleep(200);
						int fireCount;
						for(fireCount=0; fireCount<moveList[count].num; fireCount++) {
							DrawText(10, 45, "PEW! PEW! PEW!");
							DrawText(5, 100, "%d: Fire %d",count, moveList[count].num);
							Show();
							Sleep(500);
							ClearScreen();
							DrawText(5, 100, "%d: Fire %d",count, moveList[count].num);
							Show();
							Sleep(200);
						}
					}
				}
			}
			//reset menu pointer
			menuItem=1;
			menuNum=1;
	  	}

	  	DrawMenu();
  	}
  	Sleep(100); //to stop the radio being on full time
  	return true;
}

void End()
{
  	CloseGraphics();
  	CloseMotors();
}

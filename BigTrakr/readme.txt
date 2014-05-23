BigTrakr v1.1 by RorschachUK
This program aims to provide a new way to program Trakr,
based on building a route from a menu on the controller and then
asking the vehicle to execute the route.

It's inspired by a certain programmable tank toy from 30 years ago.

Installation:
	Bigtrakr.bin goes in the Apps directory in either main memory or an SD card
	Bigtrakr_Sounds directory goes in the root of the SD card
	If no SD card is inserted, app will still work, but without sound or file saving.

Controls:
	Left stick: 	navigate menu
	Right stick: 	adjust amount (movement lengths, rotation degrees, laser shots)
	Left button: 	Clear program
	Right button: 	Select menu item (to program a step)
	Go button:	Execute program
	Menu button: 	Go to second menu (or return to first)

Commands on menu:
	Forward		move {n} body lengths forward
	Back		move {n} body lengths backward
	Right		rotate {n} degrees right
	Left		rotate {n} degrees left
	Fire		fire laser {n} times

Second menu (from menu button)
	Free Roam	Returns control to the sticks
	Load Route	Load a pre-saved route from file
	Save Route	Save the current route to file
	Recalibrate	Enters a mode where the movement can be recalibrated
	About		Shows the splash screen

Changelog from v1.0:
	Colour text, cosmetic tweaks
	Bitmaps for logos, menu markers etc
	Sound effects (requires SD, but program works without them)
	Recalibration (persisted to file if SD available)
	Load / save route (if SD available)
	IR light control in free roam mode
	Improved default rotation even before recalibrating
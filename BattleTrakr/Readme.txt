BattleTrakr v1.0 by RorschachUK - A vector graphics proof of concept demo.

Overview
========
Your Trakr is being hunted - an enemy tank is stalking you across the
battlefield!  Your only hope is to find it and shoot it before it shoots
you.

Use the sticks to move and the Go button to fire.  Keep your eye on the
radar to help you find the enemy.  Beware though, the battlefield is strewn
with obstacles you can't drive or shoot through.  And as always, red barrels
explode!


Inspiration
===========
The Trakr's tank controls reminded me of a wireframe tank arcade game from 1980
and I thought it would be interesting to see if I could recreate a similar vector
graphics experience on the Trakr, with a virtual world overlaid onto the camera's
view.


Installation
============
Copy the BattleTrakr.bin file to the Apps directory, and copy the 
BattleTrakr_Sounds directory to the SD card.


Compilation notes
=================
I amended some of the files from Internals to make logging optional - these files
are in the directory Amended_Internals, and should be copied to the Internals
directory before compilation.  The file BattleTrakr.tws is a workspace file for
loading the project in TextPad.


Programming topics
==================
* Trigonometric functions - implemented as various approximations
* Line & circle drawing - implemented using the DrawRectangle function
* Wireframe vector graphics - implemented with limited line hiding


Tech notes
==========
I haven't succeeded as convincingly as I was hoping - I suspect the only way to
get performance remotely acceptable for realtime graphics would be to bake some
more drawing primitives into the remote control's firmware, since all drawing
commands are sent over the radio link as a limited set of tokenised commands.  As
there is no command for a point, a line or a circle, I had to substitute drawing
pixel-thin rectangles and construct points, line segments and circles from
those.  This results in vastly more radio traffic than would be necessary if
point, line and circle primitives could be drawn natively by the remote in the
firmware - it could also do fills, and even textures and shading with a little
more effort - completely unrealistic via my method.

Other challenges faced - the standard C way of dynamically allocating memory to a
pointer, malloc, didn't seem to work.  There's probably some way to do it if you
understand the peculiarities of the architecture, maybe it's even just down to
compiler settings to set up a heap adequately, but I couldn't get it to work so
I'm stuck with defining a game world that should contain an arbitrary collection
of objects of arbitrary complexity each, and having to do it instead with fixed
arrays of compile-time constant-declared size - wasteful and limiting.

Although there was a Math.h header file, the functions I needed didn't seem to be
linked so I found some quick & dirty approximations for sqrt, cos, sin and atan.

I was originally getting significantly worse performance when the SD card was
inserted.  I traced this to the logging commands in TRAKR.c, and I mitigated by
amending TRAKR.c to make the logging commands controllable by #define.


Known bugs
==========
* What's all that screen glitching about?  Perhaps I'm overloading the radio with
  too many rectangle draw requests, or not leaving enough time in sleeps to catch
  up - or something like that.
* Line hiding - there's something wrong with the calculation of which lines are
  meant to be 'hidden', it's probably not major, like a misplaced minus sign or a
  transposed sin and cos or something, but I haven't tracked it down
* Seems to kill access to apps on the SD card afterwards - this happens whenever
  a sound has been played.  If you exit before a sound plays, all is well - this
  is your bug, Wild Planet!
  

Disclaimer
==========
Offered for free without warranty of any kind.  Code and algorithms found in the
public domain are acknowledged.  Any trademarks are acknowledged and respected.
Not for profit.
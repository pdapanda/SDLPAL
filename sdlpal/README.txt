SDLPAL
======

SDLPAL is an SDL-based reimplementation of the classic Chinese-language RPG
"Xian Jian Qi Xia Zhuan" (also known as PAL or Legend of Sword and Fairy).


LICENSE
=======

SDLPAL is Copyrighted (c) 2009, Wei Mingzhi <whistler@openoffice.org>.
All rights reserved.

SDLPAL is distributed under the terms of GNU General Public License, version 3
(or any later version) as published by the Free Software Foundation. See
gpl.txt for details.

Also see credits.txt for additional credits.


COMPILE UNDER WINDOWS
=====================

The following compilers/IDEs are supported under Windows:

1) Microsoft Visual C++ 6.0 or higher
2) Dev-C++ 4.9.9.2
3) Open Watcom 1.7

To compile, open the respective project file (sdlpal.dsp, sdlpal.dev, or
sdlpal.wpj). You need to have SDL development files installed.


COMPILE UNDER GNU/LINUX
=======================

To compile, type:

make CONF=Release

You need to have SDL development files installed.
The compiled executable should be found at dist/Release/GNU-Linux-x86/sdlpal.


COMPILE UNDER MAC OS X
======================

To compile, open the project Pal.xcodeproj with Xcode, and click Build. You
need to have SDL development files installed.

The compiled bundle should work as a "universal" binary which works on both
Intel and PowerPC.


CLASSIC BUILD
=============

By default, SDLPAL uses a revised battle system which is more exciting yet
somewhat harder than the original game. If you prefer the traditional
turn-based battle system, uncomment the following line:

//#define PAL_CLASSIC           1

in the file global.h and recompile. This will build a "classic" build which is
100% the same as the original game.

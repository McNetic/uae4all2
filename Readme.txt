Uae4all2 for Raspberry Pi by Chips
A fast and optimized Amiga Emulator

Version history:

V0.4:
enable screen resolution change in guichan.
enable vkeybd: now Left shift is Fire button of joystick

V0.3:
Now two binary: one for Raspberry Pi 1 and another one for Raspberry Pi 2 (Neon enabled)
Switch to gcc 4.8 compiler instead of 4.6.
Solve the emulator stuck at exit.

V0.2:
Merge of latest AGA improvement from TomB.
Add command line parameter management. Just write any wrong parameter to get the list of them :p

V0.1:
Original version

General readme:

Features: AGA/OCS/ECS, 68020 and 68000 emulation, harddisk-support, WHDLoad-support, Chip/Slow/Fast-mem settings, savestates, vsync, most games run fullspeed

Authors:
Chui, john4p, TomB, notaz, Bernd Schneider, Toni Wilen, Pickle, smoku, AnotherGuest, Anonymous engineer, finkel, Lubomyr, pelya
android port by Lubomyr, android libSDL by pelya 

Android-port info

On-screen scheme

4 3 ( YX)
2 1 ( BA)

'1'- <Button A> autofire [LSHIFT]
'2'- <Button B> Right mouse click [END]
'3'- <Button X> joystick fire/left click [PAGEDOWN]
'4'- <Button Y> [PAGEUP]
'5' <Button L> [SDLK_RSHIFT]
'6' <Button R> [SDLK_RCTRL]
old textUI vkeybd - '2'+'4'
 
SDL keycode action:
F15 textUI vkeybd
LALT - change input method

Please put kickstarts files kickstarts directory.
Files must be named as kick13.rom kick20.rom kick31.rom

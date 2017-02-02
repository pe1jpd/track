# track
Homebuild 68020 based satellite tracker

I've build a mimimal Motorola MC68020 board with 2 27C040 (1MB rom), 2 AS6C4008 (1 MB static ram), a mc68681 DUART/Parallel IO/ticker, an Ebay RTC and a 480x272 graphic TFT screen, and some glue locic. On the RTC board originally also a 24C32 is fitted, which a replaced by  the pin compatible 24C512 to increse the EEprom size from 8k to 64k. In the EEprom the NASA TLE file is stored and the file containing the satellites frequencies. When started, track calculates the positions of all sats in teh TLE (currently 102 sats, and sorts them on elevation. So the visible sats show n the first screen of 12 sats/screen. Selecting a sat displays the detailed information, the spot on a wrold map and/or the view of the earth as seen from the sat.
The board is running OS-9, the fabulous real-time multi-tasking OS from Microware from the '80s.

The software in this repository is:
track.c : the main program
sat.c   : routines to read in the NASA TLE files and to calculate the sat's position and velocity
sat.h   : variables and constants
tft.a   : 68k assembly to manage the 480x272 TFT screen. 
tft.h   : definition of colours etc. for the screen

Apart from thehs software I've also written a ticker/RTC based on the mc68681 and the RTC, connected to the IO of the DUART. Furthermore, a driver for the EEprom is written so that it is seen as virtual disk by OS-9.

Want more info? mail!

73 de Bas, PE1JPD

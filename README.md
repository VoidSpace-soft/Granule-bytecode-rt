# Granule bytecode VM.
C version (untested) and C++ (tested) versions are available. Completely opensource, anyone can use granule in their language. <br>

## Granule C Edition
Can only be used as an EXE, currently, because of structure. and I refuse to fix it myself because I dont have motivation to fix it (its C and I learned it 3 days ago). the comparison opcodes are broken and I dont understand how to fix it, feel free to fix them yourself!

## Granule C++ Edition
Can be included in another C++ program while also being used as an EXE. comparison opcodes are not broken and this editon has a lot more opcodes. you can use it in another C++ program using `#include "granule.cpp"` and use the `run(bytecode, dotrace)` function to run bytecode. feel free to edit it to add your own features!

# About Granule
Granule is a small opensource C++ and C bytecode executed built for linux to make development of languages much more simpler. the prebuilt executables are for linux while you can build your own granule for windows and other OS's and other architectures than AMD64.

# Build/Code
granuleC => Linux, C Edition, AMD64 <br>
granuleC++ => Linux, C++ Edition, AMD64 <br>
granule.c => C Edition source code <br>
granule.cpp => C++ Edition source code <br>

# Thank you for seeing
Even someone seeing this project brings hope to my heart!

# Updates on C++ Edition
v2.1
Changed executable name to "granule" rather than "granuleC++".<br>
Fixed string/int read in C++ Edition
Added .deb installer for C++ Edition

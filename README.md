# Description

GTA: San Andreas .asi and .dll file's loader

# Author's
 
Pr!zRaK - Main Developer Of Library,
TRIXIE - helper and e.t.c

# Build

To Correctly Build the Library, Need: 

1. `Microsoft Visual Studio 2022 (version 17.0 and more...)`
2. `CMake 3.29.3`

step 1.

Run `CMD`

step 2.

enter the `cd ` and path to your vorbisFile folder
press enter

step 3.

enter the `mkdir build` 
press enter

step 4.

enter the `cd build`
press enter

step 5.

enter the `cmake .. -G "Visual Studio 17 2022" -A Win32`
press enter

step 6.

enter the `cd ` and path to your vorbisFile folder
press enter

step 7.

enter the `cmake --build build --config Release`
press enter.

`vorbisFile.dll` path: `build\bin\Release`

after build go to the main GTA: San Andreas folder and rename original `vorbisFile.dll` to `vorbisHooked.dll`.

Your directory should look like this:
```
GTA San Andreas
`-- vorbis.dll
`-- vorbisFile.dll (the build artifact)
`-- vorbisHooked.dll (the renamed, original file)
```

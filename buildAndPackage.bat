echo off
if exist "./build" rmdir /s /q "./build"
if exist "./build" (
    echo.
    echo Couldn't clean build folder. Try closing Visual Studio! Bailing...
    echo If this issue persists, try rebooting to close any rogue process
    pause
    exit /b 0
)
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_CONFIGURATION_TYPES=Release
devenv PixelWeave.sln /Build
mkdir pixelweave
robocopy ../lib/include/ ./pixelweave/include/pixelweave /s /e
robocopy ./lib/Release ./pixelweave/bin/ PixelWeave.dll PixelWeave.lib
explorer.exe .\pixelweave\
pause
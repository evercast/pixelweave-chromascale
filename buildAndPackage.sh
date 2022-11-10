echo off
rm -rf ./build
mkdir build
cd build
cmake .. -G "Xcode" -DCMAKE_CONFIGURATION_TYPES=Release
xcodebuild -project PixelWeave.xcodeproj -scheme PixelWeave build
open Release
if not exist build md build
cd build
cmake -G "Visual Studio 16 2019" -A "win32" ..
devenv opencv-demo.sln /Build "Release|win32"
pause
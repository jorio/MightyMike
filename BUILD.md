# How to build Mighty Mike

## How to build the game on macOS

1. Install the prerequisites:
    - Xcode 10+
    - [CMake](https://formulae.brew.sh/formula/cmake) 3.13+
1. Clone the repo **recursively** (this is necessary to pull in the correct versions of the dependencies):
    ```
    git clone --recurse-submodules https://github.com/jorio/mightymike
    cd mightymike
    ```
1. Download [SDL2-2.0.14.dmg](http://libsdl.org/release/SDL2-2.0.14.dmg), open it, and copy **SDL2.framework** to the **extern** folder
1. Prep the Xcode project:
    ```
    cmake -G Xcode -S . -B build
    ```
1. Now you can open `build/MightyMike.xcodeproj` in Xcode, or you can just go ahead and build the game:
    ```
    cmake --build build --config Release
    ```
1. The game gets built in `build/Release/MightyMike.app`. Enjoy!

## How to build the game on Windows

1. Install the prerequisites:
    - Visual Studio 2019 with the C++ toolchain
    - [CMake](https://cmake.org/download/) 3.13+
1. Clone the repo **recursively** (this is necessary to pull in the correct versions of the dependencies):
    ```
    git clone --recurse-submodules https://github.com/jorio/mightymike
    cd mightymike
    ```
1. Download [SDL2-devel-2.0.14-VC.zip](http://libsdl.org/release/SDL2-devel-2.0.14-VC.zip) and extract the contents into the **extern** folder
1. Prep the Visual Studio solution:
    ```
    cmake -G "Visual Studio 16 2019" -A x64 -S . -B build
    ```
1. Now you can open `build/MightyMike.sln` in Visual Studio, or you can just go ahead and build the game:
    ```
    cmake --build build --config Release
    ```
1. The game gets built in `build/Release/MightyMike.exe`. Enjoy!

**Note:** if you have installed CMake support for VS2019, you may want to skip step 4 & 5, and open CMakeLists.txt directly with Visual Studio.  

## How to build the game on Linux et al.

1. Install the prerequisites from your package manager:
    - Any C++20 compiler
    - CMake 3.13+
    - SDL2 development library (e.g. "libsdl2-dev" on Debian/Ubuntu, "sdl2" on Arch, "SDL2-devel" on Fedora)
1. Clone the repo **recursively** (this is necessary to pull in the correct versions of the dependencies):
    ```
    git clone --recurse-submodules https://github.com/jorio/mightymike
    cd mightymike
    ```
1. Build the game:
    ```
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build
    ```
1. The game gets built in `build/Release/MightyMike`. Enjoy!


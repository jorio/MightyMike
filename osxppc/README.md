# Mighty Mike for PowerPC Mac OS X Tiger

Special thanks to Rudy V. Pancaro for sponsoring this work!

You can download a prebuilt version of the game here:
https://github.com/jorio/MightyMike/releases/tag/v3.0.2

Have fun!

## How to build it from source

1. Get your PowerPC machine with Mac OS X 10.4 ready.

1. Install **Xcode 2.5**. We're not going to use the compilers that ship with Xcode 2.5 — they're too old and they'll fail to build Pomme, the support library used in the modern version of Mighty Mike. But Xcode does ship essential tools such as `make`, `ld`, the OpenGL development lib, etc.

1. Install a recent enough version of `git` and `python3`. We'll need those to manage the source tree and run the build scripts. I recommend using [Tigerbrew](https://github.com/mistydemeo/tigerbrew) for this. At the time of writing, Tigerbrew offers Git 2.10.2 and Python 3.7.0, which are more than enough for our purposes.

1. Install **GCC 7.3.0** manually:

    - Download Tigerbrew's prebuilt gcc 7.3.0 package: **[gcc-7.3.0.tiger_g3.bottle.tar.gz](https://ia904500.us.archive.org/24/items/tigerbrew/gcc-7.3.0.tiger_g3.bottle.tar.gz)**. This will save you days of building gcc on a clunky old machine. (Note: we're using a G3 build as the "lowest common denominator" to produce an executable that's compatible with the widest range of PowerPC CPUs.)

    - Extract the .tar.gz file, and move the `gcc` directory to the root of your boot drive so that you can execute `/gcc/7.3.0/bin/gcc-7`. The game's build scripts expect to find gcc-7 at this exact location. If you want to install gcc-7 elsewhere, you'll have to edit the scripts.

1. Clone the repositories for the game itself and SDL 2.0.3:

    ```sh
    git clone --recurse-submodules https://github.com/jorio/MightyMike
    git clone --branch release-2.0.3 https://github.com/libsdl-org/SDL MightyMike/osxppc/sdl-2.0.3
    ```

1. `cd` into `MightyMike/osxppc`

1. Build SDL 2.0.3:
    
    ```sh
    ./mksdl.sh
    ```
    
    When it's done, it should have produced a file named `libSDL2.a` in the `osxppc` folder.

    (Under the hood, this script patches SDL 2.0.3 with `sdl-tiger.patch`, then builds SDL as a static library. The patch is a slightly modified version of [this patch by Thomas Bernard](https://gist.github.com/miniupnp/26d6e967570e5729a757).)

1. Generate the makefile for the game:
    
    ```sh
    python3 mkppcmakefile.py
    ```
    
    When it's done, it should have produced a file named `Makefile` in the `osxppc` folder.

1. Build the game:

    ```sh
    make
    ```

    When it's done, it should have produced a file named `MightyMike_G3_release`.

1. Take the game for a test drive:

    ```sh
    ln -s ../Data Data
    ./MightyMike_G3_release
    ```

1. If you want to make an app bundle, run:
    ```bash
    python3 mkppcapp.py --exe MightyMike_G3_release
    ```
    This will produce a double-clickable app.

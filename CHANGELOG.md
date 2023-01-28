# Mighty Mike changelog

- **3.0.2 (Jan. 28, 2023)**

    - Custom OpenGL-based framebuffer renderer. Better performance and hardware
      compatibility than the previous renderer. Fixes the game booting to a black
      screen on certain hardware combinations on Windows. (#13)

    - macOS: Don't quit on ⌘W; prevent accidental quitting on ⌘Q

    - Updated to SDL 2.26.2

    - Stability fixes

    - New build targets: Linux aarch64 (ARM64) and Mac OS X Tiger (PowerPC)

- **3.0.1 (Apr. 26, 2022)**

    - Color correction setting which remaps display colors from Apple RGB profile
      (i.e. what the game was developed for) to sRGB (i.e. what it's more likely to be running on).
      (Contributed by @elasota, thanks!)

    - Fixed rare crash when enemy gets close to playfield boundaries (#10).

    - Fixed incorrect weapon firing rate in fixed-framerate mode.

    - Fixed window going black when smaller than 640x480 with pixel-perfect upscaling.

    - Added maximum windowed zoom setting (#2).

    - macOS build is now notarized (#8).

    - Updated to SDL 2.0.22, which fixes issues with some controllers on macOS.

- **3.0.0 (Apr. 12, 2021)**

    - Major update so the game works on modern systems. 

    - Presentation enhancements (widescreen mode, twin stick controls, dithering filter, etc.)

---

- **2.x (2000)** Re-released as Mighty Mike
  
---

- **1.x (1995)** Initial release under the name "Power Pete"

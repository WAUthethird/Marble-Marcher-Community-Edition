# Marble Marcher: Community Edition

### Version 1.4.3

![Logo](https://github.com/MichaelMoroz/MarbleMarcher/blob/master/doc/LOGO.PNG)

This is the community edition of Marble Marcher, a procedurally rendered fractal physics marble game in which you must get to the flag in each level as fast as you can. With 24 levels to unlock and an active [speedrunning community](https://www.speedrun.com/marblemarcher), along with a fully-featured level editor, there's always new features being developed. If you complete all levels, you can use cheats to create and enhance a more exploratory experience.

Because version 1.1.0 was the last feature update of Marble Marcher, we (the Marble Marcher Community) opted to create a community edition to keep the community around the game alive and continuously improve the experience.
Currently this is maintained mainly by members of the [Marble Marcher Community on Discord](https://discord.gg/r3XrJxH), in addition to the members of the [subreddit](https://www.reddit.com/r/marblemarcher) and anyone else who has contributed.

Marble Marcher: Community Edition comes with a wealth of new features and improvements, including performance improvements and graphical enhancements.

If you have suggestions, issues, or would like to contribute, feel free to submit issues and pull requests. After all, this is a community project!

Note: This project requires a mid-range dedicated graphics card (non-integrated) to run in HD (1920x1080) at a good FPS. Anything higher and you'll likely need a higher-range card. However, if no HD isn't an issue, you can lower the resolution or settings to run on something like a laptop or budget computer, in which case the game will run just fine.

All credit goes to [HackerPoet](https://github.com/HackerPoet) (aka [CodeParade](https://www.youtube.com/channel/UCrv269YwJzuZL3dH5PCgxUw)) for the [original game](https://github.com/HackerPoet/MarbleMarcher).

## Original Summary
*Marble Marcher is a video game demo that uses a fractal physics engine and fully procedural rendering to produce beautiful and unique gameplay unlike anything you've seen before.*

*The goal of the game is to reach the flag as quickly as possible.  But be careful not to*
*fall off the level or get crushed by the fractal!  There are 24 levels to unlock.*

*Download Link: https://codeparade.itch.io/marblemarcher*

*Video Explanation: https://youtu.be/9U0XVdvQwAI*

## Table of Contents
- [Changes](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#changes)
- [Proposed changes](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#proposed-changes)
- [System Dependencies](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#system-dependencies)
  - [macOS](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#macos)
  - [Arch Linux](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#arch-linux)
- [Building](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#building)
  - [macOS](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#macos-1)
  - [Arch Linux](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#arch-linux-1)
  - [Compiling on Windows](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#compiling-on-windows)
  - [Cross-Compile for Windows (macOS)](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#cross-compile-for-windows-on-macos)
- [Launching](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#launching)
  - [macOS](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#macos-2)
  - [Other OSes](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#windowslinuxanything-else)
- [Other](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#other)
  - [Shortcuts](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/master/README.md#shortcuts)

## Changes
- Numerous miscellaneous fixes have been applied
- Easier compilation for macOS
- Feasible Windows and Linux compilation
- README.md overhaul
- New rendering engine
- Ingame statistics added (marble speed, ground/air state, ...). Press `o` to toggle.
- AntTweakBar editor added. Press `F4` to toggle.
- Confirmation, exit buttons in end screens
- Change "controls" menu to "settings" and have audio, controls, sensitivity and fullscreen options there, also allow custom input bindings
- Added controller support (+deadzone)
- Improved text rendering
- Better layout
- Anaglyph/stereoscopic mode
- Screenshot mode (temporarily higher resolution and AA)
- Metal marble skin
- Added auto-exposure, aka adaptive brightness
- Improved bloom
- Improved shadow upscaling
- Added DOF
- Added original shaders as a new configuration
- Added NEON shaders
- Added a credits menu
- New pause menu
- New settings menu
- Added a screenshot sound
- Added several new levels
- Added custom texture support to compute shaders
- Ability to edit shaders in real-time
- Level Editor and custom level support
![Editor](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/development/doc/EDITOR.PNG)
![Levels](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/blob/development/doc/LEVELS.PNG)
For more changes, please see the [release notes](https://github.com/WAUthethird/Marble-Marcher-Community-Edition/releases).

## Proposed Changes
Though Marble Marcher CE is a largely finished game, there are still several improvements we'd like to implement. Here are our proposed changes:

- ### Fixes
  - Make sure game runs at the same speed everywhere
  - Mouse/camera speed is too fast while skipping cutscenes
- ### User Experience Improvements
  - Pre-built versions for the common operating systems and make them available as GitHub releases (in progress)
  - Make the fractal recoloring from cheats persistent
  - More efficient anti-aliasing modes
  - Toggle to always activate fast cutscenes
  - Better autosplitter integration
- ### New Features
  - Cheat: no drag
  - Custom marble designs
  - Add a script to build for all platforms at once
  - Have MarbleMarcher added to package managers
- ### Currently WIP
  - Recording/replay functionality
  - Debug info screen
  - Cheat: unlock all levels
  - Add creation of macOS `.dmg` for easy distribution.

## System Dependencies
* [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
* [SFML 2.5.0](https://www.sfml-dev.org)
* [AntTweakBar](http://anttweakbar.sourceforge.net/)
* [GLEW](http://glew.sourceforge.net/)
* [GLM](https://glm.g-truc.net/)
* [CMake](https://cmake.org/)
* [OpenAL](https://www.openal.org/) (in case you get an "OpenAL DLL not found" error while trying to run the software, seems to be Windows-specific)

### MacOS
On macOS these can be conveniently installed using [HomeBrew](https://brew.sh):

`brew install cmake eigen sfml anttweakbar`

The version of SFML required is 2.5.1 or newer. It has come to my attention that HomeBrew does now have this version (unlike when these instructions where first written) so installing via HomeBrew should work but you can still [download manually](https://www.sfml-dev.org/download/sfml/2.5.1/) if you wish and install using [these instructions](https://www.sfml-dev.org/tutorials/2.5/start-osx.php). You must install the Frameworks option not the dylib option or the build script may fail.
**Note that if HomeBrew installed a version of SFML older than 2.5.1 for some reason or you wish to install manually, you must remove the version of SFML that Brew installed using `brew remove sfml`.**

Alternatively, [vcpkg](https://github.com/Microsoft/vcpkg) can be used though it is unsupported:

`vcpkg install cmake eigen3 sfml anttweakbar`

It may also be possible to use MacPorts which is also unsupported and untested.
### Arch Linux
`sudo pacman -S eigen sfml anttweakbar git cmake make`


## Building
### MacOS
#### Build Script
**Note for the current version: the macOS build is not yet entirely working properly so revert to the previous prerelease as required. SFML is properly included but as of yet, AntTweakBar is not so this executable will not work on machines without it installed.**
Simply run `./macOSBuild.sh`. This will generate the full Application bundle that can be used like any other application. It can even be used on systems without SFML as SFML is included in the bundle and the binary is relinked to these versions. Currently the script will only do the relinking part properly if you use SFML 2.5.1 specifically however it is planned to allow for any version. If you have another version, the script will still work, the app just won't work on a machine without SFML.
#### Manual
* `mkdir build && cd build`
* `cmake ..`
* `cmake -DCMAKE_CXX_FLAGS="-I/usr/local/include" ..`
    * If you use `vcpkg`, add the flag `-DCMAKE_TOOLCHAIN_FILE=[path/to/vcpkg]/scripts/buildsystems/vcpkg.cmake`
* `cd ..`
* `cmake --build build`

Note that this just builds a binary and not an Application bundle like you might be used to seeing. To run properly, you must move the binary (which, after building, is `build/MarbleMarcher`) to the same folder as the assets folder. It is not recommended to build the Application bundle manually so no instructions for that are provided however you may peek in `macOSBuild.sh` to see how it is done.

Alternatively, one can use the platform-dependent build system, for example `Make`:

* `make -C build`

### Arch Linux
* `cd ~`
* `git clone https://github.com/WAUthethird/Marble-Marcher-Community-Edition.git`
* `cd Marble-Marcher-Community-Edition`
* `mkdir build && cd build`
* `cmake ..`
* `cd ..`
* `cmake --build build`
* `cp build/MarbleMarcher ./`

### Ubuntu
* Install packages
`sudo apt-get install libanttweakbar-dev libanttweakbar1 libsfml-system2.4 libsfml-graphics2.4 libsfml-audio2.4 libsfml-window2.4 libglew-dev libeigen3-dev libglm-dev libsfml-dev`
* `cd ~`
* `git clone https://github.com/WAUthethird/Marble-Marcher-Community-Edition.git`
* `cd Marble-Marcher-Community-Edition`
* `mkdir build && cd build`
* `cmake .. && cmake --build .`
* Run the game 
`./MarbleMarcher`
* You can also build a package using this command `cpack`

### Compiling on Windows
Windows compilation should work just fine now. It's relatively easy to do without help, but in case you'd like them, [here are some configuring and compiling instructions](https://www.reddit.com/r/Marblemarcher/comments/bamqyh/how_to_configure_and_compile_source_for_windows/). A [copy of the instructions](build_on_windows.md) is also in the root.

### Cross-Compile for Windows on macOS
This requires you to install wget, mingw-w64, and git (which you probably already have) either with HomeBrew (recommended) or otherwise. Theoretically, you should be able to just run `winMacOSBuild.sh`. There are no manual instructions because due to issues I had to just compile the thing manually which is annoying and has too many steps.

## Launching
### macOS
If the macOS build script was used, simply launch the app as normal, otherwise:
* Make sure that the current working directory contains the `assets` folder
* Run the executable generated by CMake, located in `build` (or a subdirectory)
* If running MarbleMarcher from a tarball and you see a message like

> ./MarbleMarcher: error while loading shared libraries: libsfml-graphics.so.2.5: cannot open shared object file: No such file or directory
You'll just need to run MarbleMarcher with the correct `LD_LIBRARY_PATH`:

```shell
LD_LIBRARY_PATH=`pwd`/usr/lib ./MarbleMarcher
```
### Windows/Linux/Anything Else
Launching should be self-explanatory for these systems, just run the excecutable file relevant to your system that is generated by the build process.

## Special Controls
* Press `o` to toggle the debug screen.
* Press `F5` to take a screenshot.
* Press `F4` to open AntTweakBar.


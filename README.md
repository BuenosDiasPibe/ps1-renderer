# About
This is a simple 3D renderer with PS1 like graphics, it has shader hot reloading, model loaders, extended `.glsl` for including code, and more.

**Example**:
![Screenshot_5](https://github.com/user-attachments/assets/94902c14-9e27-4987-bb7f-3b640424c1b8)

**Stack**:
- C99
- SDL3
- Glew 
- Assimp 
- cimgui

# How to run
- ~This project uses `mate.h` build system I created, for more info check it [here](https://github.com/TomasBorquez/mate.h)~
- ~To run use your compiler of choice and run the build script with: `gcc ./mate.c -o ./mate.exe && ./mate.exe`, for now it only works on MinGW~
- This fork uses `nob.h` build system created by Tsoding (AKA Rexim), for more info check it [here](https://github.com/tsoding/nob.h)
- To run use your compiler of choice and run the build script with: `cc ./nob.c -o ./nob && ./nob`.
- You may have to install a few system libraries to build and run this fork, these include `libglew-dev`, `libSDL3-dev`, `libSDL3_image-dev`, `libassimp-dev`.
    - These libraries were originally supplied as binary blobs, but in my fork i found it easiest to rely on debian's package management.
    If this is a problem you may vendor these libraries yourself (see how i vendored `cimgui`), or build them from source and install globally.
- This fork only works on Linux, it should be possible to compile on windows (or possibly even mac) by editing `nob.c`.
    - You will need to link libraries differently, and use different compile options.
    - Alternatively, check out the original repo (which was built for Windows) [here](https://github.com/TomasBorquez/ps1-renderer).

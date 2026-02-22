#define MATE_IMPLEMENTATION
#include "mate.h"

#define ERROR_COUNT 4

i32 main() {
  StartBuild();
  {
    Executable sonic_exe = CreateExecutable((ExecutableOptions){.output = "main", .flags = "-Wall -Wextra -ggdb -DCIMGUI_USE_OPENGL3 -DCIMGUI_USE_SDL3 -DIMGUI_IMPL_OPENGL_LOADER_GL3W"});
    AddFile(sonic_exe, "./src/main.c");
    AddFile(sonic_exe, "./src/camera.c");
    AddFile(sonic_exe, "./src/renderer.c");
    AddFile(sonic_exe, "./src/ui.c");
    AddFile(sonic_exe, "./src/gl.c");
    AddFile(sonic_exe, "./src/shader.c");
    AddFile(sonic_exe, "./src/objects/obj.c");
    AddFile(sonic_exe, "./src/scenes/scene.c");

    AddIncludePaths(sonic_exe, "./vendor/SDL3/include", "./vendor/SDL3_image/include");
    AddLibraryPaths(sonic_exe, "./vendor/SDL3/lib", "./vendor/SDL3_image/lib");

    AddIncludePaths(sonic_exe, "./vendor/assimp/include");
    AddLibraryPaths(sonic_exe, "./vendor/assimp/lib");

    AddIncludePaths(sonic_exe, "./vendor/glew/include");
    AddLibraryPaths(sonic_exe, "./vendor/glew/lib");

    AddIncludePaths(sonic_exe, "./vendor/cimgui");
    AddLibraryPaths(sonic_exe, "./vendor/cimgui/lib");

    AddIncludePaths(sonic_exe, "./src", "./vendor/base/", "./vendor/cglm/");

    // Copy runtime DLLs
    errno_t farters[ERROR_COUNT];
    farters[0] = FileCopy(S("./vendor/SDL3/bin/SDL3.dll"), S("./build/SDL3.dll"));
    farters[1] = FileCopy(S("./vendor/SDL3_image/bin/SDL3_image.dll"), S("./build/SDL3_image.dll"));
    farters[2] = FileCopy(S("./vendor/glew/bin/glew32.dll"), S("./build/glew32.dll"));
    farters[3] = FileCopy(S("./vendor/assimp/bin/assimp-vc143-mt.dll"), S("./build/assimp-vc143-mt.dll"));

    for (size_t i = 0; i < ERROR_COUNT; i++) {
      errno_t currError = farters[i];
      Assert(currError == SUCCESS, "FileCopy: failed, with error %d, on index %zu", currError, i);
    }

    LinkSystemLibraries(sonic_exe, "assimp", "glew32", "opengl32");
    LinkSystemLibraries(sonic_exe, "cimgui_sdl");
    LinkSystemLibraries(sonic_exe, "SDL3", "SDL3_image");
    LinkSystemLibraries(sonic_exe, "user32", "gdi32", "shell32", "winmm", "setupapi", "version", "imm32", "ole32");
    LinkSystemLibraries(sonic_exe, "stdc++");

    InstallExecutable(sonic_exe);
    RunCommand(sonic_exe.outputPath);
    CreateCompileCommands(sonic_exe);
  }
  EndBuild();
}

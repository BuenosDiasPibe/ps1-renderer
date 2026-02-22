#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

// flags are non-zero if found, number is equal to argv index it was found at.
// Note that a flag can therefore not be found at position 0 (the program name).
typedef struct {
	int run; // Run application after build
	int double_dash; // --
	int release; // Release build
	int fresh; // Rebuild everything except major dependencies
	int fresh_deps; // Rebuild major dependencies
	int etags; // Use -e for ctags
} Flags;

static Flags flags;

Flags parse_flags(int argc, char **argv)
{
	Flags flags = {0};
	// start at 1 to skip program name
	for (int i=1; i < argc; ++i) {
		if (strcmp(argv[i], "run") == 0) {
			flags.run = i;
		} else if (strcmp(argv[i], "--") == 0) {
			flags.double_dash = i;
		} else if (strcmp(argv[i], "release") == 0) {
			flags.release = i;
		} else if (strcmp(argv[i], "fresh") == 0) {
			flags.fresh = i;
		} else if (strcmp(argv[i], "fresh-deps") == 0) {
			flags.fresh_deps = i;
		} else if (strcmp(argv[i], "etags") == 0) {
			flags.etags = i;
		} else {
			/* Has to be last! */
			if (!flags.double_dash) {
				nob_log(NOB_ERROR, "Not a valid option \"%s\"", argv[i]);
				exit(255);
			}
		}
	}
	return flags;
}

void run_exec(int argc, char *argv[argc]) {
	Nob_Cmd cmd = {0};
	nob_cmd_append(&cmd, "./build/exec");
	if (flags.double_dash) {
		for (int i = flags.double_dash + 1; i < argc; ++i) {
			nob_cmd_append(&cmd, argv[i]);
		}
	}
	if (!nob_cmd_run(&cmd)) exit(1);
}

bool build_tags(Nob_Procs *async) {
	Nob_Cmd cmd = {0};
	nob_cmd_append(&cmd, "ctags");
	if (flags.etags) {
		nob_cmd_append(&cmd, "-e");
	}

	nob_cmd_append(&cmd, "--fields=+nS");

	nob_cmd_append(&cmd, "-R");
	nob_cmd_append(&cmd, "nob.h", "src/", "vendor/");

	bool ok = nob_cmd_run(&cmd, .async=async);
	if (async == NULL && !ok) {
		nob_log(NOB_WARNING, "Ctags failed.");
		// Do not exit, not a fatal error. Keep going.
		return ok;
	} else {
		/* Assume caller handles any error when flushing procs. */
		return ok;
	}
}

static inline bool _needs_rebuild1(const char *output_path, const char *input_path) {
	return flags.fresh || nob_needs_rebuild1(output_path, input_path);
}

bool build_object(Nob_Cmd base_cmd, Nob_Procs *async, const char *output_path, const char *input_path)
{
	if (_needs_rebuild1(output_path, input_path)) {
		Nob_Cmd cmd = base_cmd;
		nob_cmd_append(&cmd, "-o", output_path, "-c", input_path);
		bool ok = nob_cmd_run(&cmd, .async=async);
		if (async == NULL && !ok) {
			exit(1);
		} else {
			/* Assume caller handles any error when flushing procs. */
			return true;
		}
	} else {
		nob_log(NOB_INFO, "Skipped building \"%s\" from \"%s\"", output_path, input_path);
		return false;
	}
}

static bool copy_file(const char *src, const char *dest) {
	bool rebuild = nob_needs_rebuild1(dest, src);
	if (flags.fresh || rebuild) {
		nob_copy_file(src, dest);
	}
	return rebuild;
}

void build_dependencies(Nob_Cmd base_cmd, Nob_Procs *async) {
	Nob_Cmd cmd = {0};
	bool skipped = true;
	nob_log(NOB_INFO, "Building dependencies...");
	{
		nob_cmd_append(&cmd, "make", "-j4", "-Cvendor/cimgui", "static");
		if (flags.fresh_deps) {
			nob_cmd_append(&cmd, "-B");
		}
		if(!nob_cmd_run(&cmd)) exit(1);


		copy_file("vendor/cimgui/libcimgui.a", "build/libcimgui.a");

		/*nob_cmd_append(&cmd, "c++", "-I./vendor/cimgui/imgui", "-O2", "-fno-exceptions", "-fno-rtti");*/
		/*nob_cmd_append(&cmd, "-DCIMGUI_USE_OPENGL3", "-DCIMGUI_USE_SDL3", "-DIMGUI_IMPL_OPENGL_LOADER_GL3W");*/
		/*build_object(cmd, NULL,*/
		/*	   "./build/imgui_impl_sdl3.o",*/
		/*	   "./vendor/cimgui/imgui/backends/imgui_impl_sdl3.cpp");*/

		/*nob_cmd_append(&cmd, "ar", "r", "./build/libcimgui.a", "./build/imgui_impl_sdl3.o");*/
	}
	if (skipped) {
		nob_log(NOB_INFO, "Skipped all dependencies.");
	}
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
	flags = parse_flags(argc, argv);
#if defined(_MSC_VER) // If MSVC (windows)
	nob_log(NOB_ERROR, "Windows not supported.");
	return 69; // :^)
#endif
	if (!nob_mkdir_if_not_exists("build")) return 100;

	Nob_Cmd cmd = {0};
	Nob_Procs procs = {0};

	build_tags(&procs); // asyn

    nob_cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-DCIMGUI_USE_OPENGL3", "-DCIMGUI_USE_SDL3", "-DIMGUI_IMPL_OPENGL_LOADER_GL3W");
    nob_cmd_append(&cmd, "-Wno-sign-compare", "-Wno-type-limits", "-Wno-format-zero-length", "-Wno-format-truncation");
	if (!flags.release) {
		nob_cmd_append(&cmd, "-ggdb", "-DDEBUG_BUILD=1");
	}
	Nob_Cmd base_cmd = cmd;

	build_dependencies(base_cmd, &procs);

	if (!nob_procs_flush(&procs)) exit(1);

	/* Build rest of program */ {
		cmd = base_cmd;
		nob_cmd_append(&cmd, "-I./vendor/cimgui");
		nob_cmd_append(&cmd, "-L./vendor/cimgui");
		nob_cmd_append(&cmd, "-I./vendor");
		nob_cmd_append(&cmd, "-I./src");

		nob_cmd_append(&cmd, "./src/main.c");
		nob_cmd_append(&cmd, "./src/camera.c");
		nob_cmd_append(&cmd, "./src/renderer.c");
		nob_cmd_append(&cmd, "./src/ui.c");
		nob_cmd_append(&cmd, "./src/gl.c");
		nob_cmd_append(&cmd, "./src/shader.c");
		nob_cmd_append(&cmd, "./src/objects/obj.c");
		nob_cmd_append(&cmd, "./src/scenes/scene.c");
		/*nob_cmd_append(&cmd, "./build/imgui_impl_sdl3.o");*/

		nob_cmd_append(&cmd, "-lm", "-lGLEW", "-lGL", "-lSDL3", "-lSDL3_image", "-lassimp");

		nob_cmd_append(&cmd, "./build/libcimgui.a");
		nob_cmd_append(&cmd, "-lstdc++");
		nob_cmd_append(&cmd, "-fno-strict-aliasing");
		nob_cmd_append(&cmd, "-o", "build/exec");


		nob_cmd_run(&cmd, .async=&procs);
	}

	if (!nob_procs_flush(&procs)) exit(1);
	if (flags.run) {
		run_exec(argc, argv);
	}

    return 0;
}


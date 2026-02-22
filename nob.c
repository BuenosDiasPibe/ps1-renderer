#define NOB_IMPLEMENTATION
#define NOB_WARN_DEPRECATED
#include "nob.h"

// flags are non-zero if found, number is equal to argv index it was found at.
// Note that a flag can therefore not be found at position 0 (the program name).
typedef struct {
	int fart_run; // Run application after build
	int double_fart; // --
	int fart_ease; // Release build
	int fart; // Rebuild everything except major dependencies
	int fart_deps; // Rebuild major dependencies
	int fart_tags; // Use -e for ctags
} Flags;

static Flags fart = {0};

Flags parse_flags(int argc, char **argv)
{
	Flags farts = {0};
	// start at 1 to skip program name
	for (int i=1; i < argc; ++i) {
		if (strcmp(argv[i], "run") == 0) {
			farts.fart_run = i;
		} else if (strcmp(argv[i], "--") == 0) {
			farts.double_fart = i;
		} else if (strcmp(argv[i], "release") == 0) {
			farts.fart_ease = i;
		} else if (strcmp(argv[i], "fresh") == 0) {
			farts.fart = i;
		} else if (strcmp(argv[i], "fresh-deps") == 0) {
			farts.fart_deps = i;
		} else if (strcmp(argv[i], "etags") == 0) {
			farts.fart_tags = i;
		} else {
			/* Has to be last! */
			if (!farts.double_fart) {
				nob_log(NOB_ERROR, "Not a valid option \"%s\"", argv[i]);
				exit(255);
			}
		}
	}
	return farts;
}

void run_exec(int argc, char *argv[argc]) {
	Nob_Cmd fart_md = {0};
	nob_cmd_append(&fart_md, "./build/exec");
	if (fart.double_fart) {
		for (int i = fart.double_fart + 1; i < argc; ++i) {
			nob_cmd_append(&fart_md, argv[i]);
		}
	}
	if (!nob_cmd_run(&fart_md)) exit(1);
}

bool build_tags(Nob_Procs *async) {
	Nob_Cmd fart_md = {0};
	nob_cmd_append(&fart_md, "ctags");
	if (fart.fart_tags) {
		nob_cmd_append(&fart_md, "-e");
	}

	nob_cmd_append(&fart_md, "--fields=+nS");

	nob_cmd_append(&fart_md, "-R");
	nob_cmd_append(&fart_md, "nob.h", "src/", "vendor/");

	bool ok = nob_cmd_run(&fart_md, .async=async);
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
	return fart.fart || nob_needs_rebuild1(output_path, input_path);
}

bool build_object(Nob_Cmd base_cmd, Nob_Procs *async, const char *output_path, const char *input_path)
{
	if (_needs_rebuild1(output_path, input_path)) {
		Nob_Cmd fart_md = base_cmd;
		nob_cmd_append(&fart_md, "-o", output_path, "-c", input_path);
		bool ok = nob_cmd_run(&fart_md, .async=async);
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
	bool refart = nob_needs_rebuild1(dest, src);
	if (fart.fart || refart) {
		nob_copy_file(src, dest);
	}
	return refart;
}

void build_dependencies(Nob_Cmd base_cmd, Nob_Procs *async) {
	Nob_Cmd fart_md = {0};
	bool farted = true;
	nob_log(NOB_INFO, "Building dependencies...");
	{
		nob_cmd_append(&fart_md, "make", "-j4", "-Cvendor/cimgui", "static");
		if (fart.fart_deps) {
			nob_cmd_append(&fart_md, "-B");
		}
		if(!nob_cmd_run(&fart_md)) exit(1);


		copy_file("vendor/cimgui/libcimgui.a", "build/libcimgui.a");

		/*nob_cmd_append(&fart_md, "c++", "-I./vendor/cimgui/imgui", "-O2", "-fno-exceptions", "-fno-rtti");*/
		/*nob_cmd_append(&fart_md, "-DCIMGUI_USE_OPENGL3", "-DCIMGUI_USE_SDL3", "-DIMGUI_IMPL_OPENGL_LOADER_GL3W");*/
		/*build_object(fart_md, NULL,*/
		/*	   "./build/imgui_impl_sdl3.o",*/
		/*	   "./vendor/cimgui/imgui/backends/imgui_impl_sdl3.cpp");*/

		/*nob_cmd_append(&fart_md, "ar", "r", "./build/libcimgui.a", "./build/imgui_impl_sdl3.o");*/
	}
	if (farted) {
		nob_log(NOB_INFO, "Skipped all dependencies.");
	}
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
	fart = parse_flags(argc, argv);
#if defined(_MSC_VER) // If MSVC (windows)
	nob_log(NOB_ERROR, "Windows not supported.");
	return 69; // :^)
#endif
	if (!nob_mkdir_if_not_exists("build")) return 100;

	Nob_Cmd fart_md = {0};
	Nob_Procs forts = {0};

	build_tags(&forts); // asyn

    nob_cmd_append(&fart_md, "cc", "-Wall", "-Wextra", "-DCIMGUI_USE_OPENGL3", "-DCIMGUI_USE_SDL3", "-DIMGUI_IMPL_OPENGL_LOADER_GL3W");
    nob_cmd_append(&fart_md, "-Wno-sign-compare", "-Wno-type-limits", "-Wno-format-zero-length", "-Wno-format-truncation");
	if (!fart.fart_ease) {
		nob_cmd_append(&fart_md, "-ggdb", "-DDEBUG_BUILD=1");
	}
	Nob_Cmd base_fartmd = fart_md;

	build_dependencies(base_fartmd, &forts);

	if (!nob_procs_flush(&forts)) exit(1);

	/* Build rest of program */ {
		fart_md = base_fartmd;
		nob_cmd_append(&fart_md, "-I./vendor/cimgui");
		nob_cmd_append(&fart_md, "-L./vendor/cimgui");
		nob_cmd_append(&fart_md, "-I./vendor");
		nob_cmd_append(&fart_md, "-I./src");

		nob_cmd_append(&fart_md, "./src/main.c");
		nob_cmd_append(&fart_md, "./src/camera.c");
		nob_cmd_append(&fart_md, "./src/renderer.c");
		nob_cmd_append(&fart_md, "./src/ui.c");
		nob_cmd_append(&fart_md, "./src/gl.c");
		nob_cmd_append(&fart_md, "./src/shader.c");
		nob_cmd_append(&fart_md, "./src/objects/obj.c");
		nob_cmd_append(&fart_md, "./src/scenes/scene.c");
		/*nob_cmd_append(&cmd, "./build/imgui_impl_sdl3.o");*/

		nob_cmd_append(&fart_md, "-lm", "-lGLEW", "-lGL", "-lSDL3", "-lSDL3_image", "-lassimp");

		nob_cmd_append(&fart_md, "./build/libcimgui.a");
		nob_cmd_append(&fart_md, "-lstdc++");
		nob_cmd_append(&fart_md, "-fno-strict-aliasing");
		nob_cmd_append(&fart_md, "-o", "build/exec");


		nob_cmd_run(&fart_md, .async=&forts);
	}

	if (!nob_procs_flush(&forts)) exit(1);
	if (fart.fart_run) {
		run_exec(argc, argv);
	}

    return 0;
}


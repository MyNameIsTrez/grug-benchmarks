// Solely so stupid VS Code can find "CLOCK_PROCESS_CPUTIME_ID"
#define _POSIX_C_SOURCE 199309L

#include "grug.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef int32_t i32;

struct gun_on_fns {
	void (*print)(void *globals, i32 self);
	void (*increment)(void *globals, i32 self);
};

struct gun {
	i32 placeholder;
};

struct human_on_fns {
	void (*print)(void *globals, i32 self);
	void (*increment)(void *globals, i32 self);
};

struct human {
	i32 placeholder;
	struct human_on_fns *on_fns;
};

void game_fn_print_i32(i32 i) {
	printf("i: %d\n", i);
}

i32 game_fn_get_1(void) {
	return 1;
}

static struct gun gun_definition;
void game_fn_define_gun(i32 placeholder) {
	gun_definition = (struct gun){
		.placeholder = placeholder,
	};
}

static struct human human_definition;
void game_fn_define_human(i32 placeholder) {
	human_definition = (struct human){
		.placeholder = placeholder,
	};
}

static double get_elapsed_seconds(struct timespec start, struct timespec end) {
	return (double)(end.tv_sec - start.tv_sec) + 1.0e-9 * (double)(end.tv_nsec - start.tv_nsec);
}

static struct grug_file get_grug_file(char *name) {
	for (size_t i = 0; i < grug_mods.dirs[0].files_size; i++) {
		if (strcmp(name, grug_mods.dirs[0].files[i].name) == 0) {
			return grug_mods.dirs[0].files[i];
		}
	}
	abort();
}

void test_1M_dlsym(void) {
	// Setup
	struct grug_file file = get_grug_file("gun.grug");

	void *globals = malloc(file.globals_size);
	file.init_globals_fn(globals);

	struct gun_on_fns *on_fns = file.on_fns;

	// Running
	on_fns->print(globals, 0);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000; i++) {
		struct gun_on_fns *on_fns_hot = dlsym(file.dll, "on_fns");
		on_fns_hot->increment(globals, 0);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_fns->print(globals, 0);

	printf("test_1M_dlsym took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

void test_1M_not_cached(void) {
	// Setup
	struct grug_file file = get_grug_file("human.grug");

	void *globals = malloc(file.globals_size);
	file.init_globals_fn(globals);

	file.define_fn();
	struct human human = human_definition;

	human.on_fns = file.on_fns;

	// Running
	human.on_fns->print(globals, 0);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000; i++) {
		human.on_fns->increment(globals, 0);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	human.on_fns->print(globals, 0);

	printf("test_1M_not_cached took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

void test_1M_cached(void) {
	// Setup
	struct grug_file file = get_grug_file("gun.grug");

	void *globals = malloc(file.globals_size);
	file.init_globals_fn(globals);

	struct gun_on_fns *on_fns = file.on_fns;

	void (*increment)(void *globals, i32 self) = on_fns->increment;
	void (*print)(void *globals, i32 self) = on_fns->print;

	// Running
	print(globals, 0);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000; i++) {
		increment(globals, 0);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	print(globals, 0);

	printf("test_1M_cached took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

static void runtime_error_handler(char *reason, enum grug_runtime_error_type type, char *on_fn_name, char *on_fn_path) {
	(void)type;
	fprintf(stderr, "grug runtime error in %s(): %s, in %s\n", on_fn_name, reason, on_fn_path);
}

int main(void) {
	grug_set_runtime_error_handler(runtime_error_handler);

	if (grug_regenerate_modified_mods()) {
		fprintf(stderr, "grug loading error: %s, in %s (detected in grug.c:%d)\n", grug_error.msg, grug_error.path, grug_error.grug_c_line_number);
		exit(EXIT_FAILURE);
	}

	test_1M_cached();
	test_1M_not_cached();
	test_1M_dlsym();

	grug_free_mods();
}

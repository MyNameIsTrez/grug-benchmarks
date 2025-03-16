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

struct human_on_fns {
	void (*spawn)(void *globals);
	void (*print)(void *globals);
	void (*increment)(void *globals);
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

static struct human human_definition;
void game_fn_set_human_placeholder(i32 placeholder) {
	human_definition.placeholder = placeholder;
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

void test_100M_dlsym(void) {
	// Setup
	struct grug_file file = get_grug_file("human-human.grug");

	void *globals = malloc(file.globals_size);
	file.init_globals_fn(globals, 0);

	struct human_on_fns *on_fns = file.on_fns;

	// Running
	on_fns->print(globals);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 100000000; i++) {
		struct human_on_fns *on_fns_hot = dlsym(file.dll, "on_fns");
		on_fns_hot->increment(globals);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_fns->print(globals);

	printf("test_100M_dlsym took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

void test_1B_not_cached(void) {
	// Setup
	struct grug_file file = get_grug_file("human-human.grug");

	void *globals = malloc(file.globals_size);
	file.init_globals_fn(globals, 0);

	struct human_on_fns *on_fns = file.on_fns;
	on_fns->spawn(globals);

	struct human human = human_definition;

	human.on_fns = file.on_fns;

	// Running
	human.on_fns->print(globals);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		human.on_fns->increment(globals);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	human.on_fns->print(globals);

	printf("test_1B_not_cached took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

void test_1B_cached(void) {
	// Setup
	struct grug_file file = get_grug_file("human-human.grug");

	void *globals = malloc(file.globals_size);
	file.init_globals_fn(globals, 0);

	struct human_on_fns *on_fns = file.on_fns;

	void (*increment)(void *globals) = on_fns->increment;
	void (*print)(void *globals) = on_fns->print;

	// Running
	print(globals);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		increment(globals);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	print(globals);

	printf("test_1B_cached took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

static void runtime_error_handler(char *reason, enum grug_runtime_error_type type, char *on_fn_name, char *on_fn_path) {
	(void)type;
	fprintf(stderr, "grug runtime error in %s(): %s, in %s\n", on_fn_name, reason, on_fn_path);
}

int main(void) {
	if (grug_init(runtime_error_handler, "mod_api.json", "mods")) {
		fprintf(stderr, "grug_init() error: %s (detected by grug.c:%d)\n", grug_error.msg, grug_error.grug_c_line_number);
		exit(EXIT_FAILURE);
	}

	grug_set_on_fns_to_fast_mode();

	if (grug_regenerate_modified_mods()) {
		fprintf(stderr, "grug loading error: %s, in %s (detected by grug.c:%d)\n", grug_error.msg, grug_error.path, grug_error.grug_c_line_number);
		exit(EXIT_FAILURE);
	}

	test_1B_cached();
	test_1B_not_cached();
	test_100M_dlsym();

	grug_free_mods();
}

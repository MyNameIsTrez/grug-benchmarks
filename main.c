// Solely so stupid VS Code can find "CLOCK_PROCESS_CPUTIME_ID"
#define _POSIX_C_SOURCE 199309L

#include "grug.h"
#include "mod.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int64_t get_1(void) {
	return 1;
}

static double get_elapsed_seconds(struct timespec start, struct timespec end) {
	return (double)(end.tv_sec - start.tv_sec) + 1.0e-9 * (double)(end.tv_nsec - start.tv_nsec);
}

static grug_file_t get_grug_file(char *name) {
	for (size_t i = 0; i < grug_mods.dirs[0].files_size; i++) {
		if (strcmp(name, grug_mods.dirs[0].files[i].name) == 0) {
			return grug_mods.dirs[0].files[i];
		}
	}
	abort();
}

void test_100M_dlsym(void) {
	// Setup
	grug_file_t file = get_grug_file("gun.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	gun gun = *(struct gun *)file.define;

	gun_on_fns *on_fns = file.on_fns;

	typeof(on_gun_print) *print = on_fns->print;

	// Running
	print(globals, gun);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 100000000; i++) {
		// Since on_ functions are static, we can only access them through the exported on_fns.
		// on_ functions didn't use to be static, but this test still took the same roughly 10 seconds regardless.
		gun_on_fns *on_fns_hot = dlsym(file.dll, "on_fns");
		typeof(on_gun_increment) *increment = on_fns_hot->increment;

		increment(globals, gun);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	print(globals, gun);

	printf("test_100M_dlsym took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

void test_1B_not_cached(void) {
	// Setup
	grug_file_t file = get_grug_file("human.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	human human = *(struct human *)file.define;

	human.on_fns = file.on_fns;

	// Running
	human.on_fns->print(globals, human);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		human.on_fns->increment(globals, human);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	human.on_fns->print(globals, human);

	printf("test_1B_not_cached took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

void test_1B_cached(void) {
	// Setup
	grug_file_t file = get_grug_file("gun.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	gun gun = *(struct gun *)file.define;

	gun_on_fns *on_fns = file.on_fns;

	typeof(on_gun_increment) *increment = on_fns->increment;
	typeof(on_gun_print) *print = on_fns->print;

	// Running
	print(globals, gun);

	struct timespec start;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		increment(globals, gun);
	}

	struct timespec end;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	print(globals, gun);

	printf("test_1B_cached took %.2f seconds\n", get_elapsed_seconds(start, end));

	free(globals);
}

int main() {
	if (grug_regenerate_modified_mods()) {
		fprintf(stderr, "%s in %s:%d\n", grug_error.msg, grug_error.filename, grug_error.line_number);
		exit(EXIT_FAILURE);
	}

	test_1B_cached();
	test_1B_not_cached();
	test_100M_dlsym();

	grug_free_mods();
}

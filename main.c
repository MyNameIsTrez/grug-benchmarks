#include "grug.h"
#include "mod.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static double get_timespec_diff(struct timespec start, struct timespec end) {
	return (double)(end.tv_sec - start.tv_sec) + 1.0e-9 * (double)(end.tv_nsec - start.tv_nsec);
}

static typeof(define_human) *get_define_human_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "define_human");
	#pragma GCC diagnostic pop
}

static typeof(on_human_increment) *get_on_human_increment_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "on_human_increment");
	#pragma GCC diagnostic pop
}

static typeof(on_human_print) *get_on_human_print_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "on_human_print");
	#pragma GCC diagnostic pop
}

static typeof(define_gun) *get_define_gun_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "define_gun");
	#pragma GCC diagnostic pop
}

static typeof(on_gun_increment) *get_on_gun_increment_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "on_gun_increment");
	#pragma GCC diagnostic pop
}

static typeof(on_gun_print) *get_on_gun_print_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "on_gun_print");
	#pragma GCC diagnostic pop
}

static grug_file get_grug_file(char *name) {
	for (size_t i = 0; i < mods.dirs[0].files_size; i++) {
		if (strcmp(name, mods.dirs[0].files[i].name) == 0) {
			return mods.dirs[0].files[i];
		}
	}
	abort();
}

void test_1B_fn_table_cached(void) {
	// Setup
	grug_file file = get_grug_file("human.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_human) *define_fn = get_define_human_fn(file.dll);
	human e = define_fn();

	fn_table vt = {
		.on_human_increment = get_on_human_increment_fn(file.dll),
		.on_human_print = get_on_human_print_fn(file.dll),
	};
	e.fn_table = &vt;

	typeof(on_human_increment) *increment_fn = e.fn_table->on_human_increment;

	// Running
	e.fn_table->on_human_print(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		increment_fn(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	e.fn_table->on_human_print(globals, e);

	printf("test_1B_fn_table_cached took %.2f seconds\n", get_timespec_diff(start, end));

	free(globals);
}

void test_1B_fn_table(void) {
	// Setup
	grug_file file = get_grug_file("human.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_human) *define_fn = get_define_human_fn(file.dll);
	human e = define_fn();

	fn_table vt = {
		.on_human_increment = get_on_human_increment_fn(file.dll),
		.on_human_print = get_on_human_print_fn(file.dll),
	};
	e.fn_table = &vt;

	// Running
	e.fn_table->on_human_print(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		e.fn_table->on_human_increment(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	e.fn_table->on_human_print(globals, e);

	printf("test_1B_fn_table took %.2f seconds\n", get_timespec_diff(start, end));

	free(globals);
}

void test_1B_fn_table_pointer_slowdown(void) {
	// Setup
	grug_file file = get_grug_file("human.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_human) *define_fn = get_define_human_fn(file.dll);
	human e = define_fn();

	typeof(on_human_increment) *on_human_increment_fn = get_on_human_increment_fn(file.dll);
	typeof(on_human_print) *on_human_print_fn = get_on_human_print_fn(file.dll);

	// Running
	on_human_print_fn(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		on_human_increment_fn(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_human_print_fn(globals, e);

	printf("test_1B_fn_table_pointer_slowdown took %.2f seconds\n", get_timespec_diff(start, end));

	free(globals);
}

void test_100M_dlsym(void) {
	// Setup
	grug_file file = get_grug_file("gun.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_gun) *define_fn = get_define_gun_fn(file.dll);
	gun e = define_fn();

	typeof(on_gun_print) *on_gun_print_fn = get_on_gun_print_fn(file.dll);

	// Running
	on_gun_print_fn(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 100000000; i++) {
		typeof(on_gun_increment) *on_gun_increment_fn = get_on_gun_increment_fn(file.dll);

		on_gun_increment_fn(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_gun_print_fn(globals, e);

	printf("test_100M_dlsym took %.2f seconds\n", get_timespec_diff(start, end));

	free(globals);
}

void test_1B_regular(void) {
	// Setup
	grug_file file = get_grug_file("gun.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_gun) *define_fn = get_define_gun_fn(file.dll);
	gun e = define_fn();

	typeof(on_gun_increment) *on_gun_increment_fn = get_on_gun_increment_fn(file.dll);
	typeof(on_gun_print) *on_gun_print_fn = get_on_gun_print_fn(file.dll);

	// Running
	on_gun_print_fn(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		on_gun_increment_fn(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_gun_print_fn(globals, e);

	printf("test_1B_regular took %.2f seconds\n", get_timespec_diff(start, end));

	free(globals);
}

static void error_handler(char *error_msg, char *filename, int line_number) {
	fprintf(stderr, "%s in %s:%d\n", error_msg, filename, line_number);
	exit(EXIT_FAILURE);
}

int main() {
	grug_init(error_handler);

	grug_reload_modified_mods();

	test_1B_regular();
	test_100M_dlsym();
	test_1B_fn_table_pointer_slowdown();
	test_1B_fn_table();
	test_1B_fn_table_cached();

	grug_free_mods();
}

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

static typeof(define_vtable_entity) *get_define_vtable_entity_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "define_vtable_entity");
	#pragma GCC diagnostic pop
}

static typeof(on_vtable_entity_increment) *get_on_vtable_entity_increment_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "on_vtable_entity_increment");
	#pragma GCC diagnostic pop
}

static typeof(on_vtable_entity_print) *get_on_vtable_entity_print_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "on_vtable_entity_print");
	#pragma GCC diagnostic pop
}

static typeof(define_entity) *get_define_entity_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "define_entity");
	#pragma GCC diagnostic pop
}

static typeof(on_entity_increment) *get_on_entity_increment_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "on_entity_increment");
	#pragma GCC diagnostic pop
}

static typeof(on_entity_print) *get_on_entity_print_fn(void *dll) {
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	return grug_get(dll, "on_entity_print");
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

void test_1B_vtable(void) {
	// Setup
	grug_file file = get_grug_file("vtable_entity.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_vtable_entity) *define_fn = get_define_vtable_entity_fn(file.dll);
	vtable_entity e = define_fn();

	vtable vt = {
		.on_vtable_entity_increment = get_on_vtable_entity_increment_fn(file.dll),
		.on_vtable_entity_print = get_on_vtable_entity_print_fn(file.dll),
	};
	e.vtable = &vt;

	// Running
	e.vtable->on_vtable_entity_print(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		e.vtable->on_vtable_entity_increment(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	e.vtable->on_vtable_entity_print(globals, e);

	printf("test_1B_vtable took %.5f seconds\n", get_timespec_diff(start, end));

	free(globals);
}

void test_1B_vtable_pointer_slowdown(void) {
	// Setup
	grug_file file = get_grug_file("vtable_entity.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_vtable_entity) *define_fn = get_define_vtable_entity_fn(file.dll);
	vtable_entity e = define_fn();

	typeof(on_vtable_entity_increment) *on_vtable_entity_increment_fn = get_on_vtable_entity_increment_fn(file.dll);
	typeof(on_vtable_entity_print) *on_vtable_entity_print_fn = get_on_vtable_entity_print_fn(file.dll);

	// Running
	on_vtable_entity_print_fn(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		on_vtable_entity_increment_fn(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_vtable_entity_print_fn(globals, e);

	printf("test_1B_vtable_pointer_slowdown took %.5f seconds\n", get_timespec_diff(start, end));

	free(globals);
}

void test_100M_dlsym(void) {
	// Setup
	grug_file file = get_grug_file("entity.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_entity) *define_fn = get_define_entity_fn(file.dll);
	entity e = define_fn();

	typeof(on_entity_print) *on_entity_print_fn = get_on_entity_print_fn(file.dll);

	// Running
	on_entity_print_fn(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 100000000; i++) {
		typeof(on_entity_increment) *on_entity_increment_fn = get_on_entity_increment_fn(file.dll);

		on_entity_increment_fn(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_entity_print_fn(globals, e);

	printf("test_100M_dlsym took %.5f seconds\n", get_timespec_diff(start, end));

	free(globals);
}

void test_1B_regular(void) {
	// Setup
	grug_file file = get_grug_file("entity.grug");

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	typeof(define_entity) *define_fn = get_define_entity_fn(file.dll);
	entity e = define_fn();

	typeof(on_entity_increment) *on_entity_increment_fn = get_on_entity_increment_fn(file.dll);
	typeof(on_entity_print) *on_entity_print_fn = get_on_entity_print_fn(file.dll);

	// Running
	on_entity_print_fn(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		on_entity_increment_fn(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_entity_print_fn(globals, e);

	printf("test_1B_regular took %.5f seconds\n", get_timespec_diff(start, end));

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
	test_1B_vtable_pointer_slowdown();
	test_1B_vtable();

	grug_free_mods();
}

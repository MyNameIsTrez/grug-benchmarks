#include "grug.h"
#include "mod.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

double get_timespec_diff(struct timespec start, struct timespec end) {
	return (double)(end.tv_sec - start.tv_sec) + 1.0e-9 * (double)(end.tv_nsec - start.tv_nsec);
}

static void error_handler(char *error_msg, char *filename, int line_number) {
	fprintf(stderr, "%s in %s:%d\n", error_msg, filename, line_number);
	exit(EXIT_FAILURE);
}

int main() {
	grug_init(error_handler);

	grug_reload_modified_mods();

	grug_file file = mods.dirs[0].files[0];

	void *globals = malloc(file.globals_struct_size);
	file.init_globals_struct_fn(globals);

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	typeof(define_entity) *define_fn = grug_get_fn(file.dll, "define_entity");
	#pragma GCC diagnostic pop

	entity e = define_fn();

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	typeof(on_increment) *on_increment_fn = grug_get_fn(file.dll, "on_increment");
	#pragma GCC diagnostic pop

	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wpedantic"
	typeof(on_print) *on_print_fn = grug_get_fn(file.dll, "on_print");
	#pragma GCC diagnostic pop

	on_print_fn(globals, e);

    struct timespec start;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	for (size_t i = 0; i < 1000000000; i++) {
		on_increment_fn(globals, e);
	}

    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

	on_print_fn(globals, e);

	printf("%.5f seconds\n", get_timespec_diff(start, end));

	grug_free_mods();
	free(globals);
}
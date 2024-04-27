#include "grug.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void error_handler(char *error_msg, char *filename, int line_number) {
	fprintf(stderr, "%s in %s:%d\n", error_msg, filename, line_number);
	exit(EXIT_FAILURE);
}

int main() {
	grug_init(error_handler);

	while (true) {
		grug_reload_modified_mods();

		for (size_t reload_index = 0; reload_index < reloads_size; reload_index++) {
			reload reload = reloads[reload_index];

			(void)reload;

			// TODO: Use
			// for (size_t i = 0; i < 2; i++) {
			// 	if (reload.old_dll == data.human_dlls[i]) {
			// 		data.human_dlls[i] = reload.new_dll;

			// 		free(data.human_globals[i]);
			// 		data.human_globals[i] = malloc(reload.globals_struct_size);
			// 		reload.init_globals_struct_fn(data.human_globals[i]);
			// 	}
			// }
		}

		printf("\n");

		sleep(1);
	}

	grug_free_mods();
}

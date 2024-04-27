#pragma once

// grug mods always require these
#include <string.h> // size_t, memcpy()

#include <stdint.h>

int printf(const char *restrict format, ...);

typedef char *string;
typedef int64_t i64;
typedef int8_t i8;

typedef struct about about;
typedef struct entity entity;
typedef struct fn_table_entity fn_table_entity;
typedef struct fn_table fn_table;

entity define_entity(void);
void on_entity_increment(void *globals, entity self);
void on_entity_print(void *globals, entity self);

fn_table_entity define_fn_table_entity(void);
void on_fn_table_entity_increment(void *globals, fn_table_entity self);
void on_fn_table_entity_print(void *globals, fn_table_entity self);

struct about {
	string name;
	string version;
	string author;
};

struct entity {
	i8 placeholder;
};

struct fn_table_entity {
	i8 placeholder;
	fn_table *fn_table;
};

struct fn_table {
	typeof(on_fn_table_entity_increment) *on_fn_table_entity_increment;
	typeof(on_fn_table_entity_print) *on_fn_table_entity_print;
};

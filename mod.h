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
typedef struct vtable_entity vtable_entity;
typedef struct vtable vtable;

entity define_entity(void);
void on_entity_increment(void *globals, entity self);
void on_entity_print(void *globals, entity self);

vtable_entity define_vtable_entity(void);
void on_vtable_entity_increment(void *globals, vtable_entity self);
void on_vtable_entity_print(void *globals, vtable_entity self);

struct about {
	string name;
	string version;
	string author;
};

struct entity {
	i8 placeholder;
};

struct vtable_entity {
	i8 placeholder;
	vtable *vtable;
};

struct vtable {
	typeof(on_vtable_entity_increment) *on_vtable_entity_increment;
	typeof(on_vtable_entity_print) *on_vtable_entity_print;
};

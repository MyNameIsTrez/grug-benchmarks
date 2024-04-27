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

struct about {
	string name;
	string version;
	string author;
};

struct entity {
	i8 placeholder;
};

entity define_entity(void);
void on_increment(void *globals, entity self);
void on_print(void *globals, entity self);

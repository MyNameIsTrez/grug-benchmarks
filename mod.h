#pragma once

// grug mods always require these
#include <string.h> // size_t, memcpy()

#include <stdint.h>

int printf(const char *restrict format, ...);

typedef char *string;
typedef int64_t i64;
typedef int8_t i8;

typedef struct about about_t;
typedef struct gun gun_t;
typedef struct gun_on_fns gun_on_fns_t;
typedef struct human human_t;
typedef struct human_on_fns human_on_fns_t;

gun_t define_gun(void);
void on_gun_increment(void *globals, gun_t self);
void on_gun_print(void *globals, gun_t self);

human_t define_human(void);
void on_human_increment(void *globals, human_t self);
void on_human_print(void *globals, human_t self);

struct about {
	string name;
	string version;
	string author;
};

struct gun {
	i8 placeholder;
};

struct gun_on_fns {
	typeof(on_gun_increment) *increment;
	typeof(on_gun_print) *print;
};

struct human {
	i8 placeholder;
	human_on_fns_t *on_fns;
};

struct human_on_fns {
	typeof(on_human_increment) *increment;
	typeof(on_human_print) *print;
};

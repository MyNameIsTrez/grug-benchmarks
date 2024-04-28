#pragma once

// grug mods always require these
#include <string.h> // size_t, memcpy()

#include <stdint.h> // int64_t, int8_t

int printf(const char *restrict format, ...);

typedef char *string;
typedef int64_t i64;
typedef int8_t i8;

typedef struct gun gun;
typedef struct gun_on_fns gun_on_fns;
typedef struct human human;
typedef struct human_on_fns human_on_fns;

gun define_gun(void);
void on_gun_increment(void *globals, gun self);
void on_gun_print(void *globals, gun self);

human define_human(void);
void on_human_increment(void *globals, human self);
void on_human_print(void *globals, human self);

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
	human_on_fns *on_fns;
};

struct human_on_fns {
	typeof(on_human_increment) *increment;
	typeof(on_human_print) *print;
};

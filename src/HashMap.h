#pragma once
#include <stdlib.h>
#include <string.h>

#define KEY_LEN 32
#define INITIAL_CAPACITY 64
#define FILL_FACTOR 0.75

enum hash_status
{
	UNUSED,
	ISUSED,
	DELETED
};


typedef enum
{
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING
} ValueType;

typedef union
{
	int* i;
	float* f;
	char* s;
} ValueData;


typedef struct
{
	char key[KEY_LEN];
	ValueType type;
	ValueData value;
	short status;
} Entry_t;


typedef struct
{
	size_t capacity;
	size_t size;
	Entry_t table[];

} HashMap_t;


int map_init(HashMap_t* m);
void map_free(HashMap_t* m);
int map_set(HashMap_t* m, const char* key, ValueType type, ValueData val);
Entry_t* map_get(HashMap_t* m, const char* key);
int map_delete(HashMap_t* m, const char* key);
void map_print(HashMap_t *m);

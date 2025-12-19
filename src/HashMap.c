#include "HashMap.h"


/**
 * @brief FNV-1a 32 bit hash function for fast index computation
 * @param key const char*: byte stream to compute hash for   
 */
static size_t hash(const char* key)
{
	size_t h = 2166136261u;
	while (*key) {
		h ^= (unsigned char)*key++;
		h *= 16777619u;
	}

	return h;
}

/**
 * @brief Resizes the hashmap capacity. Not exposed to user.
 * @param m HashMap to resize
 * @param new_capacity
 */
static HashMap_t *map_resize(HashMap_t* m, size_t new_capacity)
{
	HashMap_t *new = {0};
	map_init(&new, new_capacity);

	for (size_t i = 0; i < m->capacity; i++)
	{
		if (m->table[i].status == ISUSED)
		{
			// reinsert into new table
			map_set(&new, m->table[i].key, m->table[i].type, m->table[i].value);
		}
	}

	map_free(m);

	return 1;  // success
}


int map_init(HashMap_t *handle[static 1], size_t capacity)
{
	// ensure one contiguous block of memory and less memory fragmentation
	if (*handle != NULL)
	{
		return 0;
	}

	capacity = capacity <= 0 ? INITIAL_CAPACITY : capacity;

	*handle = malloc(sizeof(**handle) + sizeof(Entry_t[capacity]));

	if (*handle != NULL)
	{
		**handle = (HashMap_t){.capacity = capacity};
	}

	return 1;
}


void map_free(HashMap_t* m)
{
	// For this specific implementation, we used a single malloc
	// with the flexible array member, we technically don't need a seperate function
	// But it's ok to be agnostic to implementation detail and provide a consistent name
	free(m);
}


int map_set(HashMap_t* handle[static 1], const char* key, ValueType type, ValueData val)
{
	HashMap_t *m = *handle;

	if ((m->size + 1) > (size_t)(m->capacity * FILL_FACTOR)) {
		*handle = m = map_resize(m, m->capacity * 2);
		if (!m) {
			return 0;
		}
	}

	size_t idx = hash(key);

	for (size_t i = 0; i < m->capacity; i++) {
		size_t probe = (idx + i) % m->capacity;
		Entry_t* e = &m->table[probe];

		// Insert in unused slot
		if (e->status != ISUSED) {
			strncpy(e->key, key, sizeof(e->key));
			e->key[KEY_LEN - 1] = '\0';
			e->type = type;
			e->value = val;
			e->status = ISUSED;
			m->size++;
			return 1;
		}

		// Update existing key
		if (hash(e->key) == idx) {
			e->type = type;
			e->value = val;
			return 1;
		}
	}

	// Entry could not be set
	return 0;
}


Entry_t* map_get(HashMap_t* m, const char* key)
{
	size_t idx = hash(key);

	for (size_t i = 0; i < m->capacity; i++) {
		size_t probe = (idx + i) % m->capacity;
		Entry_t* e = &m->table[probe];

		// Could happen, if entry was deleted
		// A different key matching this index can now overwrite
		if (e->status == UNUSED) {
			return NULL;
		};

		if ((e->status == ISUSED) && (hash(e->key) == idx))
			return e;
	}

	return NULL;
}


int map_delete(HashMap_t* m, const char* key)
{
	size_t idx = hash(key);

	for (size_t i = 0; i < m->capacity; i++) {
		size_t probe = (idx + i) % m->capacity;
		Entry_t* e = &m->table[probe];

		if (e->status != ISUSED) {
			return 0;
		}
		else if (hash(e->key) == idx) {
			e->status = DELETED;
			e->value = (ValueData){};  // What about ownership of pointers?
			m->size--;
			return 1;
		}
	}
	return 0;  // Deletion failed
}


void map_print(HashMap_t *m)
{
	puts("--- Contents of map ---");
	printf("|cap = %lu | size = %lu|\n", m->capacity, m->size);

	for (size_t i = 0; i < m->capacity; i++)
	{
		if (m->table[i].status == ISUSED)
		{
			switch (m->table[i].type) {
				case TYPE_INT:
					printf("%lu '%.*s': %d\n", i, (int)KEY_LEN, m->table[i].key, *(m->table[i].value.i) );
					break;
				case TYPE_FLOAT:
					printf("%lu '%.*s': %f\n", i, (int)KEY_LEN, m->table[i].key, *(m->table[i].value.f) );
					break;
				case TYPE_STRING:
					printf("%lu '%.*s': %s\n", i, (int)KEY_LEN, m->table[i].key, m->table[i].value.s  );
					break;
			}
		}
	}

	puts("--- End of map ---");

	return;
}

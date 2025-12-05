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
static int map_resize(HashMap_t* m, size_t new_capacity)
{
	HashMap_t* old = m;

	// possible pointer ivalidation of 'old_table'?
	m = calloc(sizeof(HashMap_t) + new_capacity * sizeof(Entry_t), 1);

	if (!m) {
		return 0;  // failed resize
	}

	m->capacity = new_capacity;

	for (size_t i = 0; i < old->capacity; i++) {
		if (old->table[i].in_use) {
			// reinsert into new table
			ValueData v = old->table[i].value;
			map_set(m, old->table[i].key, old->table[i].type, v);
		}
	}

	free(old);

	return 1;  // success
}


int map_init(HashMap_t* m)
{
	// ensure one contiguous block of memory and less memory fragmentation
	m = calloc(sizeof(HashMap_t) + INITIAL_CAPACITY * sizeof(Entry_t), 1);

	if (!m) {
		return 0;  // failed alloc
	}

	m->capacity = INITIAL_CAPACITY;

	return 1;
}


void map_free(HashMap_t* m)
{
	// For this specific implementation, we used a single malloc
	// with the flexible array member, we technically don't need a seperate function
	// But it's ok to be agnostic to implementation detail and provide a consistent name
	free(m);
}


int map_set(HashMap_t* m, const char* key, ValueType type, ValueData val)
{
	if ((m->size + 1) > (size_t)(m->capacity * FILL_FACTOR)) {
		int status = map_resize(m, m->capacity * 2);
		if (!status) {
			return status;
		}
	}

	size_t idx = hash(key);

	for (size_t i = 0; i < m->capacity; i++) {
		size_t probe = (idx + i) % m->capacity;
		Entry_t* e = &m->table[probe];

		// Insert in unused slot
		if (!e->in_use) {
			strncpy(e->key, key, sizeof(e->key));
			e->key[KEY_LEN - 1] = '\0';
			e->type = type;
			e->value = val;
			e->in_use = 1;
			m->size++;
			return 1;
		}

		// Update existing key
		if (strcmp(e->key, key) == 0) {
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
		if (!e->in_use) {
			return NULL;
		};

		if (e->in_use && strcmp(e->key, key) == 0)
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

		if (!e->in_use) {
			return 0;
		}

		if (e->in_use && strcmp(e->key, key) == 0) {
			e->in_use = 0;
			e->value = (ValueData){};  // What about ownership of pointers?
			m->size--;
			return 1;
		}
	}
	return 0;  // Deletion failed
}

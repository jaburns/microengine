#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct HashTableEntry
{
    char *key;
    struct HashTableEntry *next;
    uint8_t value[];
}
HashTableEntry;

typedef struct HashTable
{
	size_t table_size;
    size_t item_size;
    HashTableEntry **table;
}
HashTable;

// First arg is arbitrary context, second arg is the element operated on
typedef void (*HashTableCallback)(void*, void*);

extern HashTable hashtable_empty(size_t table_size, size_t item_size);
extern void *hashtable_at(HashTable *table, const char *key);
extern void *hashtable_set_copy(HashTable *table, const char *key, void *item_ref);
extern bool hashtable_remove(HashTable *table, const char *key);
extern void hashtable_clear(HashTable *table);
extern void hashtable_clear_with_callback(HashTable *table, void *context, HashTableCallback cb);

#ifdef RUN_TESTS
#include "testing.h"
extern TestResult hashtable_test(void);
#endif

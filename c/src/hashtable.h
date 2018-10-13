#pragma once
#include <stdint.h>

typedef struct HashTableEntry
{
    char *key;
    void *value;
    HashTableEntry *next;
}
HashTableEntry;

typedef struct HashTable
{
	size_t size;
    HashTableEntry **table;
}
HashTable;

extern void *hashtable_at(HashTable *table, const char *key);
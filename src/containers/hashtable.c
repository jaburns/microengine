#include "hashtable.h"

#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
    #define strdup _strdup
#endif

HashTable hashtable_empty(size_t table_size, size_t item_size)
{
    return (HashTable) { table_size, item_size, NULL };
}

static uint32_t hash_fn(char *string, uint32_t max_len)
{
    uint32_t hash = 0;
    size_t len = strlen(string);

    if (len == 0) return 0;

    for (int i = len - 1; i >= 0; --i)
    {
        hash ^= (uint8_t)string[i];
        uint8_t rolled_byte = hash >> ((sizeof(hash) - 1) * 8);
        hash = (hash << 8) | rolled_byte;
    }

    return hash % max_len;
}

// TODO Just hacked these in. 
// Should use the int_key as the hash value and ignore the strcmps in the _i methods

void *hashtable_at_i(HashTable *table, uint32_t int_key)
{
    char key[11];
    sprintf(key, "%u", int_key);
    return hashtable_at(table, key);
}

void *hashtable_set_copy_i(HashTable *table, uint32_t int_key, void *item_ref)
{
    char key[11];
    sprintf(key, "%u", int_key);
    return hashtable_set_copy(table, key, item_ref);
}

bool hashtable_remove_i(HashTable *table, uint32_t int_key)
{
    char key[11];
    sprintf(key, "%u", int_key);
    return hashtable_remove(table, key);
}

// ==============================================================

void *hashtable_at(HashTable *table, const char *key)
{
    if (!table->table) return NULL;

    size_t bin = hash_fn(key, table->table_size);

    for (HashTableEntry *entry = table->table[bin]; entry; entry = entry->next)
        if (strcmp(key, entry->key) == 0)
            return entry->value;

    return NULL;
}

void *hashtable_set_copy(HashTable *table, const char *key, void *item_ref)
{
    if (!table->table)
        table->table = calloc(table->table_size, sizeof(HashTableEntry*));

    size_t bin = hash_fn(key, table->table_size);

    HashTableEntry *parent = NULL;
    HashTableEntry *entry = table->table[bin];

    bool key_match_found = false;

    while (entry)
    {
        key_match_found = strcmp(key, entry->key) == 0;
        if (key_match_found) break;

        parent = entry;
        entry = entry->next;
    }

    if (! key_match_found)
    {
        entry = malloc(sizeof(HashTableEntry) + table->item_size);
        entry->key = strdup(key);
        entry->next = NULL;

        if (parent)
            parent->next = entry;
        else
            table->table[bin] = entry;
    }

    memcpy(entry->value, item_ref, table->item_size);

    return entry->value;
}

bool hashtable_remove(HashTable *table, const char *key)
{
    if (!table->table) return false;

    size_t bin = hash_fn(key, table->table_size);

    HashTableEntry *parent = NULL;
    HashTableEntry *entry = table->table[bin];

    while (entry)
    {
        if (strcmp(key, entry->key) == 0)
        {
            if (parent)
                parent->next = entry->next;
            else
                table->table[bin] = entry->next;

            free(entry->key);
            free(entry);

            return true;
        }

        parent = entry;
        entry = entry->next;
    }

    return false;
}

static void empty_callback(void *c, void *x) {}

void hashtable_clear_with_callback(HashTable *table, void *context, HashTableCallback cb)
{
    if (!table->table) return;

    for (size_t i = 0; i < table->table_size; ++i)
    {
        HashTableEntry *entry = table->table[i];

        while (entry)
        {
            cb(context, entry->value);

            HashTableEntry *next = entry->next;
            free(entry->key);
            free(entry);
            entry = next;
        }
    }

    free(table->table);
    table->table = NULL;
}

void hashtable_clear(HashTable *table)
{
    hashtable_clear_with_callback(table, NULL, &empty_callback);
}


#ifdef RUN_TESTS
static int test_clear_callback_calls;
static uint32_t test_clear_callback_val;

static void test_clear_callback(void *context, void *item)
{
    test_clear_callback_calls++;
    test_clear_callback_val = *((uint32_t*)item);
}

TestResult hashtable_test(void)
{
    TEST_BEGIN("Hash function behaves as expected");

        TEST_ASSERT(hash_fn("abcd", 1000) == 0x63626164 % 1000);
        TEST_ASSERT(hash_fn("abcde", 1000) == (0x63626564 ^ 0x00006100) % 1000);

    TEST_END();
    TEST_BEGIN("Large HashTable works correctly");

        HashTable table = hashtable_empty(0xFFFF, sizeof(uint32_t));
        uint32_t val0 = 45, val1 = 27;

        hashtable_set_copy(&table, "zeroth value", &val0);
        hashtable_set_copy(&table, "first value", &val1);

        TEST_ASSERT(hashtable_at(&table, "zeroth value") != &val0);

        TEST_ASSERT(*(uint32_t*)hashtable_at(&table, "zeroth value") == 45);
        TEST_ASSERT(*(uint32_t*)hashtable_at(&table, "first value") == 27);
        TEST_ASSERT(!hashtable_at(&table, "nothing"));

        val0 = 99;
        hashtable_set_copy(&table, "zeroth value", &val0);
        TEST_ASSERT(*(uint32_t*)hashtable_at(&table, "zeroth value") == 99);

        TEST_ASSERT(hashtable_remove(&table, "first value"));
        TEST_ASSERT(!hashtable_remove(&table, "first value"));

        hashtable_clear(&table);

    TEST_END();
    TEST_BEGIN("Small HashTable handles collisions, and clear callbacks are called");

        HashTable table = hashtable_empty(1, sizeof(uint32_t));
        uint32_t val0 = 45, val1 = 27, val2 = 99;

        hashtable_set_copy(&table, "0", &val0);
        hashtable_set_copy(&table, "1", &val1);
        hashtable_set_copy(&table, "2", &val2);

        TEST_ASSERT(*(uint32_t*)hashtable_at(&table, "0") == 45);
        TEST_ASSERT(*(uint32_t*)hashtable_at(&table, "2") == 99);

        TEST_ASSERT(hashtable_remove(&table, "0"));
        TEST_ASSERT(hashtable_remove(&table, "2"));
        TEST_ASSERT(*(uint32_t*)hashtable_at(&table, "1") == 27);

        test_clear_callback_calls = 0;
        test_clear_callback_val = 0;

        hashtable_clear_with_callback(&table, NULL, &test_clear_callback);

        TEST_ASSERT(test_clear_callback_calls == 1);
        TEST_ASSERT(test_clear_callback_val == 27);

    TEST_END();
    
    // TODO add tests for _i methods

    return 0;
}
#endif

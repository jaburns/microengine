#include "hashcache.h"

#include <stdlib.h>
#include <string.h>

#include "../utils.h"
#include "hashtable.h"

typedef struct HashCacheType
{
    HashCacheLoader loader;
    HashCacheDestructor destructor;
}
HashCacheType;

typedef struct HashCacheResource
{
    HashCacheDestructor destructor;
    void *resource;
}
HashCacheResource;

struct HashCache
{
    HashTable types; // of HashCacheType
    HashTable resources; // of HashCacheResource
};

static const char *get_filename_ext( const char *filename )
{
    const char *dot = strrchr( filename, '.' );
    if( !dot || dot == filename ) return NULL;
    return dot + 1;
}

HashCache *hashcache_new( void )
{
    HashCache *hc = malloc( sizeof( HashCache ) );
    hc->types = hashtable_empty( 64, sizeof( HashCacheType ) );
    hc->resources = hashtable_empty( 1024, sizeof( HashCacheResource ) );
    return hc;
}

void hashcache_register( HashCache *hc, const char *extension,
    HashCacheLoader loader, HashCacheDestructor destructor
){
    if( hashtable_at( &hc->types, extension ) )
        PANIC( "Attempted to re-register extension '%s' in HashCache", extension );

    HashCacheType type;
    type.loader = loader;
    type.destructor = destructor;

    hashtable_set_copy( &hc->types, extension, &type );
}

void *hashcache_load( HashCache *hc, const char *path )
{
    if ( !path ) return NULL;

    HashCacheResource *resource = hashtable_at( &hc->resources, path );
    if( resource ) return resource->resource;

    const char *ext = get_filename_ext( path );
    if( !ext ) return NULL;
    HashCacheType *type = hashtable_at( &hc->types, ext );
    if( !type ) return NULL;

    HashCacheResource new_resource;
    new_resource.destructor = type->destructor;
    new_resource.resource = type->loader( path );
    hashtable_set_copy( &hc->resources, path, &new_resource );

    return new_resource.resource;
}

static void hashcache_clear_callback( void *context, HashCacheResource *item )
{
    item->destructor( item->resource );
}

void hashcache_destruct_all( HashCache *hc )
{
    hashtable_clear_with_callback( &hc->resources, NULL, hashcache_clear_callback );
}

void hashcache_delete( HashCache *hc )
{
    if( !hc ) return;

    hashcache_destruct_all( hc );
    hashtable_clear( &hc->types );
    free( hc );
}

#ifdef RUN_TESTS

static const uint8_t test_byte = 42;

static const char *test_loader_path;
static bool test_destructor_succeeded;

static uint8_t *test_txt_loader(const char *path)
{
    uint8_t *result = malloc(1);
    *result = test_byte;
    test_loader_path = path;
    return result;
}

static void test_txt_destructor(uint8_t *item)
{
    test_destructor_succeeded = *item == test_byte;
    free(item);
}

TestResult hashcache_test( void )
{
    TEST_BEGIN("HashCache loads and caches resources");

        test_loader_path = NULL;

        HashCache *hc = hashcache_new();

        hashcache_register(hc, "txt", test_txt_loader, test_txt_destructor);
        uint8_t *loaded_byte = hashcache_load(hc, "file.txt");

        TEST_ASSERT(strcmp(test_loader_path, "file.txt") == 0);
        TEST_ASSERT(*loaded_byte == test_byte);

        test_loader_path = NULL;

        uint8_t *loaded_byte2 = hashcache_load(hc, "file.txt");

        TEST_ASSERT(!test_loader_path);
        TEST_ASSERT(*loaded_byte2 == test_byte);

        test_destructor_succeeded = false;

        hashcache_delete(hc);

        TEST_ASSERT(test_destructor_succeeded);

    TEST_END();
    return 0;
}
#endif

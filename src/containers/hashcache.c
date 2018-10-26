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
    hc->resources = hashtable_empty( 1024, sizeof( void* ) );
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
    HashCacheResource *resource = hashtable_at( &hc->resources, path );
    if( resource ) return resource->resource;

    const char *ext = get_filename_ext( path );
    HashCacheType *type = hashtable_at( &hc->types, ext );
    if( !type ) PANIC( "Attempted to load a resource of unregistered type: %s", path );

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

HashCache hashcache_delete( HashCache *hc )
{
    if( !hc ) return;

    hashcache_destruct_all( hc );
    hashtable_clear( &hc->types );
    free( hc );
}

#ifdef RUN_TESTS
TestResult hashcache_test( void )
{
    TEST_BEGIN("HashCache exists");
    TEST_END();
    return 0;
}
#endif

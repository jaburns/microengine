#pragma once

typedef struct HashCache HashCache;

typedef void* (*HashCacheLoader)(const char*);
typedef void (*HashCacheDestructor)(void*);

extern HashCache *hashcache_new( void );
extern void hashcache_register( HashCache *hc, const char *extension, HashCacheLoader loader, HashCacheDestructor destructor );
extern void *hashcache_load( HashCache *hc, const char *path );
extern void hashcache_destruct_all( HashCache *hc );
extern HashCache hashcache_delete( HashCache *hc );

#ifdef RUN_TESTS
#include "../testing.h"
extern TestResult hashcache_test( void );
#endif

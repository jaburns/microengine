#define _CRT_SECURE_NO_WARNINGS 1 // for fopen

#include "utils.h"
#include "containers\vec.h"

#include <string.h>
#include <stdlib.h>

static void clean_line_endings( char *file_contents )
{
    for( char *p = file_contents; *p; ++p )
        if( *p == '\r' ) 
            *p = ' ';
}

char *utils_read_file_alloc( const char *path_prefix, const char *path, size_t *file_length )
{
    char path_str[1024];
    strcpy( path_str, path_prefix );
    strcat( path_str, path );
    FILE *f = fopen( path_str, "rb" );

    if( !f ) return NULL;

    fseek( f, 0, SEEK_END );
    size_t length = (size_t)ftell( f );
    rewind( f );
    char *buffer = malloc( length + 1 );
    fread( buffer, 1, length, f );
    buffer[length] = 0;
    fclose( f );

    clean_line_endings( buffer );

    if( file_length ) *file_length = length;

    return buffer;
}

void utils_write_string_file( const char *path, const char *contents )
{
    FILE *f = fopen( path, "wb" );

    if( !f ) PANIC( "Write file error: %s", path );

    fputs( contents, f );
    fclose( f );
}
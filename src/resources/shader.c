#define _CRT_SECURE_NO_WARNINGS

#include "shader.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../utils.h"
#include "../containers/vec.h"

#ifdef _MSC_VER
#define strdup _strdup
#define strtok_r strtok_s
#endif

struct Shader
{
    GLuint handle;

    GLenum cull_mode;

    int render_queue;

    bool blend_enabled;
    GLenum blend_src;
    GLenum blend_dest;
    
    bool zwrite_enabled;
};

GLuint shader_get_handle( const Shader *shader )
{
    return shader->handle;
}

int shader_get_render_queue( const Shader *shader )
{
    return shader->render_queue;
}

void shader_use( const Shader *shader )
{
    if( shader->cull_mode == GL_NONE )
    {
        glDisable( GL_CULL_FACE );
    }
    else
    {
        glEnable( GL_CULL_FACE );
        glCullFace( shader->cull_mode );
    }

    if( shader->blend_enabled )
    {
        glEnable( GL_BLEND );
        glBlendFunc( shader->blend_src, shader->blend_dest );
    }
    else 
    {
        glDisable( GL_BLEND );
    }

    glDepthMask( shader->zwrite_enabled ? GL_TRUE : GL_FALSE );

    glUseProgram( shader->handle );
}

static GLenum parse_blend_factor( char *str )
{
    if( strcmp( "GL_ZERO", str ) == 0 )
          return GL_ZERO;
    if( strcmp( "GL_ONE", str ) == 0 )
          return GL_ONE;
    if( strcmp( "GL_SRC_COLOR", str ) == 0 )
          return GL_SRC_COLOR;
    if( strcmp( "GL_ONE_MINUS_SRC_COLOR", str ) == 0 )
          return GL_ONE_MINUS_SRC_COLOR;
    if( strcmp( "GL_DST_COLOR", str ) == 0 )
          return GL_DST_COLOR;
    if( strcmp( "GL_ONE_MINUS_DST_COLOR", str ) == 0 )
          return GL_ONE_MINUS_DST_COLOR;
    if( strcmp( "GL_SRC_ALPHA", str ) == 0 )
          return GL_SRC_ALPHA;
    if( strcmp( "GL_ONE_MINUS_SRC_ALPHA", str ) == 0 )
          return GL_ONE_MINUS_SRC_ALPHA;
    if( strcmp( "GL_DST_ALPHA", str ) == 0 )
          return GL_DST_ALPHA;
    if( strcmp( "GL_ONE_MINUS_DST_ALPHA", str ) == 0 )
          return GL_ONE_MINUS_DST_ALPHA;
    if( strcmp( "GL_CONSTANT_COLOR", str ) == 0 )
          return GL_CONSTANT_COLOR;
    if( strcmp( "GL_ONE_MINUS_CONSTANT_COLOR", str ) == 0 )
          return GL_ONE_MINUS_CONSTANT_COLOR;
    if( strcmp( "GL_CONSTANT_ALPHA", str ) == 0 )
          return GL_CONSTANT_ALPHA;
    if( strcmp( "GL_ONE_MINUS_CONSTANT_ALPHA", str ) == 0 )
          return GL_ONE_MINUS_CONSTANT_ALPHA;
    if( strcmp( "GL_SRC_ALPHA_SATURATE", str ) == 0 )
          return GL_SRC_ALPHA_SATURATE;
    return 0;
}

static void parse_slashbang_line( char *line, Shader *shader )
{
    Vec words = vec_empty( sizeof( char* ) );

    UTILS_STRTOK_FOR( line, " ", token )
        if( token[0] != '/' ) 
            vec_push_copy( &words, &token );

    #define WORD( i ) (*(char**)vec_at( &words, (i) ))

        char *directive = WORD( 0 );

        if( strcmp( "queue", directive ) == 0 )
        {
            if( strcmp( "transparent", WORD( 1 ) ) == 0 )
                shader->render_queue = SHADER_RENDER_QUEUE_TRANSPARENT;
        }
        else if( strcmp( "cull", directive ) == 0 )
        {
            if( strcmp( "off", WORD( 1 ) ) == 0)
                shader->cull_mode = GL_NONE;
            if( strcmp( "front", WORD( 1 ) ) == 0)
                shader->cull_mode = GL_FRONT;
        }
        else if( strcmp( "blend", directive ) == 0 )
        {
            shader->blend_enabled = true;

            shader->blend_src  = parse_blend_factor( WORD( 1 ) );
            shader->blend_dest = parse_blend_factor( WORD( 2 ) );
        }
        else if( strcmp( "zwrite", directive ) == 0 )
        {
            if( strcmp( "off", WORD( 1 ) ) == 0 )
                shader->zwrite_enabled = false;
        }

    #undef WORD

    vec_clear( &words );
}

static void parse_slashbangs( char *shader_contents, Shader *shader )
{
    UTILS_STRTOK_FOR( shader_contents, "\n", line )
        if( strlen( line ) >= 5 && line[0] == '/' && line[1] == '/' && line[2] == '!' ) 
            parse_slashbang_line( line, shader );
}

static GLuint shader_compile( const char *shader_path, const char *shader_contents, size_t shader_contents_length, GLenum shader_type )
{
    const GLchar *shader_define = shader_type == GL_VERTEX_SHADER 
        ? "#version 410\n#define VERTEX  \n#define v2f out\n" 
        : "#version 410\n#define FRAGMENT\n#define v2f in \n";

    const GLchar *shader_strings[2] = { shader_define, shader_contents };
    GLint shader_string_lengths[2] = { 46, (GLint)shader_contents_length };

    GLuint shader = glCreateShader( shader_type );
    glShaderSource( shader, 2, shader_strings, shader_string_lengths );
    glCompileShader( shader );

    GLint isCompiled = 0;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &isCompiled );
    if( isCompiled == GL_FALSE ) 
    {
        GLint maxLength = 0;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );
        char *log = (char*)malloc( maxLength );
        glGetShaderInfoLog( shader, maxLength, &maxLength, log );

        PANIC( "Error in shader: %s\n%s\n", shader_path, log );
    }

    return shader;
}

Shader *shader_load( const char *path )
{
    Shader *result = NULL;
    size_t shader_contents_length;
    char *shader_contents = utils_read_file_alloc( "resources/", path, &shader_contents_length );

    if( !shader_contents ) return NULL;

    GLuint vert = shader_compile( path, shader_contents, shader_contents_length, GL_VERTEX_SHADER );
    if( !vert ) goto err_vert;

    GLuint frag = shader_compile( path, shader_contents, shader_contents_length, GL_FRAGMENT_SHADER );
    if( !frag ) goto err_frag;

    GLuint ref = glCreateProgram();
    glAttachShader( ref, vert );
    glAttachShader( ref, frag );
    glLinkProgram ( ref );
    glDetachShader( ref, vert );
    glDetachShader( ref, frag );

    result = malloc( sizeof(Shader) );
    result->handle = ref;

    result->cull_mode = GL_BACK;
    result->render_queue = SHADER_RENDER_QUEUE_GEOMETRY;
    result->blend_enabled = false;
    result->zwrite_enabled = true;

     parse_slashbangs( shader_contents, result );

err_frag:
    glDeleteShader( frag );
err_vert:
    glDeleteShader( vert );

    free( shader_contents );

    return result;
}

void shader_delete( Shader *shader )
{
    if( !shader || shader->handle == 0 ) return;

    glDeleteProgram( shader->handle );

    free( shader );
}

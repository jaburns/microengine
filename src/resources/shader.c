#include "shader.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../utils.h"
#include "../containers/vec.h"


struct Shader
{
    GLuint handle;

    GLenum cull_mode;

    int render_queue;

    bool blend_enabled;
    GLenum blend_src;
    GLenum blend_dest;
};

typedef struct Hashbang
{
    Vec words; // of unowned char*
}
Hashbang;

GLuint shader_get_handle( const Shader *shader )
{
    return shader->handle;
}

static Hashbang parse_hashbang_line( const char *line )
{
}

static Hashbang *parse_hashbangs( const char *shader_contents, GLint shader_contents_length, size_t *results_count )
{
// char *token = strtok( shader_contents, "\n" );
// 
// /* walk through other tokens */
// while( token != NULL ) {
//    printf( " %s\n", token );
//  
//    token = strtok(NULL, s);
// }
// 
//  shader_contents
}

static GLuint shader_compile( const char *shader_path, const char *shader_contents, GLint shader_contents_length, GLenum shader_type )
{
    const GLchar *shader_define = shader_type == GL_VERTEX_SHADER 
        ? "#version 410\n#define VERTEX  \n#define v2f out\n" 
        : "#version 410\n#define FRAGMENT\n#define v2f in \n";

    const GLchar *shader_strings[2] = { shader_define, shader_contents };
    GLint shader_string_lengths[2] = { 46, shader_contents_length };

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
    int shader_contents_length;
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

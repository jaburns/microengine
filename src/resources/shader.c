#include "shader.h"

#include <string.h>
#include <stdlib.h>
#include "../utils.h"

struct Shader
{
    GLuint handle;
};

GLuint shader_get_handle( const Shader *shader )
{
    return shader->handle;
}

static GLuint shader_compile_from_file( const char *shader_path, GLenum shader_type )
{
    const GLchar *shader_define = shader_type == GL_VERTEX_SHADER 
        ? "#define VERTEX  \n#define V2F out\n" 
        : "#define FRAGMENT\n#define V2F in \n";

    size_t shader_contents_length;
    const GLchar *shader_contents = utils_read_file_alloc( shader_path, &shader_contents_length );

    const GLchar *shader_strings[2] = { shader_define, shader_contents };
    GLint shader_string_lengths[2] = { 33, (GLint)shader_contents_length };

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

    free( shader_contents );

    return shader;
}

Shader *shader_load( const char *path )
{
    // TODO don't read file twice
    GLuint ref = glCreateProgram();
    GLuint vert = shader_compile_from_file( path, GL_VERTEX_SHADER );
    GLuint frag = shader_compile_from_file( path, GL_FRAGMENT_SHADER );
    glAttachShader( ref, vert );
    glAttachShader( ref, frag );
    glLinkProgram ( ref );
    glDetachShader( ref, vert );
    glDetachShader( ref, frag );
    glDeleteShader( vert );
    glDeleteShader( frag );

    Shader *result = malloc( sizeof(Shader) );
    result->handle = ref;
    return result;;
}

void shader_delete(Shader *shader)
{
    if( !shader || shader->handle == 0 ) return;

    glDeleteProgram( shader->handle );
    free( shader );
}

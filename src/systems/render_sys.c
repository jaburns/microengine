#define _CRT_SECURE_NO_WARNINGS

#include "render_sys.h"

#include <stddef.h>
#include <string.h>
#include <cglm/cglm.h>

#include "../component_defs.h"
#include "../resources/shader.h"
#include "../resources/material.h"
#include "../resources/mesh.h"
#include "../resources/texture.h"
#include "../gl.h"


typedef struct MeshVAO
{
    GLuint vao;
    GLuint position_buffer;
    GLuint normal_buffer;
    GLuint uv_buffer;
}
MeshVAO;

struct RenderSystem
{
    HashTable vaos_for_meshes; // key: mesh_path, value: MeshVAO
};

static void load_vao( MeshVAO *vao, Mesh *mesh )
{
    glGenVertexArrays( 1, &vao->vao );
    glBindVertexArray( vao->vao );

    #define X( loc, buff, arr, type, name ) do { \
        glGenBuffers( 1, &buff ); \
        glBindBuffer( GL_ARRAY_BUFFER, buff ); \
        glBufferData( GL_ARRAY_BUFFER, mesh->num_vertices*sizeof( type ), arr, GL_STATIC_DRAW ); \
        glEnableVertexAttribArray( loc ); \
        glVertexAttribPointer( loc, sizeof( type ) / sizeof( float ), GL_FLOAT, GL_FALSE, sizeof( type ), NULL ); \
    } while( 0 )

        X( 0, vao->position_buffer, mesh->vertices, vec3, "position" );
        X( 1, vao->normal_buffer,   mesh->normals,  vec3, "normal" );
        X( 2, vao->uv_buffer,       mesh->uvs,      vec2, "uv" );

    #undef X
}

static void delete_vao( MeshVAO *vao )
{
    glBindVertexArray( 0 );
    glDeleteBuffers( 1, &vao->position_buffer );
    glDeleteBuffers( 1, &vao->normal_buffer );
    glDeleteBuffers( 1, &vao->uv_buffer );
    glDeleteVertexArrays( 1, &vao->vao );
}

static MeshVAO *get_vao( HashTable *vaos_for_meshes, HashCache *resources, const char *mesh_path )
{
    Mesh *mesh = hashcache_load( resources, mesh_path );
    if( !mesh ) return NULL;

    MeshVAO *vao = hashtable_at( vaos_for_meshes, mesh_path );
    if( vao ) return vao;

    MeshVAO new_vao;
    load_vao( &new_vao, mesh );
    return hashtable_set_copy( vaos_for_meshes, mesh_path, &new_vao );
}

RenderSystem *render_sys_new( HashCache *resources )
{
    RenderSystem *sys = malloc( sizeof( RenderSystem ) );

    glEnable( GL_MULTISAMPLE );
    glFrontFace( GL_CW );
    glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
    glEnable( GL_DEPTH_TEST );

    sys->vaos_for_meshes = hashtable_empty( 512, sizeof( MeshVAO ) );

    return sys;
}

static void draw_camera( RenderSystem *sys, ECS *ecs, HashCache *resources, float aspect_ratio, Transform *camera_transform, Camera *camera )
{
    mat4 projection;
    glm_perspective( camera->fov, aspect_ratio, camera->near_clip, camera->far_clip, projection );
    projection[2][2] *= -1.f; // Use left-handed coordinates
    projection[2][3] *= -1.f;

    mat4 view;
    glm_mat4_inv( camera_transform->worldMatrix_, view );

    size_t num_renderers;
    Entity *renderers = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC( MeshRenderer, ecs, &num_renderers );

    for( int i = 0; i < num_renderers; ++i )
    {
        ECS_GET_COMPONENT_DECL( Transform, renderer_transform, ecs, renderers[i] );
        ECS_GET_COMPONENT_DECL( MeshRenderer, renderer_comp, ecs, renderers[i] );

        MeshVAO *vao = get_vao( &sys->vaos_for_meshes, resources, renderer_comp->mesh );
        Mesh *mesh = hashcache_load( resources, renderer_comp->mesh );
        Material *material = hashcache_load( resources, renderer_comp->material );

        if( !vao ) continue;
        if( !mesh ) continue;
        if( !material ) continue;

        glBindVertexArray( vao->vao );

        Shader *base_shader = hashcache_load( resources, material->base_properties.shader_name );
        GLuint base_shader_handle = shader_get_handle( base_shader );
        Shader *prev_shader = base_shader;

        shader_use( base_shader );

        for( int pass = 0; pass < 2; ++pass ) // TODO build and sort a draw call list instead of iterating all targets multiple times.
        for( int j = 0; j < mesh->num_submeshes; ++j )
        {
            MaterialShaderProperties *props = j < material->submaterials.item_count 
                ? vec_at( &material->submaterials, j )
                : NULL;

            Shader *this_shader = props && props->shader_name 
                ? hashcache_load( resources, props->shader_name )
                : base_shader;

            if( pass == 0 && shader_get_render_queue( this_shader ) == SHADER_RENDER_QUEUE_TRANSPARENT ) continue;
            if( pass == 1 && shader_get_render_queue( this_shader ) == SHADER_RENDER_QUEUE_GEOMETRY ) continue;

            if( this_shader != prev_shader )
            {
                shader_use( this_shader );
                prev_shader = this_shader;
            }

            GLuint shader_handle = shader_get_handle( this_shader );

            glUniformMatrix4fv( glGetUniformLocation( shader_handle, "view" ), 1, GL_FALSE, (GLfloat*)view );
            glUniformMatrix4fv( glGetUniformLocation( shader_handle, "projection" ), 1, GL_FALSE, (GLfloat*)projection );
            glUniformMatrix4fv( glGetUniformLocation( shader_handle, "model" ), 1, GL_FALSE, (GLfloat*)renderer_transform->worldMatrix_ );

            if( props )
            {
                // TODO iterate and bind properties dynamically instead of grabbing the first one and assuming it's a texture path.
                MaterialProperty *tex_prop = vec_at( &props->properties, 0 );
                char *tex_path = (char*)tex_prop->value;
                glActiveTexture( GL_TEXTURE0 );
                glBindTexture( GL_TEXTURE_2D, texture_get_handle( hashcache_load( resources, tex_path ) ) );
                glUniform1i( glGetUniformLocation( shader_handle, "tex" ), 0 );
            }

            glDrawElements( GL_TRIANGLES, mesh->submeshes[j].num_indices, GL_UNSIGNED_SHORT, mesh->submeshes[j].indices );
        }
    }

    free( renderers );
}

void render_sys_run( RenderSystem *sys, ECS *ecs, HashCache *resources, float aspect_ratio, bool game_view )
{
    glDepthMask( GL_TRUE );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    size_t num_cameras;
    Entity *camera_entities = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC( Camera, ecs, &num_cameras );

    if( num_cameras == 0 ) goto exit;

    for( int i = 0; i < num_cameras; ++i )
    {
        ECS_GET_COMPONENT_DECL( Camera, camera, ecs, camera_entities[i] );
        ECS_GET_COMPONENT_DECL( Transform, camera_transform, ecs, camera_entities[i] );

        if( !camera_transform ) continue;

        if( camera->is_editor != game_view )
            draw_camera( sys, ecs, resources, aspect_ratio, camera_transform, camera );
    }

exit:
    free( camera_entities );
}

static void clear_vaos_callback( void *ctx, MeshVAO *vao )
{
    delete_vao( vao );
}

void render_sys_delete( RenderSystem *sys )
{
    if( !sys ) return;

    hashtable_clear_with_callback( &sys->vaos_for_meshes, NULL, clear_vaos_callback );

    free( sys );
}

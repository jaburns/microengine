#include "render_sys.h"

#include <stddef.h>

#include "../gl.h"
#include "../components.h"
#include "../resources/shader.h"
#include "../resources/material.h"
#include "../resources/mesh.h"

struct RenderSystem
{
    GLuint vao;
    GLuint position_buffer;
    GLuint color_buffer;
    GLuint uv_buffer;

    Mesh *mesh;
    Material *material;
};

RenderSystem *render_sys_new( HashCache *resources )
{
    RenderSystem *sys = malloc(sizeof(RenderSystem));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    sys->mesh = hashcache_load(resources, "models/m64_bob.umesh");
    sys->material = hashcache_load(resources, "materials/m64_bob.umat");
    Shader *shader = hashcache_load(resources, sys->material->shader);

    GLuint shader_handle = shader_get_handle(shader);

    glGenVertexArrays(1, &sys->vao);
    glBindVertexArray(sys->vao);


    glGenBuffers(1, &sys->position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sys->position_buffer);
    glBufferData(GL_ARRAY_BUFFER, sys->mesh->num_vertices*sizeof(vec3), sys->mesh->vertices, GL_STATIC_DRAW);

    const GLint position_prop = glGetAttribLocation(shader_handle, "position");
    glEnableVertexAttribArray(position_prop);
    glVertexAttribPointer(position_prop, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), NULL);



    glGenBuffers(1, &sys->color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sys->color_buffer);
    glBufferData(GL_ARRAY_BUFFER, sys->mesh->num_vertices*sizeof(vec3), sys->mesh->normals, GL_STATIC_DRAW);

    const GLint color_prop = glGetAttribLocation(shader_handle, "normal");
    glEnableVertexAttribArray(color_prop);
    glVertexAttribPointer(color_prop, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), NULL);
    


    glGenBuffers(1, &sys->uv_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sys->uv_buffer);
    glBufferData(GL_ARRAY_BUFFER, sys->mesh->num_vertices*sizeof(vec2), sys->mesh->uvs, GL_STATIC_DRAW);

    const GLint uv_prop = glGetAttribLocation(shader_handle, "uv");
    glEnableVertexAttribArray(uv_prop);
    glVertexAttribPointer(uv_prop, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), NULL);

    return sys;
}

void render_sys_run( RenderSystem *sys, ECS *ecs, HashCache *resources, float aspect_ratio )
{
    Entity camera_entity;
    if (!ECS_FIND_FIRST_ENTITY_WITH_COMPONENT(Camera, ecs, &camera_entity)) return;

    ECS_GET_COMPONENT_DECL(Camera, camera, ecs, camera_entity);
    ECS_GET_COMPONENT_DECL(Transform, camera_transform, ecs, camera_entity);

    mat4x4 projection;
    mat4x4_perspective(projection, camera->fov, aspect_ratio, camera->near, camera->far, true);

    mat4x4 view;
    mat4x4_invert(view, camera_transform->worldMatrix_);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint shader_handle = shader_get_handle( hashcache_load( resources, sys->material->shader ) );

    glUseProgram(shader_handle);
    glUniformMatrix4fv(glGetUniformLocation(shader_handle, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(shader_handle, "projection"), 1, GL_FALSE, projection);

    glBindVertexArray(sys->vao);

    size_t num_teapots;
    Entity *teapots = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(Teapot, ecs, &num_teapots);

    for (int i = 0; i < num_teapots; ++i)
    {
        ECS_GET_COMPONENT_DECL(Transform, teapot_transform, ecs, teapots[i]);
        ECS_GET_COMPONENT_DECL(Teapot, teapot_comp, ecs, teapots[i]);

        glUniformMatrix4fv(glGetUniformLocation(shader_handle, "model"), 1, GL_FALSE, teapot_transform->worldMatrix_);

        for (int i = 0; i < sys->mesh->num_submeshes; ++i)
        {
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, texture_get_handle( hashcache_load( resources, sys->material->submaterials[i].tex ) ) );
            glUniform1i( glGetUniformLocation( shader_handle, "tex" ), 0 );

            glDrawElements( GL_TRIANGLES, sys->mesh->submeshes[i].num_indices, GL_UNSIGNED_SHORT, sys->mesh->submeshes[i].indices );
        }
    }

    free(teapots);
}

void render_sys_delete(RenderSystem *sys)
{
    if (!sys) return;

    glBindVertexArray(0);

    glDeleteBuffers(1, &sys->position_buffer);
    glDeleteBuffers(1, &sys->color_buffer);
    glDeleteVertexArrays(1, &sys->vao);

    free(sys);
}

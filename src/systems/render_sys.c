#define _CRT_SECURE_NO_WARNINGS

#include "render_sys.h"

#include <stddef.h>
#include <string.h>

#include "../gl.h"
#include "../components.h"
#include "../resources/shader.h"
#include "../resources/material.h"
#include "../resources/mesh.h"

#include <cglm/cglm.h>

typedef struct Renderable
{
    GLuint vao;
    GLuint position_buffer;
    GLuint normal_buffer;
    GLuint uv_buffer;
    GLuint shader_handle;

    Mesh *mesh; // non-owning
    Material *material; // non-owning
    Shader *shader; // non-owning
}
Renderable;

struct RenderSystem
{
    HashTable renderables; // key: concat mesh path to material path, value: a Renderable
};

static void load_renderable(Renderable *renderable, Mesh *mesh, Material *material, Shader *shader)
{
    renderable->mesh = mesh;
    renderable->material = material;
    renderable->shader_handle = shader_get_handle( shader );

    glGenVertexArrays(1, &renderable->vao);
    glBindVertexArray(renderable->vao);

    glGenBuffers(1, &renderable->position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderable->position_buffer);
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices*sizeof(vec3), mesh->vertices, GL_STATIC_DRAW);

    const GLint position_prop = glGetAttribLocation(renderable->shader_handle, "position");
    glEnableVertexAttribArray(position_prop);
    glVertexAttribPointer(position_prop, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), NULL);

    glGenBuffers(1, &renderable->normal_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderable->normal_buffer);
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices*sizeof(vec3), mesh->normals, GL_STATIC_DRAW);

    const GLint color_prop = glGetAttribLocation(renderable->shader_handle, "normal");
    glEnableVertexAttribArray(color_prop);
    glVertexAttribPointer(color_prop, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), NULL);

    glGenBuffers(1, &renderable->uv_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, renderable->uv_buffer);
    glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices*sizeof(vec2), mesh->uvs, GL_STATIC_DRAW);

    const GLint uv_prop = glGetAttribLocation(renderable->shader_handle, "uv");
    glEnableVertexAttribArray(uv_prop);
    glVertexAttribPointer(uv_prop, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), NULL);
}

static void delete_renderable(Renderable *renderable)
{
    glBindVertexArray(0);
    glDeleteBuffers(1, &renderable->position_buffer);
    glDeleteBuffers(1, &renderable->normal_buffer);
    glDeleteBuffers(1, &renderable->uv_buffer);
    glDeleteVertexArrays(1, &renderable->vao);
}

static Renderable *get_renderable(HashTable *renderables, HashCache *resources, const char *mesh_path, const char *material_path)
{
    Mesh *mesh = hashcache_load(resources, mesh_path);
    if (!mesh) return NULL;

    Material *material = hashcache_load(resources, material_path);
    if (!material) return NULL;

    Shader *shader = hashcache_load(resources, material->shader);
    if (!shader) return NULL;

    char hash_key[1024];
    strcpy(hash_key, mesh_path);
    strcat(hash_key, material_path);

    Renderable *renderable = hashtable_at(renderables, hash_key);
    if (renderable) return renderable;

    Renderable new_renderable;
    load_renderable(&new_renderable, mesh, material, shader);
    return hashtable_set_copy(renderables, hash_key, &new_renderable);
}

RenderSystem *render_sys_new( HashCache *resources )
{
    RenderSystem *sys = malloc(sizeof(RenderSystem));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    sys->renderables = hashtable_empty(1024, sizeof(Renderable));

    return sys;
}

void render_sys_run( RenderSystem *sys, ECS *ecs, HashCache *resources, float aspect_ratio )
{
    Entity camera_entity;

    if (!ECS_FIND_FIRST_ENTITY_WITH_COMPONENT(Camera, ecs, &camera_entity)) return;

    ECS_GET_COMPONENT_DECL(Camera, camera, ecs, camera_entity);
    ECS_GET_COMPONENT_DECL(Transform, camera_transform, ecs, camera_entity);

    mat4 projection;
    glm_perspective(camera->fov, aspect_ratio, camera->near_clip, camera->far_clip, projection);
    projection[2][2] *= -1.f; // Use left-handed coordinates
    projection[2][3] *= -1.f;

    mat4 view;
    glm_mat4_inv( camera_transform->worldMatrix_, view );

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    size_t num_renderers;
    Entity *renderers = ECS_FIND_ALL_ENTITIES_WITH_COMPONENT_ALLOC(MeshRenderer, ecs, &num_renderers);

    for (int i = 0; i < num_renderers; ++i)
    {
        ECS_GET_COMPONENT_DECL(Transform, renderer_transform, ecs, renderers[i]);
        ECS_GET_COMPONENT_DECL(MeshRenderer, renderer_comp, ecs, renderers[i]);

        const Renderable *renderable = get_renderable(&sys->renderables, resources, renderer_comp->mesh, renderer_comp->material);

        if (!renderable) continue;

        glBindVertexArray(renderable->vao);
        glUseProgram(renderable->shader_handle);

        glUniformMatrix4fv(glGetUniformLocation(renderable->shader_handle, "view"), 1, GL_FALSE, view);
        glUniformMatrix4fv(glGetUniformLocation(renderable->shader_handle, "projection"), 1, GL_FALSE, projection);
        glUniformMatrix4fv(glGetUniformLocation(renderable->shader_handle, "model"), 1, GL_FALSE, renderer_transform->worldMatrix_);

        for (int i = 0; i < renderable->mesh->num_submeshes; ++i)
        {
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, texture_get_handle( hashcache_load( resources, renderable->material->submaterials[i].tex ) ) );
            glUniform1i( glGetUniformLocation( renderable->shader_handle, "tex" ), 0 );
            glDrawElements( GL_TRIANGLES, renderable->mesh->submeshes[i].num_indices, GL_UNSIGNED_SHORT, renderable->mesh->submeshes[i].indices );
        }
    }

    free(renderers);
}

void render_sys_delete(RenderSystem *sys)
{
    if (!sys) return;

    // TODO delete renderables

    free(sys);
}

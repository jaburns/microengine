#include "collision_sys.h"

#include "../component_defs.h"
#include "../geometry.h"

struct CollisionSystem
{
    Vec triangles;
};

CollisionSystem *collision_sys_new( void )
{
    return NULL;
}

void collision_sys_run( CollisionSystem *sys, ECS *ecs )
{
    // TODO find MeshColliders and cache their triangles in world space.
    // provide a raycast method which calls geometry_triangle_intersects_line_seg 
}

void collision_sys_delete( CollisionSystem *sys )
{
}

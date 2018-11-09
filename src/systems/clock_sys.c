#include "clock_sys.h"

#include "../components.h"
#include <ns_clock.h>

struct ClockSystem
{
    uint64_t first_clock;
    uint64_t last_clock;
};

ClockSystem *clock_sys_new( void )
{
    ClockSystem *sys = malloc( sizeof( ClockSystem ) );
    sys->first_clock = ns_clock();
    sys->last_clock = sys->first_clock;
    return sys;
}

void clock_sys_run( ClockSystem *sys, ECS *ecs, bool reset_clock )
{
    uint64_t now = ns_clock();
    float delta_millis = (float)(now - sys->last_clock) * 1e-6f;
    sys->last_clock = now;

    if( reset_clock ) sys->first_clock = now;

    Entity clock_entity;
    if( !ECS_FIND_FIRST_ENTITY_WITH_COMPONENT( ClockInfo, ecs, &clock_entity ) )
    {
        clock_entity = ecs_create_entity( ecs );
        ECS_ADD_COMPONENT_DEFAULT_DECL( ClockInfo, clock_, ecs, clock_entity );
    }

    ECS_GET_COMPONENT_DECL( ClockInfo, clock, ecs, clock_entity );

    clock->delta_millis = delta_millis;
    clock->millis_since_start = (float)(now - sys->first_clock) * 1e-6f;
}

void clock_sys_delete( ClockSystem *sys )
{
    if( sys ) free( sys );
}

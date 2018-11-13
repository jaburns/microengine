#include "clock_sys.h"

#include "../component_defs.h"
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

    ECS_ENSURE_SINGLETON_DECL( ClockInfo, ecs, clock );
    clock->delta_millis = delta_millis;
    clock->millis_since_start = (float)(now - sys->first_clock) * 1e-6f;
}

void clock_sys_delete( ClockSystem *sys )
{
    if( sys ) free( sys );
}

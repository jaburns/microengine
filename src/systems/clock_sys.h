#pragma once

#include "../containers/ecs.h"

typedef struct ClockSystem ClockSystem;

extern ClockSystem *clock_sys_new( void );
extern void clock_sys_run( ClockSystem *sys, ECS *ecs, bool reset_clock );
extern void clock_sys_delete( ClockSystem *sys );

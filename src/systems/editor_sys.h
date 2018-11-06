#pragma once

#include "../containers/ecs.h"
#include "../shell.h"

typedef struct EditorSystem EditorSystem;

extern EditorSystem *editor_sys_new( void );
extern ECS *editor_sys_run( EditorSystem *sys, ECS *ecs );
extern void editor_sys_delete( EditorSystem *sys );
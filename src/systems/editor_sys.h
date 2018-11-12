#pragma once

#include "../containers/ecs.h"
#include "../shell.h"

typedef struct EditorSystem EditorSystem;

typedef struct EditorSystemUpdateResult
{
    ECS *new_ecs;
    bool in_play_mode;
    bool in_game_view;
}
EditorSystemUpdateResult;

extern EditorSystem *editor_sys_new( void );
extern EditorSystemUpdateResult editor_sys_run( EditorSystem *sys, ECS *ecs );
extern void editor_sys_delete( EditorSystem *sys );
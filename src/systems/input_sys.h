#pragma once

#include "../containers/ecs.h"
#include "../shell.h"

typedef struct InputSystem InputSystem;

extern InputSystem *input_sys_new( ShellContext *shell );
extern void input_sys_run( InputSystem *sys, ECS *ecs );
extern void input_sys_delete( InputSystem *sys );

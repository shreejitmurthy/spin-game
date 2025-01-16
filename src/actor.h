#pragma once

#include <cglm/cglm.h>
#include <math.h>

#include "camera.h"

#include <stdarg.h>

typedef enum state_t {
    IDLE,
    ATTACK,
    MOVE,
} state_t;

// Just a guy lowkey
typedef struct actor_t {
    mat4 u_model;
    vec3 position;
    float angle;
    state_t state;
    const char* label;
    bool updated;
} actor_t;

actor_t create_actor(const char* label);
inline void update_actor(actor_t* actor);
void actor_lookat(actor_t* actor, vec3 position, vec3 scale);
void update_actors(actor_t* first, ...);
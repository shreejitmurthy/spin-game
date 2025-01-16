#include "actor.h"

#include <log/log.h>

actor_t create_actor(const char* label) {
    actor_t actor;
    actor.label = label;
    actor.angle = 0.f;
    actor.state = IDLE;
    glm_mat4_identity(actor.u_model);
    
    return actor;
}

void update_actor(actor_t* actor) {
    if (actor->updated) glm_translate(actor->u_model, actor->position);
}

// Must translate first, spent 1 hour trying to find this solution
void actor_lookat(actor_t* actor, vec3 position, vec3 scale) {
    float dx = actor->position[0] - position[0];
    float dz = actor->position[2] - position[2];

    actor->angle = atan2f(dx, dz);

    glm_mat4_identity(actor->u_model);
    glm_translate(actor->u_model, actor->position);
    actor->updated = true;
    glm_scale(actor->u_model, scale);
    glm_rotate_y(actor->u_model, actor->angle, actor->u_model);
}

void update_actors(actor_t* first, ...) {
    if (first == NULL) return;

    update_actor(first);

    va_list args;
    va_start(args, first);

    while (1) {
        actor_t *actor = va_arg(args, actor_t *);
        if (actor == NULL) {
            break;  // Sentinel reached
        }
        update_actor(actor);
    }

    va_end(args);
}

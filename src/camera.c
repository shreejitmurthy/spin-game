#include "camera.h"

#include <SDL3/SDL_keycode.h>
#include "keyboard.h"

camera_t camera_init(vec3 position, vec3 target, vec3 up) {
    camera_t camera;
    glm_vec3_copy(position, camera.position);
    glm_vec3_copy(target, camera.target);
    glm_vec3_copy(up, camera.up);
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera.front);
    glm_lookat(camera.position, camera.target, camera.up, camera.view);
    return camera;
}

void camera_move(camera_t* camera, vec3 movement, float delta_time) {
    vec3 delta;
    glm_vec3_scale(movement, camera->speed * delta_time, delta);
    glm_vec3_add(camera->position, delta, camera->position);
    glm_vec3_add(camera->target, delta, camera->target);
}

void camera_handle_input(camera_t* camera, controls_t controls, float delta_time) {
    vec3 forward, right, movement = {0.0f, 0.0f, 0.0f};
    glm_vec3_sub(camera->target, camera->position, forward);
    glm_vec3_normalize(forward);
    glm_vec3_crossn(forward, camera->up, right);

    if (action_key_down(controls.forward)) glm_vec3_add(movement, forward, movement);
    if (action_key_down(controls.back))    glm_vec3_sub(movement, forward, movement);
    if (action_key_down(controls.left))    glm_vec3_sub(movement, right, movement);
    if (action_key_down(controls.right))   glm_vec3_add(movement, right, movement);

    if (!glm_vec3_eqv(movement, (vec3){0.0f, 0.0f, 0.0f})) {
        glm_vec3_normalize(movement);
    }

    camera_move(camera, movement, delta_time);
}
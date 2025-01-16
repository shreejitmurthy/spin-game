#include "camera.h"
#include "keyboard.h"

#include <log/log.h>

camera_t init_camera(vec3 position, vec3 target, vec3 up) {
    camera_t camera;
    glm_vec3_copy(position, camera.position);
    glm_vec3_copy(target, camera.target);
    glm_vec3_copy(up, camera.up);
    camera.yaw = -90.0f;
    camera.pitch = 0.0f;

    glm_vec3_copy((vec3){0.0f, 0.0f, 1.0f}, camera.front);
    glm_lookat(camera.position, camera.target, camera.up, camera.view);
    return camera;
}

void move_camera(camera_t* camera, vec3 movement, float delta_time) {
    vec3 delta;
    glm_vec3_scale(movement, camera->speed * delta_time, delta);

    if (!camera->free_cam) delta[1] = 0.f;  // restrict movement in y-dir unless free cam is enabled

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

    move_camera(camera, movement, delta_time);
}

void camera_handle_mouse(camera_t *camera, float x_rel, float y_rel) {
    x_rel = glm_clamp(x_rel, -500.f, 500.f);
    y_rel = glm_clamp(y_rel, -500.f, 500.f);

    camera->yaw += x_rel * camera->sensitivity;
    // need to invert the y value
    camera->pitch -= y_rel * camera->sensitivity;

    if (camera->pitch > 89.0f)
        camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;

    vec3 front;
    front[0] = cosf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    front[1] = sinf(glm_rad(camera->pitch));
    front[2] = sinf(glm_rad(camera->yaw)) * cosf(glm_rad(camera->pitch));
    glm_vec3_normalize_to(front, camera->front);

    glm_vec3_add(camera->position, camera->front, camera->target);
    glm_lookat(camera->position, camera->target, camera->up, camera->view);
}


#pragma once

#include <cglm/cglm.h>
#include <stdbool.h>

#include "controls.h"

typedef struct camera_t {
    vec3 position;
    vec3 target;
    vec3 up;
    vec3 front;
    mat4 view;
    mat4 projection;
    float speed;
    float yaw;
    float pitch;
    float sensitivity;
    bool free_cam;
} camera_t;

camera_t init_camera(vec3 position, vec3 target, vec3 up);
void camera_handle_input(camera_t* camera, controls_t controls, float delta_time);
void camera_handle_mouse(camera_t* camera, float x_rel, float y_rel);
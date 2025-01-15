#pragma once

#include <cglm/cglm.h>

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
} camera_t;

camera_t camera_init(vec3 position, vec3 target, vec3 up);
void camera_move(camera_t* camera, vec3 movement, float delta_time);
void camera_handle_input(camera_t* camera, controls_t controls, float delta_time);
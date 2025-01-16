#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

#define SOKOL_TIME_IMPL
#include <sokol_time.h>
#include <log/log.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glad/glad.h>
#include <cglm/cglm.h>

#include "camera.h"
#include "keyboard.h"
#include "controls.h"
#include "shader.h"
#include "texture.h"
#include "actor.h"

#include <stb_image.h>

#include "enemy.img.h"

uint64_t last_time = 0;

void key_bindings(controls_t *controls) {
    controls->forward = (action_t){{SDLK_W}, 2, 0};
    controls->back = (action_t){{SDLK_S}, 2, 0};
    controls->left = (action_t){{SDLK_A}, 2, 0};
    controls->right = (action_t){{SDLK_D}, 2, 0};
}

int main() {
    stbi_set_flip_vertically_on_load(true);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_error("SDL initialization failed");
        return -1;
    }

    controls_t controls;
    init_controls(&controls, key_bindings);

    vec3 global_scale = {0.5f, 0.5f, 0.5f};

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    const uint16_t SCR_WIDTH = 1280;
    const uint16_t SCR_HEIGHT = 720;

    stm_setup();

    SDL_Window* window = SDL_CreateWindow("lazy suzan", SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        log_error("Window creation failed");
        return -1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        log_error("Failed to initialize GLAD");
        return -1;
    }

    SDL_GL_SetSwapInterval(1);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    camera_t camera = init_camera((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    camera.speed = 50.0f;
    camera.sensitivity = 0.08f;
    glm_perspective(glm_rad(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f, camera.projection);

    GLuint shd = load_shader("../shaders/texture.vert", "../shaders/texture.frag");
    texture_t enemy_texture = load_texture_raw(enemy_data, ENEMY_FRAME_WIDTH, ENEMY_FRAME_HEIGHT);
    actor_t enemy = create_actor("enemy");
    glm_vec3_copy((vec3){0.f, 0.f, 0.f}, enemy.position);

    actor_t enemy1 = create_actor("enemy1");
    glm_vec3_copy((vec3){50.f, 0.f, 0.f}, enemy1.position);

    bool open = true;
    SDL_Event event;

    float delta_time;

    SDL_SetWindowRelativeMouseMode(window, true);

    while (open) {
        delta_time = (float)stm_sec(stm_laptime(&last_time));

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                open = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                keyboard_process(event);
                if (event.key.key == SDLK_ESCAPE) {
                    open = false;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                camera_handle_mouse(&camera, event.motion.xrel, event.motion.yrel);
            }
        }

        camera_handle_input(&camera, controls, delta_time);

        glm_lookat(camera.position, camera.target, camera.up, camera.view);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        use_shader(shd);

        shader_set_mat4(shd, "u_view", camera.view);
        shader_set_mat4(shd, "u_projection", camera.projection);
        shader_set_vec4(shd, "u_tint", (vec4){1.f, 1.f, 1.f, 1.f});

        update_actors(&enemy, &enemy1, NULL);

        actor_lookat(&enemy, camera.position, global_scale);
        shader_set_mat4(shd, "u_model", enemy.u_model);
        draw_texture(enemy_texture);

        actor_lookat(&enemy1, camera.position, global_scale);
        shader_set_mat4(shd, "u_model", enemy1.u_model);
        draw_texture(enemy_texture);

        SDL_GL_SwapWindow(window);
    }

    delete_texture(enemy_texture);
    glDeleteProgram(shd);

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

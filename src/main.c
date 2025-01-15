#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define SOKOL_TIME_IMPL
#include <sokol_time.h>
#include <log/log.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <cglm/cglm.h>

#include "camera.h"
#include "keyboard.h"
#include "controls.h"
#include "shader.h"

uint64_t last_time = 0;

void key_bindings(controls_t *controls) {
    controls->forward = (action_t){{SDLK_W}, 2, 0};
    controls->back = (action_t){{SDLK_S}, 2, 0};
    controls->left = (action_t){{SDLK_A}, 2, 0};
    controls->right = (action_t){{SDLK_D}, 2, 0};
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_error("SDL initialization failed");
        return -1;
    }

    controls_t controls;
    init_controls(&controls, key_bindings);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    const uint16_t SCR_WIDTH = 800;
    const uint16_t SCR_HEIGHT = 600;

    stm_setup();

    SDL_Window* window = SDL_CreateWindow("OpenGL Window", SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window) {
        log_error("Window creation failed");
        return -1;
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        log_error("Failed to initialize GLAD");
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    camera_t camera = camera_init((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    camera.speed = 5.0f;
    camera.sensitivity = 0.1f;

    glm_perspective(glm_rad(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, camera.projection);

    GLuint shd = load_shader("../shaders/quad.vert", "../shaders/quad.frag");

    float vertices[] = {
        -0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f, 1.0f,
    };

    uint16_t indices[] = {
        0, 1, 2,
        0, 2, 3,
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    SDL_GL_SetSwapInterval(1);

    bool open = true;
    SDL_Event event;

    mat4 u_model;

    glm_mat4_identity(u_model);
    float angle = 0.0f;

    float delta_time;

    while (open) {
        delta_time = (float)stm_sec(stm_laptime(&last_time));

        angle += 0.1f;
        glm_mat4_identity(u_model);
        glm_rotate_y(u_model, angle, u_model);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                open = false;
            } else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                keyboard_process(event);
                if (event.key.key == SDLK_ESCAPE) {
                    open = false;
                }
            }
        }

        camera_handle_input(&camera, controls, delta_time);

        glm_lookat(camera.position, camera.target, camera.up, camera.view);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader_use(shd);
        shader_set_mat4(shd, "u_model", u_model);
        shader_set_mat4(shd, "u_view", camera.view);
        shader_set_mat4(shd, "u_projection", camera.projection);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shd);

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

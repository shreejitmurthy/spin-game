#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

#include "enemy.img.h"

typedef struct texture_t {
    GLuint id;
    GLuint vbo, vao, ebo;
    uint32_t width, height, nr_channels;
} texture_t;

typedef struct quad_t {
    float x, y, w, h;
} quad_t;

texture_t load_texture_raw(const uint32_t img_data[], uint32_t width, uint32_t height) {
    texture_t texture;
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const unsigned char* data = (const unsigned char*)img_data;
    texture.nr_channels = 4;

    if (data) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        log_error("Failed to read pixel data");
    }

    texture.width = width;
    texture.height = height;

    quad_t quad = (quad_t){0, 0, (float)width, (float)height};

    float u0 = quad.x / width;
    float v0 = quad.y / height;
    float u1 = (quad.x + quad.w) / width;
    float v1 = (quad.y + quad.h) / height;

    float halfWidth  = quad.w / 2.0f;
    float halfHeight = quad.h / 2.0f;

    float vertices[32] = {
        /*   positions         tx coords        colors     */
         halfWidth,  halfHeight,  0.0f,   u1, v1,    1.0f, 0.0f, 0.0f,  // top right
         halfWidth, -halfHeight,  0.0f,   u1, v0,    0.0f, 1.0f, 0.0f,  // bottom right
        -halfWidth, -halfHeight,  0.0f,   u0, v0,    0.0f, 0.0f, 1.0f,  // bottom left
        -halfWidth,  halfHeight,  0.0f,   u0, v1,    1.0f, 1.0f, 0.0f   // top left
    };

    unsigned int indices[6] = {
            0, 1, 3,  // first triangle
            1, 2, 3   // second triangle
    };

    glGenVertexArrays(1, &texture.vao);
    glGenBuffers(1, &texture.vbo);
    glGenBuffers(1, &texture.ebo);

    glBindVertexArray(texture.vao);

    glBindBuffer(GL_ARRAY_BUFFER, texture.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);           // position (3 floats)
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // texture coords (2 floats)
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); // color (3 floats)
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return texture;
}

texture_t load_texture(const char* filename) {
    texture_t texture;
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const unsigned char* data = stbi_load(filename, &texture.width, &texture.height, &texture.nr_channels, 4);

    if (data) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        log_error("Failed to read pixel data");
    }

    int width = texture.width;
    int height = texture.height;

    quad_t quad = (quad_t){0, 0, (float)width, (float)height};

    float u0 = quad.x / width;
    float v0 = quad.y / height;
    float u1 = (quad.x + quad.w) / width;
    float v1 = (quad.y + quad.h) / height;

    float halfWidth  = quad.w / 2.0f;
    float halfHeight = quad.h / 2.0f;

    float vertices[32] = {
        /*   positions         tx coords        colors     */
         halfWidth,  halfHeight,  0.0f,   u1, v1,    1.0f, 0.0f, 0.0f,  // top right
         halfWidth, -halfHeight,  0.0f,   u1, v0,    0.0f, 1.0f, 0.0f,  // bottom right
        -halfWidth, -halfHeight,  0.0f,   u0, v0,    0.0f, 0.0f, 1.0f,  // bottom left
        -halfWidth,  halfHeight,  0.0f,   u0, v1,    1.0f, 1.0f, 0.0f   // top left
    };

    unsigned int indices[6] = {
            0, 1, 3,  // first triangle
            1, 2, 3   // second triangle
    };

    glGenVertexArrays(1, &texture.vao);
    glGenBuffers(1, &texture.vbo);
    glGenBuffers(1, &texture.ebo);

    glBindVertexArray(texture.vao);

    glBindBuffer(GL_ARRAY_BUFFER, texture.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);           // position (3 floats)
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // texture coords (2 floats)
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); // color (3 floats)
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void draw_texture(texture_t texture) {
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glBindVertexArray(texture.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void delete_texture(texture_t texture) {
    glDeleteVertexArrays(1, &texture.vao);
    glDeleteBuffers(1, &texture.vbo);
    glDeleteBuffers(1, &texture.ebo);
}

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

    glEnable(GL_BLEND | GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    camera_t camera = camera_init((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    camera.speed = 100.0f;
    camera.sensitivity = 0.08f;

    // glm_perspective(glm_rad(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f, camera.projection);
    glm_perspective(glm_rad(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f, camera.projection);

    GLuint shd = load_shader("../shaders/texture.vert", "../shaders/texture.frag");

    texture_t enemy = load_texture_raw(enemy_data, ENEMY_FRAME_WIDTH, ENEMY_FRAME_HEIGHT);

    bool open = true;
    SDL_Event event;

    mat4 u_model;
    glm_mat4_identity(u_model);
    float angle = 0.0f;
    
    float delta_time;

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    SDL_SetWindowRelativeMouseMode(window, true);

    while (open) {
        delta_time = (float)stm_sec(stm_laptime(&last_time));

        // angle += 0.1f;
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
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                log_info("Mouse rel coords: (%.2f, %.2f)", event.motion.xrel, event.motion.yrel);
                camera_handle_mouse(&camera, event.motion.xrel, event.motion.yrel);

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
        shader_set_vec4(shd, "u_tint", (vec4){1.f, 1.f, 1.f, 1.f});

        draw_texture(enemy);

        SDL_GL_SwapWindow(window);
    }

    delete_texture(enemy);
    
    glDeleteProgram(shd);

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

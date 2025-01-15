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

uint64_t last_time = 0;

char* read_file_into_char(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        log_error("failed to open file");
        return NULL;
    }
 
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
 
    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL) {
        log_error("memory alloc failed");
        fclose(file);
        return NULL;
    }
 
    size_t bytes_read = fread(buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        log_error("error reading file");
        free(buffer);
        fclose(file);
        return NULL;
    }
 
    buffer[bytes_read] = '\0';
 
    fclose(file);
    return buffer;
}

void check_compile_error_shd(GLuint shader, const char* type) {
    int success;
    char infoLog[1024];
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            log_error("Shader compilation error of type: %s, %s", type, infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            log_error("Shader program linking error of type: %s, %s", type, infoLog);
        }
    }
}
 
GLuint load_shader(const char* vertex_path, const char* fragment_path) {
    GLuint id;
    const char* vertex_code = read_file_into_char(vertex_path);
    const char* fragment_code = read_file_into_char(fragment_path);
    uint32_t vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_code, NULL);
    glCompileShader(vertex);
    check_compile_error_shd(vertex, "VERTEX");
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_code, NULL);
    glCompileShader(fragment);
    check_compile_error_shd(fragment, "FRAGMENT");
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    check_compile_error_shd(id, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);
 
    return id;
}
 
void shader_use(GLuint shd) {
    glUseProgram(shd);
}
 
void shader_set_mat4(GLuint shd, const char* name, mat4 mat) {
    glUniformMatrix4fv(glGetUniformLocation(shd, name), 1, GL_FALSE, (float*)mat);
}
 
void shader_set_vec4(GLuint shd, const char* name, vec4 vec) {
    glUniform4f(glGetUniformLocation(shd, name), vec[0], vec[1], vec[2], vec[3]);
}

typedef struct {
    mat4 u_model;
    mat4 u_projection;
    mat4 u_view;
} transform_params_t;

typedef struct {
    vec4 tint;
} colour_params_t;

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_error("SDL initialization failed");
        return -1;
    }

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

    GLuint shader_program = load_shader("../shaders/quad.vert", "../shaders/quad.frag");

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

    transform_params_t vs_params;
    glm_mat4_identity(vs_params.u_model);
    glm_mat4_copy(camera.view, vs_params.u_view);
    glm_mat4_copy(camera.projection, vs_params.u_projection);
    float angle = 0.0f;

    float delta_time;

    while (open) {
        delta_time = (float)stm_sec(stm_laptime(&last_time));

        angle += 0.1f;
        glm_mat4_identity(vs_params.u_model);
        glm_rotate_y(vs_params.u_model, angle, vs_params.u_model);
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

        camera_handle_input(&camera, delta_time);
        glm_mat4_copy(camera.view, vs_params.u_view);
        glm_mat4_copy(camera.projection, vs_params.u_projection);

        glm_lookat(camera.position, camera.target, camera.up, camera.view);
        glm_mat4_copy(camera.view, vs_params.u_view);
        glm_mat4_copy(camera.projection, vs_params.u_projection);
        // glm_lookat(camera.position, camera.target, camera.up, camera.view);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader_use(shader_program);
        shader_set_mat4(shader_program, "u_model", vs_params.u_model);
        shader_set_mat4(shader_program, "u_view", camera.view);
        shader_set_mat4(shader_program, "u_projection", camera.projection);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shader_program);

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

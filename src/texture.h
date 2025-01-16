#pragma once

#include <glad/glad.h>
#include <stdio.h>

typedef struct texture_t {
    GLuint id;
    GLuint vbo, vao, ebo;
    uint32_t width, height, nr_channels;
} texture_t;

typedef struct quad_t {
    float x, y, w, h;
} quad_t;

texture_t load_texture_raw(const uint32_t *img_data, uint32_t width, uint32_t height);
texture_t load_texture(const char* filename);
void draw_texture(texture_t texture);
void delete_texture(texture_t texture);
#include "texture.h"

#include <log/log.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void setup_texture_parameters() {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void load_texture_data(const unsigned char* data, uint32_t width, uint32_t height) {
    if (data) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        log_error("Failed to read pixel data");
    }
}

void setup_buffers(texture_t *texture, float *vertices, unsigned int *indices) {
    glGenVertexArrays(1, &texture->vao);
    glGenBuffers(1, &texture->vbo);
    glGenBuffers(1, &texture->ebo);

    glBindVertexArray(texture->vao);

    glBindBuffer(GL_ARRAY_BUFFER, texture->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 32, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texture->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 6, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);           // position (3 floats)
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // texture coords (2 floats)
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); // color (3 floats)
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void generate_quad_data(uint32_t width, uint32_t height, float *vertices, unsigned int *indices) {
    quad_t quad = {0, 0, (float)width, (float)height};

    float u0 = quad.x / width;
    float v0 = quad.y / height;
    float u1 = (quad.x + quad.w) / width;
    float v1 = (quad.y + quad.h) / height;

    float halfWidth = quad.w / 2.0f;
    float halfHeight = quad.h / 2.0f;

    float temp_vertices[32] = {
        /*      positions           tx coords             colors     */
         halfWidth,  halfHeight,  0.0f,   u1, v1,    1.0f, 0.0f, 0.0f,  // top right
         halfWidth, -halfHeight,  0.0f,   u1, v0,    0.0f, 1.0f, 0.0f,  // bottom right
        -halfWidth, -halfHeight,  0.0f,   u0, v0,    0.0f, 0.0f, 1.0f,  // bottom left
        -halfWidth,  halfHeight,  0.0f,   u0, v1,    1.0f, 1.0f, 0.0f   // top left
    };

    unsigned int temp_indices[6] = {
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    };

    memcpy(vertices, temp_vertices, sizeof(temp_vertices));
    memcpy(indices, temp_indices, sizeof(temp_indices));
}

texture_t load_texture_raw(const uint32_t *img_data, uint32_t width, uint32_t height) {
    texture_t texture;
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    setup_texture_parameters();

    const unsigned char* data = (const unsigned char*)img_data;
    texture.nr_channels = 4;

    load_texture_data(data, width, height);

    texture.width = width;
    texture.height = height;

    float vertices[32];
    unsigned int indices[6];
    generate_quad_data(width, height, vertices, indices);

    setup_buffers(&texture, vertices, indices);

    return texture;
}

texture_t load_texture(const char* filename) {
    texture_t texture;
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    setup_texture_parameters();

    int width, height, nr_channels;
    const unsigned char* data = stbi_load(filename, &width, &height, &nr_channels, 4);

    texture.width = width;
    texture.height = height;
    texture.nr_channels = nr_channels;

    load_texture_data(data, width, height);

    float vertices[32];
    unsigned int indices[6];
    generate_quad_data(width, height, vertices, indices);

    setup_buffers(&texture, vertices, indices);

    return texture;
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

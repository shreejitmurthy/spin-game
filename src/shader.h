#pragma once

#include <glad/glad.h>
#include <string.h>
#include <log/log.h>

#include <cglm/cglm.h>

void shader_check_compile_err(GLuint shader, const char* type);

GLuint load_shader(const char* vertex_path, const char* fragment_path);
void use_shader(GLuint shd);
void shader_set_mat4(GLuint shd, const char* name, mat4 mat);
void shader_set_vec4(GLuint shd, const char* name, vec4 vec);
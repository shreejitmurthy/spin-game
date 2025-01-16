#include "shader.h"

#include "util.h"

void shader_check_compile_err(GLuint shader, const char* type) {
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
    shader_check_compile_err(vertex, "VERTEX");
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_code, NULL);
    glCompileShader(fragment);
    shader_check_compile_err(fragment, "FRAGMENT");
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    shader_check_compile_err(id, "PROGRAM");
    glDeleteShader(vertex);
    glDeleteShader(fragment);
 
    return id;
}

void use_shader(GLuint shd) {
    glUseProgram(shd);
}

void shader_set_mat4(GLuint shd, const char* name, mat4 mat) {
    glUniformMatrix4fv(glGetUniformLocation(shd, name), 1, GL_FALSE, (float*)mat);
}
 
void shader_set_vec4(GLuint shd, const char* name, vec4 vec) {
    glUniform4f(glGetUniformLocation(shd, name), vec[0], vec[1], vec[2], vec[3]);
}

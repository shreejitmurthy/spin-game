#version 410 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;
layout (location = 2) in vec3 color;

out vec2 TexCoords;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;

void main() {
    TexCoords = texCoords;
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0);
}

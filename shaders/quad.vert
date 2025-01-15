#version 410

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color0;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;

out vec4 color;

void main() {
    gl_Position = u_projection * u_view * u_model * position;
    color = color0;
}

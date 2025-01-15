#version 410 core

layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 u_model;
uniform mat4 u_projection;
uniform mat4 u_view;

void main() {
	TexCoords = vertex.zw;
	gl_Position = u_projection * u_view * u_model * vec4(vertex.xy, 0.0, 1.0);
}

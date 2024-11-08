#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelMatrix;

out vec2 vTexcoords;
out vec3 vNormal;

void main() {
	vTexcoords = uv;
	vNormal = normal;
	gl_Position = modelMatrix * vec4(position, 1.0f);
}

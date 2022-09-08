#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 DepthModelViewProjectionMatrix;
uniform mat4 ModelMatrix;

out vData {
    vec2 UV;
    vec4 position_depth;
} vert;

void main() {
    vert.UV = uv;
    vert.position_depth = DepthModelViewProjectionMatrix * vec4(position, 1);
	vert.position_depth.xyz = vert.position_depth.xyz * 0.5f + 0.5f;
    gl_Position = ModelMatrix * vec4(position,1);
}
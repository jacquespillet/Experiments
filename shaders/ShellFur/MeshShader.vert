 //attribs
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;

out vec2 fragUv;
out vec2 colorfragUv;
out vec3 fragNormal;

uniform mat4 shadowMapViewProjectionMatrix;

uniform float layerFloat;
uniform float length;
uniform float tiling;

//main
void main()
{
	vec3 hairPos = position.xyz + normal * layerFloat * length;
	fragNormal = (modelMatrix * vec4(normal, 1.0f)).xyz;
	
	gl_Position =  modelViewProjectionMatrix * vec4(hairPos,1);
	colorfragUv = uv * 3;
	fragUv = uv * tiling;
}
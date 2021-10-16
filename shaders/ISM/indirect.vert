 //attribs
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 viewProjectionMatrix;
uniform int index;

uniform sampler2D randomTexture;
uniform sampler2D rsmPositionTexture;
uniform sampler2D rsmFluxTexture;

out vec2 fragUv;

//main
void main()
{
	vec2 samplePosition = texelFetch(randomTexture, ivec2(index, 0), 0).xy;
	vec3 worldPosition = texture2D(rsmPositionTexture, samplePosition).xyz;

	float scale = 0.1;
	vec3 vertexPosition = position;
	vec3 scaledVertexPosition = scale * position;
	vec3 worldScaledVertexPosition = scaledVertexPosition + worldPosition;

	gl_Position =  viewProjectionMatrix * vec4(worldScaledVertexPosition, 1.0);
	fragUv = uv;
}
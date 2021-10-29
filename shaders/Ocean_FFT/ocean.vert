 //attribs
#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelViewProjectionMatrix;

uniform vec3 cameraPosition;
uniform vec3 lightDirection;

out vec3 fragWorldPos;
out vec3 fragNormal;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec2 fragUv;
out vec3 fragToEyeWorld;
out vec3 fragToLightWorld;
out vec3 fragToEyeTangent;
out vec4 depthMapUV;

uniform sampler2D displacementTexture;
uniform float size;

//main
void main()
{
	float halfSize = size/2;
	vec2 samplePosition = position.xz;
	samplePosition += vec2(halfSize, halfSize);
	samplePosition /= (size);
	

	float height = texture(displacementTexture, samplePosition).r;
	// float height = 0;
	
	vec3 newPosition = position+ vec3(0, height, 0);

	//Projected position
	gl_Position =  modelViewProjectionMatrix * vec4(newPosition,1);



	//World newPosition
	fragWorldPos = newPosition.xyz;

	//tangent space vectors
	fragNormal = normalize(normal);
	fragTangent = normalize(tangent);
	fragBitangent = normalize(bitangent);

	//Vector from fragment position to camerea
	fragToEyeWorld = cameraPosition - fragWorldPos; // Normalize in fragment shader or else it will be interpolated wrong
	fragToLightWorld = lightDirection; // Normalize in fragment shader or else it will be interpolated wrong

	//Vertex
	fragUv = samplePosition;
}
 //attribs
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

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


in vec3 eyePosition,eyeNormal,eyeLightPos;


uniform mat4 shadowMapViewProjectionMatrix;


//main
void main()
{
	//Projected position
	gl_Position =  modelViewProjectionMatrix * vec4(position,1);

	//World position
	fragWorldPos = (modelMatrix * vec4(position,1)).xyz;

	//Position on depth map, normalized on texture coords ([0-1, 0-1])
	depthMapUV = shadowMapViewProjectionMatrix * vec4(fragWorldPos, 1);
	depthMapUV.xyz = depthMapUV.xyz * 0.5 + 0.5;

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	//tangent space vectors
	fragNormal = normalize((normalMatrix * normal));
	fragTangent = normalize((normalMatrix * tangent));
	fragBitangent = normalize((normalMatrix * bitangent));

	//Vector from fragment position to camerea
	fragToEyeWorld = cameraPosition - fragWorldPos; // Normalize in fragment shader or else it will be interpolated wrong
	fragToLightWorld = lightDirection; // Normalize in fragment shader or else it will be interpolated wrong

	//Vertex
	fragUv = uv;
}
 //attribs
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;

uniform mat4 shadowMapViewProjectionMatrix;

uniform vec3 cameraPosition;

out vec3 fragNormal;
out vec3 fragWorldPos;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec2 fragUv;
out vec3 fragToEyeWorld;
out vec3 fragToLightWorld;
out vec3 fragToEyeTangent;
out vec4 depthMapUV;


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

	//tangent space vectors
	fragNormal = normalize((modelMatrix * vec4(normal,0)).xyz);
	fragTangent = normalize((modelMatrix * vec4(tangent,0)).xyz);
	fragBitangent = normalize((modelMatrix * vec4(bitangent,0)).xyz);

	//Vector from fragment position to camerea
	fragToEyeWorld = cameraPosition - fragWorldPos; // Normalize in fragment shader or else it will be interpolated wrong

	//Vertex
	fragUv = uv;
}
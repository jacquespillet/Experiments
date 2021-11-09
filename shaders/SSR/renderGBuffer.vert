 //attribs
#version 440
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;

out vec3 fragWorldPos;
out vec3 fragWorldNormal;
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

	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	//tangent space vectors
	fragWorldNormal = normalize((normalMatrix * normal));
	fragTangent = normalize((normalMatrix * tangent));
	fragBitangent = normalize((normalMatrix * bitangent));
	
	//Vertex
	fragUv = uv;
}
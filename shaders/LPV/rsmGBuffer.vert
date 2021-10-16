 //attribs
#version 440
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 shadowMapViewProjectionMatrix;

out vec3 fragWorldPosition;
out vec3 fragWorldNormal;
out vec2 fragUv;
out vec4 depthMapUV;

//main
void main()
{
    vec4 worldPosition =  modelMatrix * vec4(position.xyz, 1.0);
    vec4 worldNormal =  transpose(inverse(modelMatrix)) * vec4(normal.xyz, 0.0);
    fragWorldPosition = worldPosition.xyz;
    fragWorldNormal = normalize(worldNormal.xyz);
    
    fragUv = uv;

    
	//Position on depth map, normalized on texture coords ([0-1, 0-1])
	depthMapUV = shadowMapViewProjectionMatrix * vec4(fragWorldPosition, 1);
	depthMapUV.xyz = depthMapUV.xyz * 0.5 + 0.5;

    gl_Position = modelViewProjectionMatrix * vec4(position.xyz, 1.0);
}
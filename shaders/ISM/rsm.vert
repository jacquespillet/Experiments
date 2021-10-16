 //attribs
#version 440
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 modelViewProjectionMatrix;

out vec3 fragWorldPosition;
out vec3 fragWorldNormal;
out vec2 fragUv;

uniform vec3 mat_ambient;
uniform vec3 mat_diffuse;
uniform vec3 mat_specular;
uniform vec3 mat_emissive;
uniform vec3 mat_transparent;
uniform float mat_shininess;
uniform float mat_opacity;

//main
void main()
{
    vec4 worldPosition =  modelMatrix * vec4(position.xyz, 1.0);
    vec4 worldNormal =  transpose(inverse(modelMatrix)) * vec4(normal.xyz, 0.0);
    fragWorldPosition = worldPosition.xyz;
    fragWorldNormal = worldNormal.xyz;
    
    fragUv = uv;

    gl_Position = modelViewProjectionMatrix * vec4(position.xyz, 1.0);
}
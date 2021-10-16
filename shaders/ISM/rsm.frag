//inputs
#version 440
//output
layout(location = 0) out vec4 outputPosition; 
layout (location = 1) out vec4 outputNormal;
layout (location = 2) out vec4 outputFlux;
//main

uniform vec4 color;

in vec3 fragWorldPosition;
in vec3 fragWorldNormal;
in vec2 fragUv;


uniform sampler2D diffuseTexture;
uniform vec3 mat_diffuse;

void main()
{
    outputPosition = vec4(fragWorldPosition, 0);
    outputNormal = vec4(fragWorldNormal, 0);
    // outputFlux = vec4(texture(diffuseTexture, fragUv).rgb, 1);
    outputFlux = vec4(mat_diffuse, 1.0);
    // outputFlux = vec4(0,0,1, 1);
}
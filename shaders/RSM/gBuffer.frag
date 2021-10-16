//inputs
#version 440
//output
layout(location = 0) out vec4 outputPosition; 
layout (location = 1) out vec4 outputNormal;
layout (location = 2) out vec4 outputFlux;
// layout (location = 2) out vec4 gAlbedoSpec;
//main

uniform vec4 color;

in vec3 fragWorldPosition;
in vec3 fragWorldNormal;
in vec2 fragUv;


uniform sampler2D diffuseTexture;

void main()
{
    outputPosition = vec4(fragWorldPosition, 0);
    outputNormal = vec4(normalize(fragWorldNormal), 0);
    outputFlux = vec4(texture(diffuseTexture, fragUv).rgb, 1);
}
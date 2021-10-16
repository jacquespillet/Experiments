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
uniform vec3 lightDirection;

void main()
{
    outputPosition = vec4(fragWorldPosition, 0);
    outputNormal = vec4(fragWorldNormal, 0);
    vec3 diffuse = texture(diffuseTexture, fragUv).rgb;
    outputFlux = vec4(diffuse * clamp( dot( lightDirection, fragWorldNormal ),0.0,1.0 ), 1.0);
}
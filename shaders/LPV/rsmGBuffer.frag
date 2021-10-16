//inputs
#version 440
//output
layout(location = 0) out vec4 outputPosition; 
layout (location = 1) out vec4 outputNormal;
layout (location = 2) out vec4 outputColor;

in vec3 fragWorldPosition;
in vec3 fragWorldNormal;
in vec2 fragUv;
in vec4 depthMapUV;


uniform sampler2D diffuseTexture;


uniform sampler2DShadow rsmDepth;
uniform sampler2D rsmPosition;
uniform sampler2D rsmNormal;
uniform sampler2D rsmFlux;
uniform sampler2D randomTexture;

uniform float maxDistance;
uniform float intensity;
uniform int numSamples;
uniform int showDirect;
uniform int showIndirect;


void main()
{
    outputPosition = vec4(fragWorldPosition, 0);
    outputNormal = vec4(fragWorldNormal, 0);
    outputColor = vec4(1, 0, 0, 1);
}
//inputs
#version 430 core
//output
layout(location = 0) out vec4 outputColor; 
// layout(location = 1) out vec4 outputDepth; 
//main
#define PI 3.1415926535897932384626433832795

in vec2 fragUv;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;
uniform sampler2D postProcessedTexture;

void main()
{
    vec4 sampleDiffuse = texture(postProcessedTexture, fragUv);
    outputColor = vec4(sampleDiffuse.rgb, 1.0f);

}
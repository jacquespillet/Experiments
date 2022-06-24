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

void main()
{
    vec4 sampleDiffuse = texture(colorTexture, fragUv);
    // vec3 ambientColor =vec3(0.1) * diffuseColor;
    vec3 finalColor= sampleDiffuse.xyz;
    // vec3 finalColor= ambientColor;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));
    outputColor = vec4(finalColor, 1.0f);

}
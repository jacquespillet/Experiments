//inputs
#version 400 core
//output
layout(location = 0) out vec4 outputColor; 
//main
#define PI 3.1415926535897932384626433832795

in vec2 fragUv;
in vec2 colorfragUv;
in vec3 fragNormal;
 
uniform sampler2D furTexture;
uniform sampler2D furColor;

uniform float layerFloat;
uniform vec3 lightDirection;

void main()
{
    float intens = dot(fragNormal, lightDirection);
    float shadow = mix(0.0f,1.0f,layerFloat);

    vec4 furMaskColor = texture(furTexture, fragUv);
    vec4 furMask = furMaskColor.aaaa;
    vec4 furColor = texture(furColor, colorfragUv);

    if(furMaskColor.r <= layerFloat) discard;

    vec4 finalFurColor = furMask * furColor;
    finalFurColor.rgb *= shadow;
    finalFurColor.rgb *= intens;
    // furMask = vec4(1,0,0,1);
    float gamma = 2.2;
    vec4 finalColor = pow(finalFurColor, vec4(1.0/gamma));
    outputColor = finalColor;
}
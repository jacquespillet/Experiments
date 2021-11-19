#version 400 core
//output
layout(location = 0) out vec4 outputColor; 
//main
#define PI 3.1415926535897932384626433832795

in vec3 fragNormal;
in vec3 fragWorldPos;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
in vec3 fragToEyeWorld;
in vec3 fragToLightWorld;
in vec3 fragToEyeTangent;
in vec4 depthMapUV;

 
uniform sampler2D pageTableTexture;
uniform sampler2D physicalTexture;
uniform vec2 physicalTexturePageSize;
uniform vec2 numPages;

void main()
{

    //Sample diffuse texture
    //Sample global alpha

    float mipExp = 1; // alpha channel has mipmap-level
    // float mipExp = exp2(pageTableEntry.a); // alpha channel has mipmap-level
    vec4 pageTableEntry = texture(pageTableTexture, fragUv, 0) * 255.0;
    vec2 pageCoord = pageTableEntry.rg;
    vec2 withinPageCoord = fract(fragUv * numPages);
    // vec2 withinPageCoord = fragUv;
    // vec2 finalCoord =  (pageCoord + withinPageCoord) / numTiles);
    vec2 finalCoord = (pageCoord + withinPageCoord) / physicalTexturePageSize;
    
    vec3 finalColor = texture(physicalTexture, finalCoord).rgb;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));
    outputColor = vec4(finalColor, 1.0f);
}
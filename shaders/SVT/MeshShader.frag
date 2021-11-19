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
uniform int sampleMipMap;
uniform int numMipmaps;


float mipmapLevel(vec2 uv)  {
    vec2 dx = dFdx(uv * 512);
    vec2 dy = dFdy(uv * 512);
    float d = max(dot(dx, dx), dot(dy, dy));
    return 0.5 * log2(d);   // explanation: 0.5*log(x) = log(sqrt(x)) + mip_bias - readback_reduction_shift;
}

void main()
{

    //Sample diffuse texture
    //Sample global alpha


#if 0
    // float mipmap = textureLod(pageTableTexture, fragUv, 0).b * 255.0;
    // vec4 pageTableEntry = textureLod(pageTableTexture, fragUv, sampleMipMap) * 255.0;
    // float mip = floor(mipmapLevel(fragUv));
    // float mip = numMipmaps-1- mipmapLevel(fragUv);
    float mip = sampleMipMap;
    // float mip = fragUv.x * numMipmaps;
    vec4 pageTableEntry = textureLod(pageTableTexture, fragUv, mip) * 255.0;
    
    //NOT WORKING
    vec2 withinPageCoord =  fragUv * exp2(numMipmaps-1- mip);
    withinPageCoord = fract(withinPageCoord);

    vec2 finalCoord = pageTableEntry.rg + withinPageCoord;
    finalCoord /= physicalTexturePageSize;
    
#else
    // float bias = 6 - 0.5 + 0;
    // vec4 pageTableEntry = texture2D(pageTableTexture, fragUv, bias) * 255.0;
    // float mipmap = textureLod(pageTableTexture, fragUv, 0).b * 255.0;
    vec4 pageTableEntry = texture(pageTableTexture, fragUv) * 255.0;
    
    vec2 pageCoord = pageTableEntry.rg; // blue-green has x-y coordinates
    vec2 withinPageCoord = fract(fragUv * exp2(pageTableEntry.b));
    vec2 finalCoord = ((pageCoord + withinPageCoord) / physicalTexturePageSize);
#endif
  
    vec3 finalColor = texture(physicalTexture, finalCoord).rgb;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));
    outputColor = vec4(finalColor, 1.0f);
}
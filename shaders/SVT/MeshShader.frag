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
uniform float sampleMipMap;
uniform int numMipmaps;


float mipmapLevel(vec2 uv)  {
    vec2 dx = dFdx(uv * 512);
    vec2 dy = dFdy(uv * 512);
    float d = max(dot(dx, dx), dot(dy, dy));
    return 0.5 * log2(d);   // explanation: 0.5*log(x) = log(sqrt(x)) + mip_bias - readback_reduction_shift;
}

void main()
{
#if 1
    int maxMipmap = numMipmaps-1;
    //Problem :
    //We sample the page table, and the max resolution of the tile is not yet available.
    //This means we are redirected to a lower mip map, but we do not know which!
    //So we need to read which mip map this is in the b channel.

    // vec4 pageTableEntry = textureLod(pageTableTexture, fragUv, mip) * 255.0;
    
    float sampleMip = floor(mipmapLevel(fragUv));
    // float sampleMip = floor(sampleMipMap);
    // float sampleMip = 3;
    vec4 pageTableEntry = textureLod(pageTableTexture, fragUv, sampleMip) * 255.0;
    
    float mip = max(0, floor(maxMipmap - (pageTableEntry.b-0.5)));
    
    float texSize = exp2(mip);

    // float texSize = exp2(numMipmaps - 1 - floor(pageTableEntry.b));
    vec2 withinPageCoord =  fract(fragUv * texSize);
    // vec2 withinPageCoord =  fract(fragUv * exp2(numMipmaps-1-pageTableEntry.b));
    vec2 finalCoord = pageTableEntry.rg + withinPageCoord;
    finalCoord /= physicalTexturePageSize;

    outputColor = vec4(mip/maxMipmap, mip/maxMipmap, mip/maxMipmap, 1);
    
    // if(sampleMipMap==0) {
    //     outputColor = vec4(mip / float(numMipmaps), mip / float(numMipmaps), mip / float(numMipmaps), 1);
    //     return;
    // }
    // else
    // {
    //     outputColor = vec4(pageTableEntry.b/float(numMipmaps), pageTableEntry.b/float(numMipmaps), pageTableEntry.b/float(numMipmaps), 1);
    //     return;
    // }
#else
    // float bias = sampleMipMap;

    vec4 pageTableEntry = texture2D(pageTableTexture, fragUv) * 255.0;
    float mipExp = exp2(floor(pageTableEntry.b)); // alpha channel has mipmap-level
    vec2 withinPageCoord = fract(fragUv * mipExp);
    
    
    vec2 pageCoord = pageTableEntry.rg; // blue-green has x-y coordinates
    vec2 finalCoord =  ((pageCoord + withinPageCoord) / physicalTexturePageSize);
    // outputColor = vec4(pageTableEntry.b/float(numMipmaps), pageTableEntry.b/float(numMipmaps), pageTableEntry.b/float(numMipmaps), 1);
    // return;
#endif
      
    vec3 finalColor = texture(physicalTexture, finalCoord).rgb;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));
    outputColor = vec4(finalColor, 1.0f);
}
#version 400 core
//output
layout(location = 0) out vec4 outputColor; 
//main
#define PI 3.1415926535897932384626433832795

in vec2 fragUv;

 
uniform int numPagesX;
uniform int numPagesY;
uniform int numMipmaps;

uniform vec2 virtualTextureSize;

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
    float mip = (clamp(floor(mipmapLevel(fragUv)), 0, numMipmaps-1));
    
    //0 : 1,   1 : 2,  2 : 4,  3 : 8
    float texSize = exp2(mip);
    // vec2 pageID = floor(fragUv * (vec2(numPagesX, numPagesY) / texSize));
    vec2 pageID = floor(fragUv * (vec2(numPagesX, numPagesY)));

    pageID /= 255.0f;
    // float mip = floor(mipmapLevel(fragUv)) / 255.0;
    outputColor = vec4(pageID,mip/255.0f, 1.0f);
}
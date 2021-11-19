#version 400 core
//output
layout(location = 0) out vec4 outputColor; 
//main
#define PI 3.1415926535897932384626433832795

in vec2 fragUv;

 
uniform int numPagesX;
uniform int numPagesY;
uniform vec2 virtualTextureSize;

float mipmapLevel(vec2 uv)  {
    vec2 dx = dFdx(uv * virtualTextureSize.x);
    vec2 dy = dFdy(uv * virtualTextureSize.y);
    float d = max(dot(dx, dx), dot(dy, dy));
    return 0.5 * log2(d);   // explanation: 0.5*log(x) = log(sqrt(x)) + mip_bias - readback_reduction_shift;
}

void main()
{

    //Sample diffuse texture
    //Sample global alpha
    vec2 pageID = floor(fragUv * vec2(numPagesX, numPagesY));
    pageID /= 255.0f;

    float mip = floor(mipmapLevel(fragUv)) / 255.0;

    outputColor = vec4(pageID,mip, 1.0f);
}
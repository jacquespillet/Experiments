//inputs
#version 440
//output
layout(location = 0) out vec4 color; 
layout(location = 1) out float depth; 

uniform float pointSize;
uniform mat4 p;

const vec3 lightDir = vec3(1,1,1);

in vec2 uv;
in vec4 outColor;
in float outPointSize;
in vec3 eyeSpacePos;
void main()
{
    vec3 N;
    N.xy = uv*2.0-1.0;
    float r2 = dot(N.xy, N.xy);

    if (r2 > 1.0) discard;   // kill pixels outside circle

    N.z = -sqrt(1.0 - r2);
    
    // calculate depth
    vec4 pixelPos = vec4(eyeSpacePos + N * outPointSize, 1.0);
    vec4 clipSpacePos = p * pixelPos;

    float fragDepth = clipSpacePos.z / clipSpacePos.w;
    float diffuse = max(0.0, dot(N, -lightDir));
    
    // color = vec4(fragDepth, fragDepth, fragDepth, 1);
    color = clipSpacePos;
    // color = vec4(1,0,0,1);
    depth = fragDepth;
    gl_FragDepth = fragDepth;
}
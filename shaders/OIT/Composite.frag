#version 440 core

// shader outputs
layout (location = 0) out vec4 outputColor;

uniform sampler2D accum;
uniform sampler2D reveal;

// epsilon number
const float EPSILON = 0.00001f;

// calculate floating point numbers equality accurately
bool isApproxEqual(float a, float b)
{
    return abs(a - b) <= (abs(a) < abs(b) ? abs(b) : abs(a)) * EPSILON;
}

// get the max value between three values
float max3(vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

void main()
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    float revealage = texelFetch(reveal, coords, 0).r;
    if (isApproxEqual(revealage, 1.0f))
        discard;

    vec4 accumulation = texelFetch(accum, coords, 0);
    if (isinf(max3(abs(accumulation.rgb))))
        accumulation.rgb = vec3(accumulation.a);

    vec3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);
    outputColor = vec4(averageColor, 1.0f - revealage);
}
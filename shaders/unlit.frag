//inputs
#version 440
//output
layout(location = 0) out vec4 outputColor; 
//main

in vec3 fragNormal;
in vec3 fragPos;

const vec3 lightPos = vec3(0, 50, 0);

uniform sampler3D tmpTex;

void main()
{
    vec3 fragToLight = normalize(lightPos - fragPos);

    float intens = dot(fragNormal, fragToLight);
    outputColor = vec4(intens, intens, intens,1);
    // outputColor = vec4(fragNormal,1);    
}
#version 440
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

uniform mat4 modelViewProjectionMatrix;

out vec4 fragColor;

//main
void main()
{
    vec4 worldPosition =  modelViewProjectionMatrix * vec4(position.xyz, 1.0);
    fragColor = color;
    gl_Position = worldPosition;
}
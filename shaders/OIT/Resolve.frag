#version 440 core

layout (location = 0) out vec4 outputColor;

uniform sampler2D tex;
in vec2 fragUv;

void main()
{
    outputColor = texture(tex, fragUv);
}
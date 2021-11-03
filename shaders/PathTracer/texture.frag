//inputs
#version 400 core
layout(location = 0) out vec4 outputColor; 

in vec2 fragUv;
uniform sampler2D tex;
void main()
{
    outputColor = texture(tex, fragUv);
}
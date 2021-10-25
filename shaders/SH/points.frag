#version 440
layout(location = 0) out vec4 outputColor; 

in vec4 fragColor;
void main()
{
    outputColor = fragColor;    
}

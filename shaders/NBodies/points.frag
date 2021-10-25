//inputs
#version 440
//output
layout(location = 0) out vec4 color; 

uniform sampler2D particleTexture;
in vec2 uv;
void main()
{
    color = texture(particleTexture, uv) * vec4(0.8, 0.7, 0.3, 1);
}
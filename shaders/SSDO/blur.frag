#version 440 core
out vec4 fragColor;
  
in vec2 fragUv;

uniform sampler2D ssaoTexture;
// uniform vec2
void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(ssaoTexture, 0));
    vec4 result = vec4(0.0);
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoTexture, fragUv + offset);
        }
    }
    fragColor = result / (4.0 * 4.0);
}

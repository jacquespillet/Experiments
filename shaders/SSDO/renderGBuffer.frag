//inputs
#version 440
//output
layout(location = 0) out vec4 outputPosition; 
layout (location = 1) out vec4 outputNormal;
layout (location = 2) out vec4 outputColor;

in vec3 fragViewPos;
in vec3 fragViewNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
in vec3 fragToEyeWorld;
in vec3 fragToLightWorld;
in vec3 fragToEyeTangent;
in vec4 depthMapUV;


uniform int diffuseTextureSet;
uniform sampler2D diffuseTexture;
uniform vec3 mat_diffuse;

uniform int normalTextureSet;
uniform sampler2D normalTexture;

void main()
{
    vec3 N = fragViewNormal; //CalBump()
    if(normalTextureSet>0) {
        mat3 TBN = mat3(fragTangent, fragBitangent, fragViewNormal);
        N = texture(normalTexture, fragUv).rgb;
        N = N * 2.0 - 1.0;   
        N = normalize(TBN * N); 
    }
    vec4 color = vec4(mat_diffuse, 1.0f);
    if(diffuseTextureSet>0) color = texture(diffuseTexture, fragUv);

    if(color.a<0.5) discard;
    outputPosition = vec4(fragViewPos, 0);
    outputNormal = vec4(N, 0);
    outputColor = color;
}
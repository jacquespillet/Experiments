//inputs
#version 440
//output
layout(location = 0) out vec4 outputPosition; 
layout (location = 1) out vec4 outputNormal;
layout (location = 2) out vec4 outputColor;

in vec3 fragWorldPos;
in vec3 fragWorldNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
in vec3 fragToEyeWorld;
in vec3 fragToLightWorld;
in vec3 fragToEyeTangent;
in vec4 depthMapUV;


uniform sampler2D diffuseTexture;
uniform vec3 mat_diffuse;
uniform float mat_roughness;

uniform int normalTextureSet;
uniform sampler2D normalTexture;

void main()
{
    vec3 N = fragWorldNormal; //CalBump()
    if(normalTextureSet>0) {
        mat3 TBN = mat3(fragTangent, fragBitangent, fragWorldNormal);
        N = texture(normalTexture, fragUv).rgb;
        N = N * 2.0 - 1.0;   
        N = normalize(TBN * N); 
    }
    vec4 color = texture(diffuseTexture, fragUv);
    if(color.a<0.5) discard;
    outputPosition = vec4(fragWorldPos, mat_roughness);
    outputNormal = vec4(N, 0);
    outputColor = color;
}
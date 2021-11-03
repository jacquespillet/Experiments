//inputs
#version 400 core
//output
layout(location = 0) out vec4 outputPosition; 
layout(location = 1) out vec4 outputNormal; 
layout(location = 2) out vec4 outputColor; 
//main

in vec3 fragNormal;
in vec3 fragWorldPos;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
in vec3 fragToEyeWorld;
in vec3 fragToLightWorld;
in vec3 fragToEyeTangent;
in vec4 depthMapUV;

 
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D opacityTexture;
uniform sampler2D ambientTexture;
uniform sampler2D normalTexture;

uniform int diffuseTextureSet;
uniform int specularTextureSet;
uniform int opacityTextureSet;
uniform int ambientTextureSet;
uniform int normalTextureSet;


uniform vec3 mat_ambient;
uniform vec3 mat_diffuse;
uniform vec3 mat_specular;
uniform vec3 mat_emissive;
uniform vec3 mat_transparent;
uniform float mat_shininess;
uniform float mat_opacity;

uniform vec3 lightDirection;

mat3 tangentToWorld;


void main()
{
    vec4 sampleDiffuse =  texture(diffuseTexture, fragUv);
    float alpha = sampleDiffuse.a;
    if(alpha < 0.5) discard;
    
    vec3 diffuseColor     = pow(sampleDiffuse.rgb, vec3(2.2));
    outputColor = vec4(diffuseColor,alpha);

    vec3 N = fragNormal; //CalBump()
    if(normalTextureSet>0) {
        mat3 TBN = mat3(fragTangent, fragBitangent, fragNormal);
        N = texture(normalTexture, fragUv).rgb;
        N = N * 2.0 - 1.0;   
        N = normalize(TBN * N); 
    }
    outputNormal = vec4(N, 1);
    
    outputPosition = vec4(fragWorldPos, 1);
}
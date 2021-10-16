//inputs
#version 440
//output
layout(location = 0) out vec4 outputPosition; 
layout (location = 1) out vec4 outputNormal;
layout (location = 2) out vec4 outputColor;

in vec3 fragWorldPosition;
in vec3 fragWorldNormal;
in vec2 fragUv;
in vec4 depthMapUV;


uniform sampler2D diffuseTexture;


uniform sampler2DShadow rsmDepth;
uniform sampler2D rsmPosition;
uniform sampler2D rsmNormal;
uniform sampler2D rsmFlux;
uniform sampler2D randomTexture;

uniform float maxDistance;
uniform float intensity;
uniform int numSamples;
uniform int showDirect;
uniform int showIndirect;

vec3 DoReflectiveShadowMapping()
{
    vec3 indirectIllumination = vec3(0, 0, 0);
    float rMax = maxDistance;

    for (uint i = 0; i < numSamples; ++i)
    {
        vec3 rnd =texelFetch(randomTexture, ivec2(i, 0), 0).xyz;
        vec2 coords=depthMapUV.xy+rnd.xy*rMax;
		float weight=rnd.z;

        vec3 vplNormalWS = texture(rsmNormal, coords.xy).xyz;
        vec3 vplPositionWS = texture(rsmPosition, coords.xy).xyz;
        vec3 flux = texture(rsmFlux, coords.xy).xyz;
        
		vec3 result=flux*max(0, dot(vplNormalWS, fragWorldPosition-vplPositionWS))*max(0, dot(fragWorldNormal, vplPositionWS-fragWorldPosition))/pow(length(fragWorldPosition-vplPositionWS),4.0);
		result*=weight;
		indirectIllumination +=result;
    }
    return clamp((indirectIllumination / float(numSamples)) * intensity, 0, 1);
}

void main()
{
    outputPosition = vec4(fragWorldPosition, 0);
    outputNormal = vec4(normalize(fragWorldNormal), 0);
    vec3 indirect = DoReflectiveShadowMapping();
    outputColor = vec4(indirect, 1);
}
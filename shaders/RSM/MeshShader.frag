//inputs
#version 400 core
//output
layout(location = 0) out vec4 outputColor; 
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

uniform vec3 mat_ambient;
uniform vec3 mat_diffuse;
uniform vec3 mat_specular;
uniform vec3 mat_emissive;
uniform vec3 mat_transparent;
uniform float mat_shininess;
uniform float mat_opacity;

uniform vec3 lightDirection;

mat3 tangentToWorld;

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

uniform sampler2D lowResRSM;
uniform sampler2D lowResNormals;
uniform sampler2D lowResPositions;

uniform vec2 windowSize;


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
        
		vec3 result=flux*max(0, dot(vplNormalWS, fragWorldPos-vplPositionWS))*max(0, dot(fragNormal, vplPositionWS-fragWorldPos))/pow(length(fragWorldPos-vplPositionWS),4.0);
		result*=weight;
		indirectIllumination +=result;
    }
    return clamp((indirectIllumination / float(numSamples)) * intensity, 0, 1);
}

void main()
{
    //Sample diffuse texture
    vec4 diffuseColor = texture(diffuseTexture, fragUv);
    
    //Sample global alpha
    float alpha = diffuseColor.a;

    //Discard if less than 0.5
    if(alpha < 0.5) {
        discard;
    }
    
    // Normal, light direction and eye direction in world coordinates
    vec3 N = fragNormal; //CalBump()
    vec3 L = lightDirection;
    vec3 E = normalize(fragToEyeWorld);
    
    // Calculate diffuse light
    vec3 diffuseReflection;
    {
        float visibility = texture(rsmDepth, vec3(depthMapUV.xy, (depthMapUV.z - 0.0005)/depthMapUV.w));
                
        float cosTheta = max(0, dot(N, L));
        vec3 directDiffuseLight = vec3(visibility * cosTheta);

		float occlusion = 1.0;
        diffuseReflection = occlusion * (directDiffuseLight ) * diffuseColor.rgb;
    }
    
    // Calculate specular light
    vec3 specularReflection = vec3(0,0,0);

    vec3 specularColor = vec3(0.2, 0.2, 0.2);
    vec3 halfwayVec = normalize(L + E);
    specularReflection = specularColor  * pow(max(dot(fragNormal, halfwayVec),0), 64);

    float directAmbient = 0.1;

    outputColor = vec4(0,0,0,0);
    if(showDirect>0)
    {
        outputColor += vec4(
                            diffuseReflection 
                            + directAmbient * diffuseColor.rgb
                            , alpha);
    }
    if(showIndirect>0)
    {
        vec2 st = gl_FragCoord.xy/windowSize;
        vec3 lowResNormal = texture(lowResNormals, st).xyz;
        float normalDiff = abs(dot(lowResNormal, fragNormal));
        if(normalDiff < 0.9)
        {
            vec3 indirect = DoReflectiveShadowMapping();
            outputColor += vec4(indirect, 0);
        }
        else
        {
            vec3 indirect = texture(lowResRSM, st).xyz;
            outputColor += vec4(indirect, 0);
        }
    }
    
}
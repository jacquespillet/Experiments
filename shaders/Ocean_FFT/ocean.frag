//inputs
#version 430 core
//output
layout(location = 0) out vec4 outputColor; 
//main
#define PI 3.1415926535897932384626433832795

in vec3 fragNormal;
in vec3 fragWorldPos;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;
in vec3 fragToEyeWorld;
in vec3 fragToLightWorld;
in vec3 fragToEyeTangent;
in vec4 depthMapUV;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

mat3 tangentToWorld;
uniform sampler2D normalTexture;

uniform float metallic = 0.7;
uniform float roughness = 0.4;
uniform float lightIntensity = 1;
uniform float ambient = 0.15;

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}


float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float _roughness)
{
    return F0 + (max(vec3(1.0 - _roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
} 

void BSDF(vec3 V, vec3 L, vec3 normal, vec3 finalColor, vec3 radiance, out vec3 diffuse, out vec3 specular, float _roughness)
{
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, finalColor, metallic);

    vec3 H = normalize(V + L);
    
   // cook-torrance brdf
    float NDF = DistributionGGX(normal, H, _roughness);
    float G   = GeometrySmith(normal, V, L, _roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0);
    specular     = (numerator / max(denominator, 0.001)) * radiance;
    diffuse = (kD * finalColor / PI) * radiance;
}

void main()
{		
    vec3 waterColor = vec3(0.1, 0.2, 0.7);

    vec3 finalNormal = texture(normalTexture, fragUv).rgb;
    vec3 V = normalize(cameraPosition - fragWorldPos);
    
    vec3 R = reflect(-V, finalNormal); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, waterColor, metallic);

    vec3 Lo = vec3(0.0);
        

    vec3 L;
    L = -lightDirection;    
    vec3 radiance = lightIntensity * vec3(1,1,1);        
    vec3 specular;
    vec3 diffuse;
    BSDF(V, L, finalNormal, waterColor, radiance, diffuse, specular, roughness);
    float NdotL = max(dot(finalNormal, L), 0.0);                
    Lo += (diffuse + specular) * NdotL; 

    
    vec3 L2 = vec3(1, -1, 1);    
    BSDF(V, L, finalNormal, waterColor, radiance, diffuse, specular, roughness);
    float NdotL2 = max(dot(finalNormal, L2), 0.0);                
    Lo += (diffuse + specular) * NdotL2; 
    
    vec3 ambientColor =vec3(ambient) * waterColor;
    vec3 finalColor = ambientColor + Lo;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));
    outputColor = vec4(finalColor, 1.0f);


}
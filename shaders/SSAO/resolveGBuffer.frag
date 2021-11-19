//inputs
#version 430 core
//output
layout(location = 0) out vec4 outputColor; 
// layout(location = 1) out vec4 outputDepth; 
//main
#define PI 3.1415926535897932384626433832795

in vec2 fragUv;

uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D positionTexture;
uniform sampler2D ssaoTexture;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform int numLights;
uniform mat4 inverseViewMatrix;
uniform float ambient;

float roughness = 0.8;
float metallic = 0.1;

uniform int renderSSAO;
uniform int ssaoEnabled;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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
    vec4 sampleDiffuse = texture(colorTexture, fragUv);
    vec3 N = (inverseViewMatrix *vec4(texture(normalTexture, fragUv).xyz, 0.0f)).xyz;
    vec3 samplePosition = (inverseViewMatrix * vec4(texture(positionTexture, fragUv).xyz, 1.0f)).xyz;
    float AmbientOcclusion=1;
    if(ssaoEnabled>0) AmbientOcclusion = texture(ssaoTexture, fragUv).r;

    //Sample diffuse texture
    //Sample global alpha
    // vec3 diffuseColor     = sampleDiffuse.rgb
    vec3 diffuseColor     = sampleDiffuse.rgb;
    float alpha = sampleDiffuse.a;
    

    vec3 V = normalize(cameraPosition - samplePosition);
    vec3 R = reflect(-V, N); 
    vec3 F0 = mix(vec3(0.04), diffuseColor, metallic);
    vec3 Lo = vec3(0.0);
        

    vec3 L = -lightDirection;    
    vec3 radiance = vec3(1,1,1);        
    vec3 specular;
    vec3 diffuse;
    BSDF(V, L, N, diffuseColor, radiance, diffuse, specular, roughness);
    float NdotL = max(dot(N, L), 0.0);                
    Lo += (diffuse + specular) * NdotL; 
    
    if(renderSSAO>0)
    {
        outputColor = vec4(AmbientOcclusion,AmbientOcclusion,AmbientOcclusion, 1.0f);
    }
    else
    {
        // vec3 ambientColor =vec3(0.1) * diffuseColor;
        vec3 ambientColor = vec3(ambient * diffuseColor * AmbientOcclusion ); // here we add occlusion factor
        vec3 finalColor= ambientColor + Lo;
        // vec3 finalColor= ambientColor;
        float gamma = 2.2;
        finalColor = pow(finalColor, vec3(1.0/gamma));
        finalColor ;
        outputColor = vec4(finalColor, 1.0f);
    }
}
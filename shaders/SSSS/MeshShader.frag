#version 400 core
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
in float fragDepth;

 
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

uniform vec3 cameraPosition;
uniform vec3 lightDirection;

uniform float metallic = 0.5;

uniform float distortion;
uniform int power;
uniform float scale;
uniform sampler2D depthTexture;

const vec3 lightColor = vec3(0.2, 0.5, 0.8);

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

    //Sample diffuse texture
    //Sample global alpha
    vec3 diffuseColor = vec3(1,1,1);
    float alpha = 1;
    if(diffuseTextureSet>0)
    {
        vec4 sampleDiffuse =  texture(diffuseTexture, fragUv);
        diffuseColor     = sampleDiffuse.rgb;
        alpha = sampleDiffuse.a;
    }
    
    float roughness = 0.05;
    if(specularTextureSet>0)
    {
        roughness = 1 - texture(specularTexture, fragUv).r;
    }
    
    //Discard if less than 0.5
    if(alpha < 0.5) {
        discard;
    }
    
    // Normal, light direction and eye direction in world coordinates
    vec3 N = fragNormal; //CalBump()
    if(normalTextureSet>0) {
        mat3 TBN = mat3(fragTangent, fragBitangent, fragNormal);
        N = texture(normalTexture, fragUv).rgb;
        N = N * 2.0 - 1.0;   
        N = normalize(TBN * N); 
    }

    vec3 V = normalize(cameraPosition - fragWorldPos);
    vec3 R = reflect(-V, N); 
    vec3 F0 = mix(vec3(0.04), diffuseColor, metallic);
    vec3 Lo = vec3(0.0);
        

    vec3 L = -lightDirection;    
    vec3 radiance = 5 * lightColor;        
    vec3 specular;
    vec3 diffuse;
    BSDF(V, L, N, diffuseColor, radiance, diffuse, specular, roughness);
    float NdotL = max(dot(N, L), 0.0);                
    Lo += (diffuse + specular) * NdotL; 


    float fragBack = texelFetch(depthTexture, ivec2(gl_FragCoord), 0).x;
    float thickness = 1 - abs(fragDepth - fragBack);

    vec3 H = normalize(-L + N * distortion);
    float I = pow(clamp(dot(V, H), 0, 1), power) * scale * thickness;
    vec3 ss = I *lightColor;
    
    vec3 ambientColor =vec3(0.1) * diffuseColor;    
    vec3 finalColor= ambientColor + Lo + ss;
    // vec3 finalColor= ss;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));

    outputColor = vec4(finalColor, 1.0f);
    // outputColor = vec4(thickness,thickness,thickness, 1.0f);
}
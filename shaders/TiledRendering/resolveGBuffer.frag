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

uniform vec3 cameraPosition;
uniform int numLights;

float roughness = 0.4;
float metallic = 0.1;

struct PointLight
{
    vec3 position;
    float radius;
    vec4 color;
    vec4 currentPosition;
};
layout (std430, binding = 0) buffer lightsBlock {
   PointLight lights [];
};

float attenuate(vec3 lightDirection, float radius) {
	radius *= 0.85;
    float cutoff = 0.5;
	float attenuation = dot(lightDirection, lightDirection) / (100.0 * radius);
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);

	return clamp(attenuation, 0.0, 1.0);
}


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
    vec3 sampleNormal = texture(normalTexture, fragUv).xyz;
    vec3 samplePosition = texture(positionTexture, fragUv).xyz;

    //Sample diffuse texture
    //Sample global alpha
    // vec3 diffuseColor     = sampleDiffuse.rgb
    vec3 diffuseColor     = sampleDiffuse.rgb;
    float alpha = sampleDiffuse.a;
    
    float roughness = 0.4;
    
    //Discard if less than 0.5
    if(alpha < 0.5) {
        discard;
    }
    
    // Normal, light direction and eye direction in world coordinates
    vec3 N = sampleNormal; //CalBump()
    
    vec3 V = normalize(cameraPosition - samplePosition);
    vec3 R = reflect(-V, N); 
    vec3 F0 = mix(vec3(0.04), diffuseColor, metallic);
    vec3 Lo = vec3(0.0);

	//Start index of the current tile in the visible lights buffer
	//Iterate through all lights inside the cell
    for (uint lightIndex = 0; lightIndex < numLights ; lightIndex++) {
        //Get the light
        vec3 lightPos = lights[lightIndex].position;
        vec3 lightColor = lights[lightIndex].color.xyz;
        float lightRadius = lights[lightIndex].radius;

        vec3 fragToLight = lightPos - samplePosition;
        vec3 L = normalize(fragToLight);    

        float distance    = length(lightPos - samplePosition);
        // float attenuation = 1.0 / (distance * distance);
        
        float distanceOverRadius = distance / lights[lightIndex].radius;
        float term = clamp(1-pow((distanceOverRadius), 4), 0, 1);

        float attenuation = attenuate(fragToLight, lights[lightIndex].radius);
        vec3 radiance     = lightColor * attenuation; 

        vec3 specular;
        vec3 diffuse;
        BSDF(V, L, N, diffuseColor, radiance, diffuse, specular, roughness);
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (diffuse + specular) * NdotL * radiance; 
    }
 
    vec3 ambientColor =vec3(0.0) * diffuseColor;
    vec3 finalColor= ambientColor + Lo;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));
    outputColor = vec4(finalColor, 1.0f);
    
}
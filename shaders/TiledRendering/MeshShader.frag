//inputs
#version 440 core
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

uniform float metallic = 0.1;

uniform int numTilesX;
uniform int mode; //0 : Forward, 1: Forward+

uniform float zNear;
uniform float zFar;
uniform float scale;
uniform float bias;
uniform int tileSizeInPx;
uniform int numLights;
uniform uvec3 numClusters;

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

layout (std430, binding = 1) buffer tileLightIndices {
   uint tileLights[];
};


struct GridIndex
{
    uint startIndex;
    uint size;
    uint pad[2];
};
layout(std430, binding = 4) buffer lightIndicesBlock {
	uint lightIndices[];
};

layout(std430, binding = 3) buffer gridIndiceslock {
	GridIndex gridIndices[];
};

layout(std430, binding = 6) buffer debugColorBlock {
	vec4 randomColors[];
};


float attenuate(vec3 lightDirection, float radius) {
	radius *= 0.85;
    float cutoff = 0.5;
	float attenuation = dot(lightDirection, lightDirection) / (100.0 * radius);
	attenuation = 1.0 / (attenuation * 15.0 + 1.0);
	attenuation = (attenuation - cutoff) / (1.0 - cutoff);

	return clamp(attenuation, 0.0, 1.0);
}

float linearDepth(float depthSample){
    float depthRange = 2.0 * depthSample - 1.0;
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}

uint getClusterIndex(){
    // Uses equation (3) from Building a Cluster Grid section
    uint clusterZVal     = uint(max(log2(linearDepth(gl_FragCoord.z)) * scale + bias, 0.0));
    uvec3 clusters    = uvec3( uvec2( gl_FragCoord.xy / tileSizeInPx), clusterZVal);
    uint clusterIndex = clusters.x +
                        numClusters.x * clusters.y +
                        (numClusters.x * numClusters.y) * clusters.z;
    return clusterIndex;
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
	

    //Sample diffuse texture
    //Sample global alpha
    vec4 sampleDiffuse =  texture(diffuseTexture, fragUv);
    // vec3 diffuseColor     = pow(sampleDiffuse.rgb, vec3(2.2));
    vec3 diffuseColor     = sampleDiffuse.rgb;
    float alpha = sampleDiffuse.a;
    
    float roughness = 0.4;
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

	//Iterate through all lights inside the cell
    if(mode==0)
    {
        for (uint lightIndex = 0; lightIndex < numLights ; lightIndex++) {
            //Get the light
            vec3 lightPos = lights[lightIndex].position;
            vec3 lightColor = lights[lightIndex].color.xyz;
            float lightRadius = lights[lightIndex].radius;

            vec3 fragToLight = lightPos - fragWorldPos;
            vec3 L = normalize(fragToLight);    

            float distance    = length(lightPos - fragWorldPos);
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
    }
    else if(mode==1)
    {
        //Start index of the current tile in the visible lights buffer
        ivec2 tileID = ivec2(gl_FragCoord.xy) / ivec2(16, 16);
        uint tileFlatIndex = tileID.y * numTilesX + tileID.x;
        uint offset = tileFlatIndex * 1024;
        
        for (uint i = 0; i < 1024 && tileLights[offset + i] != -1; i++) {
            uint lightIndex = tileLights[offset + i];
            //Get the light
            vec3 lightPos = lights[lightIndex].position;
            vec3 lightColor = lights[lightIndex].color.xyz;
            float lightRadius = lights[lightIndex].radius;

            vec3 fragToLight = lightPos - fragWorldPos;
            vec3 L = normalize(fragToLight);    

            float distance    = length(lightPos - fragWorldPos);
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
    }
    if(mode==3)
    {
        uint index = getClusterIndex();
        GridIndex gridIndex = gridIndices[index];
        uint start = gridIndex.startIndex;
        uint size = gridIndex.size;
        vec3 color = vec3(float(size) / 100.0f);
        for(uint i = 0; i < size; i++){
            uint lightIndex = lightIndices[start + i];
            vec3 lightPos = lights[lightIndex].position;
            vec3 lightColor = lights[lightIndex].color.xyz;
            float lightRadius = lights[lightIndex].radius;

            vec3 fragToLight = lightPos - fragWorldPos;
            vec3 L = normalize(fragToLight);    

            float distance    = length(lightPos - fragWorldPos);
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
    }
    
    vec3 ambientColor =vec3(0.0) * diffuseColor;
    vec3 finalColor= ambientColor + Lo;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));
    outputColor = vec4(finalColor, 1.0f);
}
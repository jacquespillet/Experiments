//inputs
#version 400 core
//output
layout(location = 0) out vec4 outputColor; 
// layout(location = 1) out vec4 outputDepth; 
//main
#define PI 3.1415926535897932384626433832795

in vec2 fragUv;

uniform sampler2D colorTexture;
uniform sampler2D depthTexture;
uniform mat4 ip;
uniform mat4 iv;
uniform mat4 ivp;

uniform vec3 eyePos;

vec3 getEyePos(vec2 uv)
{
    float depth = texture(depthTexture, uv).x;
    float z = depth * 2.0 - 1.0;

    vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 viewSpacePosition = ip * clipSpacePosition;

    viewSpacePosition /= viewSpacePosition.w;

    return viewSpacePosition.xyz;
}


uniform float metallic = 0.7;
uniform float roughness = 0.4;
uniform float lightIntensity = 1.5;
uniform float ambient = 0.7;

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
    float depth = texture(depthTexture, fragUv).x;
    if (depth >1 || depth <=0) {
        outputColor = vec4(0,0,0,0);
        discard;
        return;
    }

    vec3 posEye = getEyePos(fragUv);
    vec3 worldPos = (iv * vec4(posEye, 1)).xyz;

    vec2 texelSize = vec2(1.0f/1920.0f, 1.0f/1080.0f);

    vec3 ddx = getEyePos(fragUv + vec2(texelSize.x, 0)) - posEye;
    vec3 ddx2 = posEye - getEyePos(fragUv + vec2(-texelSize.x, 0));

    if (abs(ddx.z) > abs(ddx2.z)) {
        ddx = ddx2;
    }
    vec3 ddy = getEyePos(fragUv + vec2(0, texelSize.y)) - posEye;
    vec3 ddy2 = posEye - getEyePos(fragUv + vec2(0, -texelSize.y));
    if (abs(ddy2.z) < abs(ddy.z)) {
        ddy = ddy2;
    }

    // // // calculate normal
    vec3 n = (iv * vec4(normalize(cross(ddx, ddy)), 0)).xyz;
    n = normalize(n);

    // vec3 finalColor = ;
    vec3 finalColor = texture(colorTexture, fragUv).xyz;

    vec3 finalNormal = n;
    vec3 V = normalize(eyePos - worldPos);
    
    vec3 R = reflect(-V, finalNormal); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, finalColor, metallic);

    vec3 Lo = vec3(0.0);
        

    vec3 L;
    L = vec3(1, 1, 1);    
    vec3 radiance = lightIntensity * vec3(1,1,1);        
    vec3 specular;
    vec3 diffuse;
    BSDF(V, L, finalNormal, finalColor, radiance, diffuse, specular, roughness);
    float NdotL = max(dot(finalNormal, L), 0.0);                
    Lo += (diffuse + specular) * NdotL; 

    
    vec3 L2 = vec3(-1, 1, -1);    
    BSDF(V, L, finalNormal, finalColor, radiance, diffuse, specular, roughness);
    float NdotL2 = max(dot(finalNormal, L2), 0.0);                
    Lo += (diffuse + specular) * NdotL2; 
    
    vec3 ambientColor =vec3(ambient) * finalColor;
    outputColor = vec4(ambientColor + Lo, 1.0f);
}
//inputs
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
in vec3 tangentViewDir;

 
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D opacityTexture;
uniform sampler2D ambientTexture;
uniform sampler2D normalTexture;
uniform sampler2D heightTexture;

uniform sampler2D texTest;

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

uniform float strength;
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

vec2 GetParallaxOffsetUv()
{
    vec3 viewDir   = normalize(tangentViewDir);
    viewDir.xy /= (viewDir.z  + 0.42f);

    float height =texture(heightTexture, fragUv).r;
    height -= 0.5f;

    vec2 finalFragUv = fragUv;
    height *= strength;
    finalFragUv += viewDir.xy * height;
    return finalFragUv;
}

vec2 GetParallaxRayMarching()
{
    vec2 finalFragUv = fragUv;

    vec3 viewDir   = normalize(tangentViewDir);
    viewDir.xy /= (viewDir.z  + 0.42f);
    

    vec2 uvOffset = vec2(0,0);
    float currentHeight = 1;
    
    float stepSize = 0.01f;
    vec2 uvDelta = viewDir.xy * stepSize * strength;
    

    float surfaceHeight = texture(heightTexture, finalFragUv).r;
    while(currentHeight > surfaceHeight)
    {
        finalFragUv -= uvDelta;
        currentHeight -= stepSize;
        surfaceHeight= texture(heightTexture, finalFragUv).r;
    }

    return finalFragUv;
}

vec2 GetParallaxRayMarchingInterpolation()
{
    int marchingSteps = 10;

    vec2 finalFragUv = fragUv;

    vec3 viewDir   = normalize(tangentViewDir);
    viewDir.xy /= (viewDir.z  + 0.42f);

  	vec2 uvOffset = vec2(0,0);
	float stepSize = 1.0f / marchingSteps;
	vec2 uvDelta = viewDir.xy * (stepSize * strength);

	float stepHeight = 1;
	float surfaceHeight = texture(heightTexture, finalFragUv).r;

	vec2 prevUVOffset = uvOffset;
	float prevStepHeight = stepHeight;
	float prevSurfaceHeight = surfaceHeight;
	for (int i = 1;i < marchingSteps && stepHeight > surfaceHeight; i++) {        
		prevUVOffset = uvOffset;
		prevStepHeight = stepHeight;
		prevSurfaceHeight = surfaceHeight;

		uvOffset -= uvDelta;
		stepHeight -= stepSize;
		surfaceHeight = texture(heightTexture, fragUv + uvOffset).r;
	}

	float prevDifference = prevStepHeight - prevSurfaceHeight;
	float difference = surfaceHeight - stepHeight;
    
    float t = prevDifference / (prevDifference + difference);
	uvOffset = mix(prevUVOffset, uvOffset, t);

    return fragUv + uvOffset;
}

vec3 QuadtreeDispMapping(vec3 v)
{
    int MaxLevel = 9;
    float NodeCount = pow(2.0f, MaxLevel);
    float HalfTexel = 1.0 / NodeCount / 2.0;
    float d;
    vec3 p2 = fragWorldPos;

    int Level = MaxLevel;
    //We calculate ray movement vector in inter -cell numbers .
    vec2 DirSign = sign(v.xy);
    // Main loop
    while (Level >= 0)
    {
        //We get current cell minimum plane using tex2Dlod .
        d = textureLod(texTest , p2.xy, Level).r;
        //If we are not blocked by the cell we move the ray .
        if (d > p2.z)
        {
            //We calculate predictive new ray position .
            vec3 tmpP2 = fragWorldPos + v * d;
            //We compute current and predictive position .
            // Calculations are performed in cell integer numbers .
            int NodeCount = int(pow(2, (MaxLevel - Level)));
            ivec4 NodeID = ivec4((int(p2.x), int(p2.y) , int(tmpP2.x), int(tmpP2.y)) * NodeCount );
            //We test if both positions are still in the same cell.
            //If not , we have to move the ray to nearest cell boundary .
            if (NodeID.x != NodeID.z || NodeID.y != NodeID.w)
            {
                //We compute the distance to current cell boundary .
                //We perform the calculations in continuous space .
                vec2 a = ( p2.xy - fragWorldPos.xy);
                vec2 p3 = ( NodeID.xy + DirSign) / NodeCount;
                vec2 b = ( p3.xy - fragWorldPos.xy);
                
                //We are choosing the nearest cell
                //by choosing smaller distance .
                vec2 dNC = abs(p2.z * b / a);
                d = min(d, min(dNC.x, dNC.y));
                // During cell crossing we ascend in hierarchy .
                Level+=2;
                // Predictive refinement
                tmpP2 = fragWorldPos + v * d;
            }
            //Final ray movement
            p2 = tmpP2;
        }
        // Default descent in hierarchy
        // nullified by ascend in case of cell crossing
        Level --;
    }
    //Use xy difference to offset tex coords
    return p2;    
}

void main()
{
    // vec2 finalFragUv = GetParallaxOffsetUv();
    // vec2 finalFragUv = GetParallaxRayMarching();
    // vec2 finalFragUv = GetParallaxRayMarchingInterpolation();
    vec3 viewDir   = normalize(tangentViewDir);
    // viewDir.xy /= (viewDir.z  + 0.42f);
    vec3 finalposition = QuadtreeDispMapping(viewDir);
    // vec2 diff = finalposition - fragWorldPos;
    vec2 finalFragUv = finalposition.xy;

    //Sample diffuse texture
    //Sample global alpha
    vec4 sampleDiffuse =  texture(diffuseTexture, finalFragUv);
    vec3 diffuseColor   = sampleDiffuse.rgb;
    float alpha = sampleDiffuse.a;
    
    
    float roughness = 0.4;
    if(specularTextureSet>0)
    {
        roughness = 1 - texture(specularTexture, finalFragUv).r;
    }
    
    //Discard if less than 0.5
    if(alpha < 0.5) {
        discard;
    }
    
    // Normal, light direction and eye direction in world coordinates
    vec3 N = fragNormal; //CalBump()
    if(normalTextureSet>0) {
        mat3 TBN = mat3(fragTangent, fragBitangent, fragNormal);
        N = texture(normalTexture, finalFragUv).rgb;
        N = N * 2.0 - 1.0;   
        N = normalize(TBN * N); 
    }

    vec3 V = normalize(cameraPosition - fragWorldPos);
    vec3 R = reflect(-V, N); 
    vec3 F0 = mix(vec3(0.04), diffuseColor, metallic);
    vec3 Lo = vec3(0.0);
        

    vec3 L = -lightDirection;    
    vec3 radiance = 5 * vec3(1,1,1);        
    vec3 specular;
    vec3 diffuse;
    BSDF(V, L, N, diffuseColor, radiance, diffuse, specular, roughness);
    float NdotL = max(dot(N, L), 0.0);                
    Lo += (diffuse + specular) * NdotL; 
    
    vec3 ambientColor =vec3(0.1) * diffuseColor;    
    vec3 finalColor= ambientColor + Lo;
    float gamma = 2.2;
    finalColor = pow(finalColor, vec3(1.0/gamma));
    outputColor = vec4(finalColor, 1.0f);
}
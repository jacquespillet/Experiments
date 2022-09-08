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

uniform sampler2DShadow shadowMap;
// uniform sampler2D shadowMap;

//UNIFORMS TO UPLOAD
uniform float VoxelGridWorldSize;
uniform int VoxelDimensions;
uniform sampler3D VoxelTexture;

uniform int showOcclusion;
uniform int showAmbient;
uniform int showIndirectDiffuse;
uniform int showDirectDiffuse;
uniform int showSpecular;

uniform float directAmbient;
uniform float specularity;

const int NUM_CONES = 6;
const float MAX_DIST = 100.0;
const float ALPHA_THRESH = 0.95;

vec3 coneDirections[6] = vec3[]
(                            vec3(0, 1, 0),
                            vec3(0, 0.5, 0.866025),
                            vec3(0.823639, 0.5, 0.267617),
                            vec3(0.509037, 0.5, -0.700629),
                            vec3(-0.509037, 0.5, -0.700629),
                            vec3(-0.823639, 0.5, 0.267617)
                            );
float coneWeights[6] = float[](0.25, 0.15, 0.15, 0.15, 0.15, 0.15);


vec4 sampleVoxels(vec3 worldPosition, float lod) {
    vec3 offset = vec3(1.0 / VoxelDimensions, 1.0 / VoxelDimensions, 0); // Why??
    vec3 voxelTextureUV = worldPosition / (VoxelGridWorldSize * 0.5);
    voxelTextureUV = voxelTextureUV * 0.5 + 0.5 + offset;
    return textureLod(VoxelTexture, voxelTextureUV, lod);
}
// Third argument to say how long between steps?
vec4 coneTrace(vec3 direction, float tanHalfAngle, out float occlusion) {
    
    // lod level 0 mipmap is full size, level 1 is half that size and so on
    float lod = 0.0;
    vec3 color = vec3(0);
    float alpha = 0.0;
    occlusion = 0.0;

    float voxelWorldSize = VoxelGridWorldSize / VoxelDimensions;
    float dist = voxelWorldSize; // Start one voxel away to avoid self occlusion
    vec3 startPos = fragWorldPos + fragNormal * voxelWorldSize; // Plus move away slightly in the normal direction to avoid
                                                                    // self occlusion in flat surfaces

    while(dist < MAX_DIST && alpha < ALPHA_THRESH) {
        // Calculate the diameter of the cone at this step
        float diameter = max(voxelWorldSize, 2.0 * tanHalfAngle * dist);
        
        //Calculate the mip level accordingly
        float lodLevel = log2(diameter / voxelWorldSize);
        
        //Sample the voxelized scene
        vec4 voxelColor = sampleVoxels(startPos + dist * direction, lodLevel);
        
        color = color + (1 - alpha) * voxelColor.rgb;
        alpha = alpha + (1 - alpha) * voxelColor.a;

        dist += diameter * 0.5; // smoother
    }

    occlusion = 1.0 - alpha;

    return vec4(color, 0);
}


vec4 indirectLight(out float occlusion_out) {
    vec4 color = vec4(0);
    occlusion_out = 0.0;

    //Tracing cones in the hemisphere
    for(int i = 0; i < NUM_CONES; i++) {
        float localOcclusion = 0.0;
        // 60 degree cones -> tan(30) = 0.577
        // 90 degree cones -> tan(45) = 1.0
        color += coneWeights[i] * coneTrace(tangentToWorld * coneDirections[i], 0.577, localOcclusion);
        occlusion_out += coneWeights[i] * localOcclusion;
    }

    // occlusion_out = 1.0 - occlusion_out;

    return color;
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
    
    //tangent to world
    tangentToWorld = inverse(transpose(mat3(fragTangent, fragNormal, fragBitangent)));

    // Normal, light direction and eye direction in world coordinates
    // vec3 N = fragNormal; //CalBump()
    vec3 N = fragNormal; //CalBump()
    if(normalTextureSet>0) {
        mat3 TBN = mat3(fragTangent, fragBitangent, fragNormal);
        N = texture(normalTexture, fragUv).rgb;
        N = N * 2.0 - 1.0;   
        N = normalize(TBN * N); 
    }

    vec3 L = lightDirection;
    vec3 E = normalize(fragToEyeWorld);
    
    // Calculate diffuse light
    vec3 diffuseReflection;
    {
        // Shadow map
        float visibility = texture(shadowMap, vec3(depthMapUV.xy, (depthMapUV.z - 0.0005)/depthMapUV.w));
        
        // Direct diffuse light
        float cosTheta = max(0, dot(N, L));
        vec3 directDiffuseLight = (showDirectDiffuse>0) ? vec3(visibility * cosTheta) : vec3(0,0,0);

        // Indirect diffuse light
		float occlusion = 0.0;
        vec3 indirectDiffuseLight = indirectLight(occlusion).rgb;
        indirectDiffuseLight = (showIndirectDiffuse>0) ? indirectDiffuseLight : vec3(0,0,0);
        indirectDiffuseLight = 4.0 * indirectDiffuseLight;

        
        occlusion = (showOcclusion > 0) ? min(1.0, 1.5* occlusion) : 1; // Make occlusion brighter
        diffuseReflection = 2.0 * occlusion * 
                                            (
                                            directDiffuseLight 
                                            + indirectDiffuseLight
                                            ) 
                                            * diffuseColor.rgb;
    }
    
    // Calculate specular light
    vec3 specularReflection = vec3(0,0,0);
    if(showSpecular>0){
        vec4 specularColor = vec4(specularity, specularity, specularity, 1);
        specularColor = length(specularColor.gb) > 0.0 ? specularColor : specularColor.rrra;
        vec3 reflectDir = normalize(-E - 2.0 * dot(-E, N) * N);

        float specularOcclusion;
        vec4 tracedSpecular = coneTrace(reflectDir, 0.06, specularOcclusion); 
        specularReflection = 2.0 * specularColor.rgb * tracedSpecular.rgb;
    }

    float directAmbient = (showAmbient>0) ?  directAmbient : 0;

    vec3 color = diffuseReflection + specularReflection + directAmbient * diffuseColor.rgb;
    float gamma = 2.2;
    color = pow(color, vec3(1.0/gamma));
    outputColor = vec4(color, alpha);
            
}
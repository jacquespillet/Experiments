#version 440 core
out vec4 fragColor;
  
in vec2 fragUv;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D colorTexture;
uniform sampler2D noiseTexture;

uniform mat4 projection;
uniform vec2 screenResolution;
uniform vec3 lightDirection;

uniform int kernelSize;
uniform float radius;
uniform float indirectRadiance;
uniform float occlusionThreshold;
uniform float strength;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D envmapTexture;

layout (std430, binding = 0) buffer kernelBlock {
   vec3 kernel [];
};

#define PI 3.1415926535897932384626433832795

void main()
{
    vec2 noiseScale = vec2(screenResolution.x/4.0, screenResolution.y/4.0);

    vec3 fragPos   = texture(positionTexture, fragUv).xyz;
    vec3 normal    = texture(normalTexture, fragUv).rgb;
    vec3 randomVec = texture(noiseTexture, fragUv * noiseScale).xyz;  

    //Create a TBN basis using a random vector as tangent axis
    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);  

    vec3 directRadianceSum = vec3(0.0);
    vec3 occluderRadianceSum = vec3(0.0);
    vec3 ambientRadianceSum = vec3(0.0);
    float ambientOcclusion = 0.0;
    
    for(int i = 0; i < kernelSize ; i++) {
        vec3 samplePos = TBN * kernel[i]; 
        vec3 normalizedSample = normalize(samplePos);
        
        vec4 worldSampleOccluderPosition = vec4(fragPos, 1.0f) + radius * vec4(samplePos.x, samplePos.y, samplePos.z, 0.0f);
        float sampleDepth = (viewMatrix * worldSampleOccluderPosition).z;
        
        
        vec4 occluderSamplePosition = ( projectionMatrix * viewMatrix) * worldSampleOccluderPosition;
        vec2 occluderTexCoord = vec2(0.5) + 0.5 * (occluderSamplePosition.xy / occluderSamplePosition.w);
        
        vec3 occluderPosition = texture(positionTexture, occluderTexCoord).xyz;
        vec3 occluderNormal = texture(normalTexture, occluderTexCoord).xyz;
        float storedDepth = ( viewMatrix * vec4(occluderPosition, 1.0f)).z;
        
        //If the distance is very small, this is 1
        float distanceTerm = abs(sampleDepth - storedDepth) < radius ? 1.0f : 0.0f;
        float occlusion = 1.0 - strength * (storedDepth > sampleDepth ? 1.0 : 0.0) * distanceTerm;
        
        float receiverGeometricTerm = max(0.0, dot(normalizedSample , normal)); 
        
        // vec3 senderRadiance = texture(envmapTexture , vec2( phi / (2.0*PI), 1.0 - theta / PI ) ). rgb;     
        float senderRadiance = max(0, dot(-lightDirection, normalizedSample));
        vec3 radiance = occlusion * receiverGeometricTerm * vec3(senderRadiance);
        directRadianceSum += radiance;

        
        
        vec3 directRadiance = texture(colorTexture , occluderTexCoord).rgb;
        vec3 delta = fragPos.xyz - occluderPosition.xyz;
        vec3 normalizedDelta = normalize(delta);
        float unclampedBounceGeometricTerm =
        max(0.0, dot(normalizedDelta , - normal)) *
        max(0.0, dot(normalizedDelta , occluderNormal));
        // /dot(delta , delta);

        float bounceGeometricTerm = min(unclampedBounceGeometricTerm, radius);
        vec3 occluderRadiance = indirectRadiance * bounceGeometricTerm * directRadiance;
        occluderRadianceSum += occluderRadiance;
    }
    directRadianceSum = max(vec3(0), directRadianceSum);
    occluderRadianceSum = max(vec3(0), occluderRadianceSum);
    
    // vec3 radianceSum = directRadianceSum + occluderRadianceSum;
    vec3 radianceSum = directRadianceSum + occluderRadianceSum;
    // vec3 radianceSum = occluderRadianceSum;
    radianceSum *= 2.0 * PI / kernelSize ;
    
    fragColor = vec4(radianceSum , 1.0);
}

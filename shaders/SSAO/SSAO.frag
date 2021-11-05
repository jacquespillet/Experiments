#version 440 core
out float fragColor;
  
in vec2 fragUv;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D noiseTexture;

uniform mat4 projection;
uniform vec2 screenResolution;

uniform int kernelSize;
uniform float radius;
uniform float bias;
uniform int power;

layout (std430, binding = 0) buffer kernelBlock {
   vec3 kernel [];
};


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

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        //Transform the kernel in to the local tangent space
        vec3 samplePos = TBN * kernel[i]; 
        
        //Calculate the sample pos in view space
        samplePos = fragPos + samplePos * radius;

        //Transform the sample position to uv space where we can sample the depth map
        vec4 offset = vec4(samplePos, 1.0);
        offset      = projection * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  

        float sampleDepth = texture(positionTexture, offset.xy).z; 

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion       += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
        // occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0);  
    }  
    occlusion = 1.0 - (occlusion / kernelSize);
    fragColor = pow(occlusion, power);
}

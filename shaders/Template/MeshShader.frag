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

uniform vec3 mat_ambient;
uniform vec3 mat_diffuse;
uniform vec3 mat_specular;
uniform vec3 mat_emissive;
uniform vec3 mat_transparent;
uniform float mat_shininess;
uniform float mat_opacity;

uniform vec3 lightDirection;

mat3 tangentToWorld;


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
    
    // Normal, light direction and eye direction in world coordinates
    vec3 N = fragNormal; //CalBump()
    vec3 L = lightDirection;
    vec3 E = normalize(fragToEyeWorld);
    
    // Calculate diffuse light
    vec3 diffuseReflection;
    {
        // float visibility = texture(rsmDepth, vec3(depthMapUV.xy, (depthMapUV.z - 0.0005)/depthMapUV.w));
        float visibility = 1;
                
        float cosTheta = max(0, dot(N, L));
        vec3 directDiffuseLight = vec3(visibility * cosTheta);

		float occlusion = 1.0;
        diffuseReflection = occlusion * (directDiffuseLight ) * diffuseColor.rgb;
    }
    
    // Calculate specular light
    vec3 specularReflection = vec3(0,0,0);

    vec3 specularColor = vec3(0.2, 0.2, 0.2);
    vec3 halfwayVec = normalize(L + E);
    specularReflection = specularColor  * pow(max(dot(fragNormal, halfwayVec),0), 64);

    float directAmbient = 0.1;

    outputColor = vec4(0,0,0,0);
    outputColor += vec4(
                        diffuseReflection 
                        + directAmbient * diffuseColor.rgb
                        , alpha);    
}
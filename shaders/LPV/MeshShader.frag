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

#define PI 3.1415926f

 
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

uniform sampler2DShadow shadowMap;

uniform sampler3D RAccumulatorLPV_l0;
uniform sampler3D GAccumulatorLPV_l0;
uniform sampler3D BAccumulatorLPV_l0;
uniform vec3 gridDim;
uniform vec3 allGridMins;
uniform vec3 allCellSizes;

uniform int displayGI;
uniform int displayShadows;
uniform int displayDirectDiffuse;
uniform float ambient;
uniform float specular;
uniform float GIIntensity;

/*Spherical harmonics coefficients - precomputed*/
#define SH_C0 0.282094792f // 1 / 2sqrt(pi)
#define SH_C1 0.488602512f // sqrt(3/pi) / 2

// no normalization
vec4 evalSH_direct( vec3 direction ) {	
	return vec4( SH_C0, -SH_C1 * direction.y, SH_C1 * direction.z, -SH_C1 * direction.x );
}

void main()
{
    float visibility = 1;
    if(displayShadows>0) visibility = texture(shadowMap, vec3(depthMapUV.xy, (depthMapUV.z - 0.0005)/depthMapUV.w));

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
    vec3 diffuseReflection=vec3(0);
    if(displayDirectDiffuse>0){
                
        float cosTheta = max(0, dot(N, L));
        vec3 directDiffuseLight = vec3(visibility * cosTheta);
		float occlusion = 1.0;
        diffuseReflection = occlusion * (directDiffuseLight ) * diffuseColor.rgb;
    }
    
    // Calculate specular light
    vec3 specularReflection = vec3(0,0,0);

    vec3 specularColor = vec3(specular);
    vec3 halfwayVec = normalize(L + E);
    specularReflection = specularColor  * pow(max(dot(fragNormal, halfwayVec),0), 64);


    outputColor = vec4(0,0,0,0);

    vec3 GI=vec3(0);
    if(displayGI > 0)
    {
        float f_indirectAttenuation = 1.0;
        vec4 SHintensity = evalSH_direct( -fragNormal );
        vec3 lpvIntensity = vec3(0.0);
        vec3 lpvCellCoords = (fragWorldPos - allGridMins[0]) / allCellSizes.x / gridDim; //<0,1>
        lpvIntensity = vec3( 
            dot( SHintensity, texture( RAccumulatorLPV_l0, lpvCellCoords) ),
            dot( SHintensity, texture( GAccumulatorLPV_l0, lpvCellCoords ) ),
            dot( SHintensity, texture( BAccumulatorLPV_l0, lpvCellCoords ) )
        );
        vec3 finalLPVRadiance = (f_indirectAttenuation / PI)*  max( lpvIntensity, 0 ) ;
        GI = diffuseColor.xyz * finalLPVRadiance * GIIntensity;
    }
	
    {
        
        vec3 color= diffuseReflection 
                            + ambient * diffuseColor.rgb
                            + specularReflection
                            + GI;
        float gamma = 2.2;
        color = pow(color, vec3(1.0/gamma));
        outputColor = vec4( color
                            , alpha);
    }
}
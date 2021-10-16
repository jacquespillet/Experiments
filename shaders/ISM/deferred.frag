//inputs
#version 400 core
//output
layout(location = 0) out vec4 outputColor; 
//main

in vec2 fragUv;

uniform sampler2D positionTexture; 
uniform sampler2D normalTexture;
uniform sampler2D colorTexture;
uniform sampler2D randomTexture;

uniform sampler2D rsmDepth;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

void main()
{
    vec3 worldPosition = texture2D(positionTexture, fragUv).xyz;
    vec3 worldNormal = texture2D(normalTexture, fragUv).xyz;
    vec4 diffuseColor = texture2D(colorTexture, fragUv);

    float alpha = diffuseColor.a;
    //Discard if less than 0.5
    if(alpha < 0.5) {
        discard;
    }
    
    // Normal, light direction and eye direction in world coordinates
    vec3 N = worldNormal; //CalBump()
    vec3 L = lightDirection;
    vec3 E = normalize(cameraPosition - worldPosition);
    
    // Calculate diffuse light
    vec3 diffuseReflection;
    {
        // float visibility = texture(rsmDepth, vec3(depthMapUV.xy, (depthMapUV.z - 0.0005)/depthMapUV.w));
        float visibility = 1;
                
        float cosTheta = max(0, dot(N, L));
        vec3 directDiffuseLight = vec3(visibility * cosTheta);

		float occlusion = 1.0;
        diffuseReflection = occlusion * (directDiffuseLight ) * diffuseColor.rgb * 3.0f;
    }

    outputColor = vec4(diffuseReflection, 1);
}
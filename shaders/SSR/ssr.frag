//inputs
#version 440
#define NUM_SAMPLES 32
#define NUM_SAMPLES_HALF 16

//output
layout(location = 0) out vec4 outputColor;

uniform sampler2D colorTexture;
uniform sampler2D positionTexture;
uniform sampler2D normalTexture;
uniform sampler2D noiseTexture;
uniform sampler2D depthTexture;

uniform vec3 cameraPosition;
uniform mat4 invV;
uniform mat4 invP;
uniform mat4 vp;
uniform float zNear;
uniform float zFar;

uniform vec2 screenResolution;
uniform vec2 invNoiseResolution;
uniform float downScale;
uniform float bias;

in vec2 fragUv;

float linearDepth(float depthSample){
    float depthRange = 2.0 * depthSample - 1.0;
    float linear = 2.0 * zNear * zFar / (zFar + zNear - depthRange * (zFar - zNear));
    return linear;
}

vec2 world2screen(vec3 wp)
{
    vec4 vec = vec4(wp - cameraPosition, 0.0);
    //Screen ray
    vec4 screenRay = vp * vec;
    screenRay.xy /= screenRay.w;
    screenRay.xy += 1.0;
    screenRay.xy *= 0.5;
    return screenRay.xy;
}

vec3 ray(vec3 worldPosition)
{
    vec2 screenPos = world2screen(worldPosition);
    float depth = linearDepth(texture(depthTexture, screenPos).r);
    
    float currentDistance = distance(worldPosition, cameraPosition);

    return vec3(screenPos, depth - currentDistance);
}

void main()
{
    vec4 SSRValue = vec4(0);
    //Get world position
    vec3 worldPosition=texture(positionTexture, fragUv).xyz;
    
    //Get distance from frag to camera
    float distanceToCamera= distance(worldPosition, cameraPosition);
    
    
    //direction from camera to frag
    vec3 viewDir=normalize(worldPosition-cameraPosition);

    //World normal
    vec3 worldNormal = texture(normalTexture, fragUv).rgb;
    
    //Calculate start sampling distance
    float sampleDistance=distanceToCamera/NUM_SAMPLES_HALF*1.5;
    
    //Calculate world space reflection, scaled by the sample distance
    vec3 dir =reflect(viewDir, worldNormal) * sampleDistance;//sample dis
    //Sampling position
    //pos = dir*tex2Dlod(_Noise, i.noiseUV).r*4+worldPosition;
    vec2 noiseUv = screenResolution * invNoiseResolution * fragUv / downScale;
    float noise = texture(noiseTexture, noiseUv).r;
    //noise = 1;
    
    //Calculate sampling position. Multiply the direction vector with a random number
    //pos = dir*noise*4  + worldPosition;
    vec3 pos = dir*noise* bias+ worldPosition;
    //return vec4 (pos, 1);
    
    //Contains whether we hit or not
    float collision;
    vec3 result;
    vec4 ssrValue;
    for (int j = 0; j < NUM_SAMPLES; ++j) //sample count
    {
        collision = 0;
        result = ray(pos);

        //If one of the screen results is out of the screen
        if ((floor(result.xy)!=vec2(0,0)))
        {
            collision = 1;
            SSRValue=vec4(1,1,1,1);
            break; //out of screen 
        }

        //we hit something
        //If depth is > 0, we're still in front.
        //If its under, we're behind.
        //We make sure that the sample is not too far away as well to remove artifacts
        if(result.z<0&&result.z>-(sampleDistance))
        { 
            //Set as a random number
            //collision = texture(ssrMask, result.xy).r;
            collision = 1;
            ssrValue= vec4(texture(colorTexture, result.xy).xyz,1);
            break;
        }

        pos += dir;
    }

    outputColor = clamp( vec4(ssrValue.rgb,collision), 0, 1);
}
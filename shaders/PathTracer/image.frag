//inputs
#version 400 core
layout(location = 0) out vec4 outputColor0; 
layout(location = 1) out vec4 outputColor1; 

in vec2 fragUv;

const float minDistance = 0.01f;
const float maxDistance = 10000.0f;
const float rayPosDelta = 0.01f;
const float twoPi = 6.28318530718f; 
const vec3 backgroundColor = vec3(0.8, 0.8, 0.8);
const int numSamples = 20;

uniform mat4 viewMatrix;

struct material
{
    float specular;
    float roughness;
    vec3 specularColor;
    vec3 albedo;
    float IOR;
};

struct hit
{
    float distance;
    vec3 normal;
    vec3 emission;
    material materialInfo;
};

struct sphere
{
    vec3 position;
    float radius;
};

float FresnelReflectAmount(float n1, float n2, vec3 normal, vec3 incident, float f0, float f90)
{
        // Schlick aproximation
        float r0 = (n1-n2) / (n1+n2);
        r0 *= r0;
        float cosX = -dot(normal, incident);
        if (n1 > n2)
        {
            float n = n1/n2;
            float sinT2 = n*n*(1.0-cosX*cosX);
            // Total internal reflection
            if (sinT2 > 1.0)
                return f90;
            cosX = sqrt(1.0-sinT2);
        }
        float x = 1.0-cosX;
        float ret = r0+(1.0-r0)*x*x*x*x*x;
 
        // adjust reflect multiplier for object reflectivity
        return mix(f0, f90, ret);
}

bool RaySphere(vec3 rayOrigin, vec3 rayDirection, inout hit hitInfo, sphere sphereInfo)
{
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    float a = dot(rayDirection, rayDirection);
    vec3 rayToSphere = rayOrigin - sphereInfo.position;
    float b = 2.0 * dot(rayDirection, rayToSphere);
    float c = dot(rayToSphere, rayToSphere) - (sphereInfo.radius * sphereInfo.radius);
    if (b*b - 4.0*a*c < 0.0) {
        return false;
    }
    float distance = (-b - sqrt((b*b) - 4.0*a*c))/(2.0*a);
    if (distance > minDistance && distance < hitInfo.distance)
    {
        hitInfo.distance = distance;
        vec3 hitPosition = rayOrigin + hitInfo.distance * rayDirection;
        hitInfo.normal = normalize(hitPosition - sphereInfo.position);
        return true;
    }
    return false;
}
float ScalarTriple(vec3 u, vec3 v, vec3 w)
{
    return dot(cross(u, v), w);
}
bool RayQuad(vec3 rayPosition, vec3 rayDirection, inout hit hitInfo, vec3 a, vec3 b, vec3 c, vec3 d)
{
    vec3 normal = normalize(cross(c-a, c-b));
    if (dot(normal, rayDirection) > 0.0f)
    {
        normal *= -1.0f;
        
		vec3 temp = d;
        d = a;
        a = temp;
        
        temp = b;
        b = c;
        c = temp;
    }
    
    vec3 p = rayPosition;
    vec3 q = rayPosition + rayDirection;
    vec3 pq = q - p;
    vec3 pa = a - p;
    vec3 pb = b - p;
    vec3 pc = c - p;
    
    // determine which triangle to test against by testing against diagonal first
    vec3 m = cross(pc, pq);
    float v = dot(pa, m);
    vec3 intersectPos;
    if (v >= 0.0f)
    {
        // test against triangle a,b,c
        float u = -dot(pb, m);
        if (u < 0.0f) return false;
        float w = ScalarTriple(pq, pb, pa);
        if (w < 0.0f) return false;
        float denom = 1.0f / (u+v+w);
        u*=denom;
        v*=denom;
        w*=denom;
        intersectPos = u*a+v*b+w*c;
    }
    else
    {
        vec3 pd = d - p;
        float u = dot(pd, m);
        if (u < 0.0f) return false;
        float w = ScalarTriple(pq, pa, pd);
        if (w < 0.0f) return false;
        v = -v;
        float denom = 1.0f / (u+v+w);
        u*=denom;
        v*=denom;
        w*=denom;
        intersectPos = u*a+v*d+w*c;
    }
    
    float dist;
    if (abs(rayDirection.x) > 0.1f)
    {
        dist = (intersectPos.x - rayPosition.x) / rayDirection.x;
    }
    else if (abs(rayDirection.y) > 0.1f)
    {
        dist = (intersectPos.y - rayPosition.y) / rayDirection.y;
    }
    else
    {
        dist = (intersectPos.z - rayPosition.z) / rayDirection.z;
    }
    
	if (dist > minDistance && dist < hitInfo.distance)
    {
        hitInfo.distance = dist;        
        hitInfo.normal = normal;        
        return true;
    }    
    
    return false;
}

uint wang_hash(inout uint seed)
{
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}
 
float RandomFloat01(inout uint state)
{
    return float(wang_hash(state)) / 4294967296.0;
}
 
vec3 RandomUnitVector(inout uint state)
{
    float z = RandomFloat01(state) * 2.0f - 1.0f;
    float a = RandomFloat01(state) * twoPi;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}

void TraceScene(vec3 rayOrigin, vec3 rayDirection, inout hit hitInfo)
{
    vec3 sceneTranslation = vec3(0.0f, 0.0f, 0.0f);

   	// back wall
    {
        vec3 A = vec3(-12.6f, -12.6f, 0.0f) + sceneTranslation;
        vec3 B = vec3( 12.6f, -12.6f, 0.0f) + sceneTranslation;
        vec3 C = vec3( 12.6f,  12.6f, 0.0f) + sceneTranslation;
        vec3 D = vec3(-12.6f,  12.6f, 0.0f) + sceneTranslation;
        if (RayQuad(rayOrigin, rayDirection, hitInfo, A, B, C, D))
        {
            hitInfo.materialInfo.albedo = vec3(0.7f, 0.7f, 0.7f);
            hitInfo.emission = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.specular = 0.0f;
            hitInfo.materialInfo.roughness = 1.0f;
            hitInfo.materialInfo.specularColor = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.IOR = 1.0f;
        }
	}    
    
    // floor
    {
        vec3 A = vec3(-12.6f, -12.45f, 0.0f) + sceneTranslation;
        vec3 B = vec3( 12.6f, -12.45f, 0.0f) + sceneTranslation;
        vec3 C = vec3( 12.6f, -12.45f, -10.0f) + sceneTranslation;
        vec3 D = vec3(-12.6f, -12.45f, -10.0f) + sceneTranslation;
        if (RayQuad(rayOrigin, rayDirection, hitInfo, A, B, C, D))
        {
            hitInfo.materialInfo.albedo = vec3(0.7f, 0.7f, 0.7f);
            hitInfo.emission = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.specular = 0.0f;
            hitInfo.materialInfo.roughness = 1.0f;
            hitInfo.materialInfo.specularColor = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.IOR = 1.0f;
        }        
    }
    
    // cieling
    {
        vec3 A = vec3(-12.6f, 12.5f, 0.0f) + sceneTranslation;
        vec3 B = vec3( 12.6f, 12.5f, 0.0f) + sceneTranslation;
        vec3 C = vec3( 12.6f, 12.5f, -10.0f) + sceneTranslation;
        vec3 D = vec3(-12.6f, 12.5f, -10.0f) + sceneTranslation;
        if (RayQuad(rayOrigin, rayDirection, hitInfo, A, B, C, D))
        {
            hitInfo.materialInfo.albedo = vec3(0.7f, 0.7f, 0.7f);
            hitInfo.emission = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.specular = 0.0f;
            hitInfo.materialInfo.roughness = 1.0f;
            hitInfo.materialInfo.specularColor = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.IOR = 1.0f;
        }        
    }    
    
    // left wall
    {
        vec3 A = vec3(-12.5f, -12.6f, 0.0f) + sceneTranslation;
        vec3 B = vec3(-12.5f, -12.6f, -10.0f) + sceneTranslation;
        vec3 C = vec3(-12.5f,  12.6f, -10.0f) + sceneTranslation;
        vec3 D = vec3(-12.5f,  12.6f, 0.0f) + sceneTranslation;
        if (RayQuad(rayOrigin, rayDirection, hitInfo, A, B, C, D))
        {
            hitInfo.materialInfo.albedo = vec3(0.7f, 0.1f, 0.1f);
            hitInfo.emission = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.specular = 0.0f;
            hitInfo.materialInfo.roughness = 1.0f;
            hitInfo.materialInfo.specularColor = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.IOR = 1.0f;
        }        
    }
    
    // right wall 
    {
        vec3 A = vec3( 12.5f, -12.6f, 0.0f) + sceneTranslation;
        vec3 B = vec3( 12.5f, -12.6f, -10.0f) + sceneTranslation;
        vec3 C = vec3( 12.5f,  12.6f, -10.0f) + sceneTranslation;
        vec3 D = vec3( 12.5f,  12.6f, 0.0f) + sceneTranslation;
        if (RayQuad(rayOrigin, rayDirection, hitInfo, A, B, C, D))
        {
            hitInfo.materialInfo.albedo = vec3(0.1f, 0.7f, 0.1f);
            hitInfo.emission = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.specular = 0.0f;
            hitInfo.materialInfo.roughness = 1.0f;
            hitInfo.materialInfo.specularColor = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.IOR = 1.0f;
        }        
    }        
        
    // light
    {
        vec3 A = vec3(-5.0f, 12.4f,  -2.5) + sceneTranslation;
        vec3 B = vec3( 5.0f, 12.4f,  -2.5) + sceneTranslation;
        vec3 C = vec3( 5.0f, 12.4f,  -5) + sceneTranslation;
        vec3 D = vec3(-5.0f, 12.4f,  -5) + sceneTranslation;
        if (RayQuad(rayOrigin, rayDirection, hitInfo, A, B, C, D))
        {
            hitInfo.materialInfo.albedo = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.emission = vec3(1.0f, 0.9f, 0.7f) * 20.0f;
            hitInfo.materialInfo.specular = 0.0f;
            hitInfo.materialInfo.roughness = 1.0f;
            hitInfo.materialInfo.specularColor = vec3(0.0f, 0.0f, 0.0f);
            hitInfo.materialInfo.IOR = 1.0f;
        }        
    }

    sphere sphere1 = sphere( sceneTranslation + vec3(-9.0f, -9.5f, -5), 3.0f);
    if(RaySphere(rayOrigin, rayDirection, hitInfo, sphere1))
    {
        hitInfo.materialInfo.albedo = vec3(0.9f, 0.9f, 0.75f);
        hitInfo.emission = vec3(0.0f,0.0f,0.0f);
        hitInfo.materialInfo.specular = 1;
        hitInfo.materialInfo.roughness = 0.05;
        hitInfo.materialInfo.specularColor = vec3(0.8, 0.5, 0.5);
        hitInfo.materialInfo.IOR = 1.0f;
    }

    sphere sphere2 = sphere( sceneTranslation + vec3(0.0f, -9.5f, -5), 3.0f);
    if(RaySphere(rayOrigin, rayDirection, hitInfo, sphere2))
    {
        hitInfo.emission = vec3(0.0f,0.0f,0.0f);
        hitInfo.materialInfo.albedo = vec3(0.9f, 0.75f, 0.9f);
        hitInfo.materialInfo.specular = 1;
        hitInfo.materialInfo.roughness = 0.4;
        hitInfo.materialInfo.specularColor = vec3(0.8, 0.8, 0.5);
        hitInfo.materialInfo.IOR = 1.0f;
    }

    sphere sphere3 = sphere( sceneTranslation + vec3(9.0f, -9.5f, -5), 3.0f);
    if(RaySphere(rayOrigin, rayDirection, hitInfo, sphere3))
    {
        hitInfo.materialInfo.albedo = vec3(0.75f, 0.9f, 0.9f);
        hitInfo.emission = vec3(0.0f,0.0f,0.0f);
        hitInfo.materialInfo.specular = 1;
        hitInfo.materialInfo.roughness = 0.7;
        hitInfo.materialInfo.specularColor = vec3(0.8, 0.8, 0.8);
        hitInfo.materialInfo.IOR = 1.0f;
    }
}

vec3 Color(vec3 rayOriginStart, vec3 rayDirectionStart, inout uint entropy)
{
    vec3 result = vec3(0.0f,0.0f,0.0f);
    vec3 attenuation = vec3(1.0f, 1.0f, 1.0f);

    vec3 rayOrigin = rayOriginStart;
    vec3 rayDirection = rayDirectionStart;

    for(int bounce=0; bounce<=8; ++bounce)
    {
        hit hitInfo;
        hitInfo.distance = maxDistance;
        TraceScene(rayOrigin, rayDirection, hitInfo);

        if(hitInfo.distance == maxDistance) {
            result += attenuation * backgroundColor;
            break;
        }

        vec3 hitPosition = rayOrigin + rayDirection * hitInfo.distance;

        // apply fresnel
        float specularChance = hitInfo.materialInfo.specular;
        if (specularChance > 0.0f)
        {
            specularChance = FresnelReflectAmount(
                1.0,
                hitInfo.materialInfo.IOR,
                rayDirection, hitInfo.normal, hitInfo.materialInfo.specular, 1.0f);  
        }
        float doSpecular = (RandomFloat01(entropy) < specularChance) ? 1.0f : 0.0f;
        
        //calculate next ray
        vec3 diffuseRayDirection = normalize(hitInfo.normal + RandomUnitVector(entropy));
        vec3 specularRayDirection =  reflect(rayDirection, hitInfo.normal);
        specularRayDirection = normalize(mix(specularRayDirection, diffuseRayDirection, hitInfo.materialInfo.roughness * hitInfo.materialInfo.roughness ));

        rayOrigin = hitPosition + hitInfo.normal * rayPosDelta;
        rayDirection = mix(diffuseRayDirection, specularRayDirection, doSpecular);

        //Add to the result
        result += hitInfo.emission * attenuation;
        attenuation *= mix(hitInfo.materialInfo.albedo, hitInfo.materialInfo.specularColor, doSpecular);

        //Divide by PDF
        float rayProbability = (doSpecular == 1.0f) ? specularChance : 1.0f - specularChance;
        rayProbability = max(rayProbability, 0.001f); 
        attenuation /= rayProbability;

        //Russian roulette
        float p = max(attenuation.r, max(attenuation.g, attenuation.b));
        if (RandomFloat01(entropy) > p)
            break;
        attenuation *= 1.0f / p;
    
    
    }
    return result;
}


uniform int width;
uniform int height; 
uniform uint frame;
uniform int pingPongInx;
uniform sampler2D oldTex;
void main()
{
    ivec2 fragCoord = ivec2(fragUv * vec2(width, height));

    uint rngState = uint(uint(fragCoord.x) * uint(1973) + uint(fragCoord.y) * uint(9277) + uint(frame) * uint(26699)) | uint(1);

    vec3 outColor = vec3(0,0,0);
    float invNumSample = 1.0f / float(numSamples);
    for(int i=0; i<numSamples; i++)
    {
        vec2 jitter = vec2(RandomFloat01(rngState), RandomFloat01(rngState)) - 0.5f;
        jitter /= vec2(width, height);
        vec3 filmPosition = vec3((fragUv + jitter) * 2.0f - 1.0f, -1);

        float aspectRatio = float(width) / float(height);
        filmPosition.y /= aspectRatio;

        vec3 rayOrigin = vec3(0,0,0);
        vec3 rayDirection = normalize(filmPosition - rayOrigin);

        rayOrigin = (viewMatrix * vec4(rayOrigin, 1.0f)).xyz;
        rayDirection = (viewMatrix * vec4(rayDirection, 0.0f)).xyz;

        vec3 color = Color(rayOrigin, rayDirection, rngState);
        
        
        vec3 lastFrameColor = texture(oldTex, fragUv).rgb;
        color = mix(lastFrameColor, color, 1.0f / float(frame+1));

        outColor += color * invNumSample;
    }    
    
    if(pingPongInx==0) outputColor0 = vec4(outColor, 1);
    else outputColor1 = vec4(outColor, 1);
}
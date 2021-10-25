#version 430

layout(location=0) out vec4 LPVGridR;
layout(location=1) out vec4 LPVGridG;
layout(location=2) out vec4 LPVGridB;

in vec3 rsmPosition;
in vec3 rsmNormal;
in vec4 rsmFlux;

#define PI 3.1415926f
#define cosLobe_C0 0.886226925f // sqrt(pi)/2 
#define cosLobe_C1 1.02332671f // sqrt(pi/3) 

uniform vec3 gridDim;

//Should I normalize the dir vector?
vec4 evalCosineLobeToDir(vec3 dir) {
	dir = normalize(dir);
	return vec4( cosLobe_C0, -cosLobe_C1 * dir.y, cosLobe_C1 * dir.z, -cosLobe_C1 * dir.x );
}

void main()
{
	//Set the color output with
	vec4 SHCoeffsR = evalCosineLobeToDir(rsmNormal) / PI * rsmFlux.r;
	vec4 SHCoeffsG = evalCosineLobeToDir(rsmNormal) / PI* rsmFlux.g;
	vec4 SHCoeffsB = evalCosineLobeToDir(rsmNormal) / PI * rsmFlux.b;

	LPVGridR = SHCoeffsR;
	LPVGridG = SHCoeffsG;
	LPVGridB = SHCoeffsB;
}
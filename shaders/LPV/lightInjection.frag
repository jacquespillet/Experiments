#version 430

layout(location=0) out vec4 LPVGridR;
layout(location=1) out vec4 LPVGridG;
layout(location=2) out vec4 LPVGridB;

in vec3 GSposFromRSM;
in vec3 GSnormalFromRSM;
in vec4 GSfluxFromRSM;

#define PI 3.1415926f

/*Spherical harmonics coefficients - precomputed*/
#define SH_C0 0.282094792f // 1 / 2sqrt(pi)
#define SH_C1 0.488602512f // sqrt(3/pi) / 2

/*Cosine lobe coeff*/
#define SH_cosLobe_C0 0.886226925f // sqrt(pi)/2 
#define SH_cosLobe_C1 1.02332671f // sqrt(pi/3) 

// SH_C0 * SH_cosLobe_C0 = 0.25000000007f
// SH_C1 * SH_cosLobe_C1 = 0.5000000011f

uniform vec3 gridDim;

//Should I normalize the dir vector?
vec4 evalCosineLobeToDir(vec3 dir) {
	dir = normalize(dir);
	//f00, f-11, f01, f11
	return vec4( SH_cosLobe_C0, -SH_cosLobe_C1 * dir.y, SH_cosLobe_C1 * dir.z, -SH_cosLobe_C1 * dir.x );
}

void main()
{
	//Set the color output with
	vec4 SHCoeffsR = evalCosineLobeToDir(GSnormalFromRSM) / PI * GSfluxFromRSM.r;
	vec4 SHCoeffsG = evalCosineLobeToDir(GSnormalFromRSM) / PI* GSfluxFromRSM.g;
	vec4 SHCoeffsB = evalCosineLobeToDir(GSnormalFromRSM) / PI * GSfluxFromRSM.b;

	LPVGridR = SHCoeffsR;
	LPVGridG = SHCoeffsG;
	LPVGridB = SHCoeffsB;
}
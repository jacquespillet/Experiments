#version 430

layout(location=0) out vec4 RAccumulatorLPV;
layout(location=1) out vec4 GAccumulatorLPV;
layout(location=2) out vec4 BAccumulatorLPV;
layout(location = 3) out vec4 RLightGridForNextStep;
layout(location = 4) out vec4 GLightGridForNextStep;
layout(location = 5) out vec4 BLightGridForNextStep;


// uniform sampler3D GeometryVolume;
uniform sampler3D LPVGridR;
uniform sampler3D LPVGridG;
uniform sampler3D LPVGridB;

uniform vec3 gridDim; //Resolution of the grid
flat in ivec3 GScellIndex;

const float directFaceSubtendedSolidAngle = 0.12753712; // 0.4006696846f / Pi;
const float sideFaceSubtendedSolidAngle = 0.13478556; // 0.4234413544f / Pi;


#define PI 3.1415926f
#define C0 0.282094792f // 1 / 2sqrt(pi)
#define C1 0.488602512f // sqrt(3/pi) / 2

/*Cosine lobe coeff*/
#define cosLobe_C0 0.886226925f // sqrt(pi)/2 
#define cosLobe_C1 1.02332671f // sqrt(pi/3) 

vec4 evaldirect( vec3 direction ) {	
	return vec4( C0, -C1 * direction.y, C1 * direction.z, -C1 * direction.x );
}

vec4 evalCosineLobeToDir_direct( vec3 direction ) {
	return vec4( cosLobe_C0, -cosLobe_C1 * direction.y, cosLobe_C1 * direction.z, -cosLobe_C1 * direction.x );
}


const ivec3 propDirections[6] = {
	//+Z
	ivec3(0,0,1),
	//-Z
	ivec3(0,0,-1),
	//+X
	ivec3(1,0,0),
	//-X
	ivec3(-1,0,0),
	//+Y
	ivec3(0,1,0),
	//-Y
	ivec3(0,-1,0)
};

//Sides of the cell - right, top, left, bottom
ivec2 cellSides[4] = { ivec2( 1.0, 0.0 ), ivec2( 0.0, 1.0 ), ivec2( -1.0, 0.0 ), ivec2( 0.0, -1.0 ) };

//Get side direction
vec3 getEvalSideDirection( int index, ivec3 orientation ) {
	const float smallComponent = 0.4472135; // 1 / sqrt(5)
	const float bigComponent = 0.894427; // 2 / sqrt(5)
	
	const ivec2 side = cellSides[ index ];
	vec3 tmp = vec3( side.x * smallComponent, side.y * smallComponent, bigComponent );
	return vec3(orientation.x * tmp.x, orientation.y * tmp.y, orientation.z * tmp.z);
}

vec3 getReprojSideDirection( int index, ivec3 orientation ) {
	const ivec2 side = cellSides[ index ];
	return vec3( orientation.x*side.x, orientation.y*side.y, 0 );
}

struct contribution {
	vec4 R,G,B;
};

float occlusionAmplifier = 1.0f;
contribution c;

void propagate() {
	c.R = vec4(0.0);
	c.G = vec4(0.0);
	c.B = vec4(0.0);

	//Propagate current cell to 6 neighbours

	for(int neighbour = 0; neighbour < 6; neighbour++) {
		vec4 RSHcoeffsNeighbour = vec4(0.0);
		vec4 GSHcoeffsNeighbour = vec4(0.0);
		vec4 BSHcoeffsNeighbour = vec4(0.0);
		
		//Get main direction
		ivec3 mainDirection = propDirections[neighbour]; 
		//get neighbour cell indexindex
		ivec3 neighbourGScellIndex = GScellIndex - mainDirection;
		
		//Load neighbour sh coeffs
		RSHcoeffsNeighbour = texelFetch(LPVGridR, neighbourGScellIndex,0);
		GSHcoeffsNeighbour = texelFetch(LPVGridG, neighbourGScellIndex,0);
		BSHcoeffsNeighbour = texelFetch(LPVGridB, neighbourGScellIndex,0);

		float occludedDirectFaceContribution = directFaceSubtendedSolidAngle;

		vec4 mainDirectionCosineLobeSH = evalCosineLobeToDir_direct( mainDirection );
		vec4 mainDirectionSH = evaldirect( mainDirection );
		c.R += occludedDirectFaceContribution * max(0.0, dot( RSHcoeffsNeighbour, mainDirectionSH )) * mainDirectionCosineLobeSH;
		c.G += occludedDirectFaceContribution * max(0.0, dot( GSHcoeffsNeighbour, mainDirectionSH )) * mainDirectionCosineLobeSH;
		c.B += occludedDirectFaceContribution * max(0.0, dot( BSHcoeffsNeighbour, mainDirectionSH )) * mainDirectionCosineLobeSH;

		for(int face = 0; face < 4; face++) {
			//Get the direction to the face
			vec3 evalDirection = getEvalSideDirection( face, mainDirection );
			//Reprojected direction
			vec3 reprojDirection = getReprojSideDirection( face, mainDirection );
			float occludedSideFaceContribution = sideFaceSubtendedSolidAngle;
			
			//Get sh coeff
			vec4 reprojDirectionCosineLobeSH = evalCosineLobeToDir_direct( reprojDirection );
			vec4 evalDirectionSH = evaldirect( evalDirection );
			
			c.R += occludedSideFaceContribution * max(0.0, dot( RSHcoeffsNeighbour, evalDirectionSH )) * reprojDirectionCosineLobeSH;
			c.G += occludedSideFaceContribution * max(0.0, dot( GSHcoeffsNeighbour, evalDirectionSH )) * reprojDirectionCosineLobeSH;
			c.B += occludedSideFaceContribution * max(0.0, dot( BSHcoeffsNeighbour, evalDirectionSH )) * reprojDirectionCosineLobeSH;

		}
	}
}

void main()
{
	propagate();

	RLightGridForNextStep = c.R;
	GLightGridForNextStep = c.G;
	BLightGridForNextStep = c.B;

	RAccumulatorLPV = c.R;
	GAccumulatorLPV = c.G;
	BAccumulatorLPV = c.B;
}

#version 430

layout(location = 0) in vec2 vPosition;

uniform sampler2D rsmWorldPosition;
uniform sampler2D rsmWorldNormal;
uniform sampler2D rsmFlux;

uniform int RSMsize;
uniform float cellSize;
uniform vec3 min; //min corner of the volume
uniform vec3 gridDim;

flat out ivec3 volumeCellIndex;
out vec3 posFromRSM;
out vec3 normalFromRSM;
out vec4 fluxFromRSM;


ivec3 convertPointToGridIndex(vec3 pos, vec3 normal) {
	return ivec3((pos - min) / cellSize + 0.5 * normal);
}

void main()
{
	//Get grid coordinate of the current vertex
	ivec2 v_RSMCoords = ivec2(gl_VertexID % RSMsize, gl_VertexID / RSMsize);

	//Fetch pos, normal and flux from the RSM
	posFromRSM = texelFetch(rsmWorldPosition, v_RSMCoords,0).rgb;
	normalFromRSM = texelFetch(rsmWorldNormal, v_RSMCoords,0).rgb;
	fluxFromRSM = texelFetch(rsmFlux, v_RSMCoords,0);

	//Convert from position to 3d integer cell index
	volumeCellIndex = convertPointToGridIndex(posFromRSM,normalFromRSM);
	
	//set screen position using x y coords
	vec2 screenPos = (vec2(volumeCellIndex.xy) + 0.5) / gridDim.xy * 2.0 - 1.0;
	gl_Position = vec4(screenPos , 0.0,1.0);
}
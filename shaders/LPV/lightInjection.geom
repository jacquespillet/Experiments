#version 430

layout(points )in;
layout(points ,max_vertices = 1 )out;

flat in ivec3 volumeCellIndex[];
in vec3 posFromRSM[];
in vec3 normalFromRSM[];
in vec4 fluxFromRSM[];

out vec3 rsmPosition;
out vec3 rsmNormal;
out vec4 rsmFlux;


void main(){
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = volumeCellIndex[0].z;

	rsmPosition = posFromRSM[0];
	rsmNormal = normalFromRSM[0];
	rsmFlux = fluxFromRSM[0];

	EmitVertex();
	EndPrimitive();
}

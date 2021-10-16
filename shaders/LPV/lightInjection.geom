#version 430

layout(points )in;
layout(points ,max_vertices = 1 )out;

flat in ivec3 volumeCellIndex[];
in vec3 posFromRSM[];
in vec3 normalFromRSM[];
in vec4 fluxFromRSM[];

out vec3 GSposFromRSM;
out vec3 GSnormalFromRSM;
out vec4 GSfluxFromRSM;


void main(){
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = volumeCellIndex[0].z;

	GSposFromRSM = posFromRSM[0];
	GSnormalFromRSM = normalFromRSM[0];
	GSfluxFromRSM = fluxFromRSM[0];

	EmitVertex();
	EndPrimitive();
}

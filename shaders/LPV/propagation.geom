#version 430

layout(points )in;
layout(points ,max_vertices = 1 )out;

flat in ivec3 cellIndex[];

flat out ivec3 GScellIndex;


void main(){
	//Set 3d texture position
	gl_Position=gl_in[0].gl_Position;
	gl_Layer = cellIndex[0].z;

	//Pass cell index
	GScellIndex = cellIndex[0];

	EmitVertex();
	EndPrimitive();

}
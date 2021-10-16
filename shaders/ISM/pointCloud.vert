 //attribs
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 barycentric;
layout(location = 2) in uint index;

uniform mat4 modelViewProjectionMatrix;
//main
void main()
{
	//Projected position
	gl_Position =  modelViewProjectionMatrix * vec4(position,1);
}
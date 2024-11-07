 //attribs
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 bitangent;
layout(location = 4) in vec2 uv;

out vec2 fragUv;

//main
void main()
{
	//Projected position
	gl_Position =  vec4(position, 1.0);
	fragUv = uv;
}
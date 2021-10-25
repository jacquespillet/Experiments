 //attribs
#version 440

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

uniform mat4 v;
uniform mat4 p;

out vec2 uv;
uniform float pointSize;
void main()
{
   vec3 Pos = gl_in[0].gl_Position.xyz;
   
   vec4 particlePositionModelSpace = vec4(Pos, 1.0f);
   vec4 particlePositionViewSpace = v *  particlePositionModelSpace;
   float dist = particlePositionViewSpace.z;

   gl_Position = p * particlePositionViewSpace + dist * 0.01 * vec4(-0.5f, -0.5f, 0.0f, 0.0f) * pointSize;
   uv = vec2(0, 0);
   EmitVertex();

   gl_Position = p * particlePositionViewSpace + dist * 0.01 * vec4(-0.5f, 0.5f, 0.0f, 0.0f) * pointSize;
   uv = vec2(0, 1);
   EmitVertex();

   gl_Position = p * particlePositionViewSpace + dist * 0.01 * vec4(0.5f, -0.5f, 0.0f, 0.0f) * pointSize;
   uv = vec2(1, 0);
   EmitVertex();

   gl_Position = p * particlePositionViewSpace + dist * 0.01 * vec4(0.5f, 0.5f, 0.0f, 0.0f) * pointSize;
   uv = vec2(1, 1);
   EmitVertex();


   EndPrimitive();
}
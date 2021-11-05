 //attribs
#version 440
layout(location = 0) in float dummy;

struct particle
{
   vec3 position;
   float pad0;
   vec3 velocity;
   float pad1;
   vec3 acceleration;
   float pad2;
};
layout (std430, binding = 10) buffer ParticlesBuffer {
   particle particles [];
};

//main
void main()
{
    vec4 position = vec4(particles[gl_VertexID].position, 1); 
    gl_Position = position;
}
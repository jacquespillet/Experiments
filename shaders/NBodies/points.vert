 //attribs
#version 440
layout(location = 0) in float dummy;

struct particle
{
   vec3 position;
   float r;
   vec3 velocity;
   float g;
   vec3 acceleration;
   float b;
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
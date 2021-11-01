 //attribs
#version 440
layout(location = 0) in float dummy;

struct Particle {
    vec4 color;
    vec3 pos;
    vec3 vel;
};


layout(std430, binding=0) restrict buffer ParticlesBuffer {
    Particle particles[];
};

out vec4 color;

//main
void main()
{
    vec4 position = vec4(particles[gl_VertexID].pos, 1); 
    gl_Position = position;

    vec3 vel = particles[gl_VertexID].vel;
    color = vec4(vel * 0.5 + 0.5, 1.0);
}
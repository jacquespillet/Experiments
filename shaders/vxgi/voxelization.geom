#version 330 core

#define M_PI 3.1415926535897932384626433832795

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vData {
    vec2 UV;
    vec4 position_depth;
} vertices[];

out fData {
    vec2 UV;
    mat4 projectionMatrix;
    flat int axis;
    vec4 position_depth;
} frag;

uniform mat4 ProjX;
uniform mat4 ProjY;
uniform mat4 ProjZ;

void main() {
    //Edges of the current triangle
    vec3 p1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    
    //Normal of the current triangle
    vec3 normal = normalize(cross(p1,p2));

    //Project the triangle on one of the 3 basis planes.
    float nDotX = abs(normal.x);
    float nDotY = abs(normal.y);
    float nDotZ = abs(normal.z);
    frag.axis = (nDotX >= nDotY && nDotX >= nDotZ) ? 1 : (nDotY >= nDotX && nDotY >= nDotZ) ? 2 : 3;
    frag.projectionMatrix = frag.axis == 1 ? ProjX : frag.axis == 2 ? ProjY : ProjZ;
    
    //Do the projection by multiplying with appropriate ortho projection matrix
    for(int i = 0;i < gl_in.length(); i++) {
        frag.UV = vertices[i].UV;
        frag.position_depth = vertices[i].position_depth;
        gl_Position = frag.projectionMatrix * gl_in[i].gl_Position;
        EmitVertex();
    }
    
    EndPrimitive();
}
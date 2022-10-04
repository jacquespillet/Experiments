#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vNormal[];
in vec2 vTexcoords[];
// flat in uint vTexture[];
flat in vec3 vColor[];

out vec2 gTexcoords;
out vec3 gNormal;
out vec3 gVoxelPos;

uniform int voxelRes;

vec2 Project(in vec3 v, in int axis) { return axis == 0 ? v.yz : (axis == 1 ? v.xz : v.xy); }

void main() {
    //Edges of the current triangle
    vec3 p1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
    
    //Normal of the current triangle
    vec3 normal = (normalize(cross(p1,p2)));

    //Project the triangle on one of the 3 basis planes.
    float nDotX = abs(normal.x);
    float nDotY = abs(normal.y);
    float nDotZ = abs(normal.z);
    int axis = (nDotX >= nDotY && nDotX >= nDotZ) ? 0 : (nDotY >= nDotX && nDotY >= nDotZ) ? 1 : 2;

	//project the positions
	vec3 pos0 = gl_in[0].gl_Position.xyz;
	vec3 pos1 = gl_in[1].gl_Position.xyz;
	vec3 pos2 = gl_in[2].gl_Position.xyz;

	gTexcoords = vTexcoords[0];
	gNormal = normalize(vNormal[0]);
	gVoxelPos = (pos0 + 1.0f) * 0.5f * voxelRes;
	gl_Position = vec4(Project(pos0, axis), 1.0f, 1.0f);
	EmitVertex();

	gTexcoords = vTexcoords[1];
	gNormal = normalize(vNormal[1]);
	gVoxelPos = (pos1 + 1.0f) * 0.5f * voxelRes;
	gl_Position = vec4(Project(pos1, axis), 1.0f, 1.0f);
	EmitVertex();
	
	gTexcoords = vTexcoords[2];
	gNormal = normalize(vNormal[2]);
	gVoxelPos = (pos2 + 1.0f) * 0.5f * voxelRes;
	gl_Position = vec4(Project(pos2, axis), 1.0f, 1.0f);
	EmitVertex();
	EndPrimitive();
}

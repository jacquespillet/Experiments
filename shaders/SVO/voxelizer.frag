#version 450 core

layout(binding = 1, offset = 0) uniform atomic_uint uCounter;

struct Voxel
{
	uvec3 position;
	uint color;
};
layout(std430, binding = 2) writeonly buffer VoxelList { Voxel voxelList[]; };

in vec2 gTexcoords;
in vec3 gNormal;
in vec3 gVoxelPos;

uniform int voxelRes, doVoxelize;
uniform sampler2D DiffuseTexture;

void main() {
	vec4 color = texture(DiffuseTexture, gTexcoords);
	if(color.w < 0.5f) discard;
	
	//Increments the voxel counter
	uint cur = atomicCounterIncrement(uCounter);
	
	//set fragment list
	if(doVoxelize == 1) {
		uvec3 voxelPosition = clamp(uvec3(gVoxelPos), uvec3(0u), uvec3(voxelRes - 1u));
		voxelList[cur].position = voxelPosition;
		uvec3 voxelColor = uvec3(round(color.rgb * 255.0f));
		voxelList[cur].color = (voxelColor.x | (voxelColor.y << 8u) | (voxelColor.z << 16u)); //pack the the color in an uint
	}
}

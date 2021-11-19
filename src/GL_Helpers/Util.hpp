#pragma once
#include "GL_Mesh.hpp"

struct AABB{
    glm::vec3 bounds[2];
};

GL_Mesh *MeshFromFile(std::string filename, bool swapYZ=false, int subMeshIndex=0);

void MeshesFromFile(std::string filename, std::vector<GL_Mesh*>* OutMeshes, std::vector<GL_Material*>* OutMaterials);

GL_Mesh *PlaneMesh(float sizeX, float sizeY, int subdivX, int subdivY);

void CreateComputeShader(std::string filename, GLint *programShaderObject);

void CreateComputeShader(const std::vector<std::string>& filenames, GLint *programShaderObject);

void BindSSBO(GLuint shader, GLuint ssbo, std::string name, GLuint bindingPoint);

float RandomFloat(float min, float max);

glm::vec3 RandomVec3(glm::vec3 min, glm::vec3 max);

int RandomInt(int min, int max);

float Clamp01(float input);

float Clamp(float input, float min, float max);

float Lerp(float a, float b, float f);

bool RayTriangleIntersection( 
    glm::vec3 &orig, glm::vec3 &dir, 
    glm::vec3 &v0,glm::vec3 &v1,glm::vec3 &v2,
    float *t, glm::vec2*uv) ;
    
bool RayAABBIntersection(glm::vec3 &orig, glm::vec3 &invDir, int *sign, AABB &aabb);


void CalculateTangents(std::vector<GL_Mesh::Vertex>& vertices, std::vector<uint32_t> &triangles);

GL_Mesh *GetQuad();
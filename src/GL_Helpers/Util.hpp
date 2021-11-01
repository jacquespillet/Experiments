#pragma once
#include "GL_Mesh.hpp"

GL_Mesh MeshFromFile(std::string filename, bool swapYZ=false, int subMeshIndex=0);

void MeshesFromFile(std::string filename, std::vector<GL_Mesh*>* OutMeshes, std::vector<GL_Material*>* OutMaterials);

GL_Mesh *PlaneMesh(float sizeX, float sizeY, int subdivX, int subdivY);

void CreateComputeShader(std::string filename, GLint *programShaderObject);

void CreateComputeShader(const std::vector<std::string>& filenames, GLint *programShaderObject);

void BindSSBO(GLuint shader, GLuint ssbo, std::string name, GLuint bindingPoint);

float RandomFloat(float min, float max);

int RandomInt(int min, int max);

float Clamp01(float input);

float Clamp(float input, float min, float max);

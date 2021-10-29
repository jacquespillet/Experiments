#pragma once
#include "GL_Mesh.hpp"

GL_Mesh MeshFromFile(std::string filename, bool swapYZ=false, int subMeshIndex=0);

void MeshesFromFile(std::string filename, std::vector<GL_Mesh*>* OutMeshes, std::vector<GL_Material*>* OutMaterials);

GL_Mesh *PlaneMesh(float sizeX, float sizeY, int subdivX, int subdivY);

void CreateComputeShader(std::string filename, GLint *programShaderObject);

void BindSSBO(GLuint shader, GLuint ssbo, std::string name, GLuint bindingPoint);
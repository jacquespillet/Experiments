#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"
#include <glm/gtx/hash.hpp>
#include <unordered_map>

struct SubdivVertex
{
    glm::vec3 position;
    std::vector<int> edges;
    std::vector<int> faces;
};

struct SubdivEdge
{
    int lowVertex=-1;
    int highVertex=-1;
    int faceCount=0;
    int faces[2]={-1,-1};
};

struct SubdivFace
{
    int vertices[3]={-1,-1,-1};
    int edges[3]={-1,-1,-1};
};

struct SubdivModel
{
    std::vector<SubdivVertex> vertices;
    std::vector<SubdivEdge> edges;
    std::vector<SubdivFace> faces;

    //Mappings
    
    //Contains the index of a given position in the vertices list
    std::unordered_map<glm::vec3, int> positionToVertex;

    //Contains the index of a given edge into the edges list
    std::unordered_map<glm::ivec2, int> vertexPairToEdge;

    void MoveVertex(int vertexIndex, const glm::vec3& newPosition);

    int GetVertexIndex(const glm::vec3 &position);

    int GetEdgeIndex(const glm::ivec2& vertexPair);
};

class MeshManipulation : public Demo {
public : 
    MeshManipulation();
    void Load();
    void Render();
    void RenderGUI();
    void Unload();

    void MouseMove(float x, float y);
    void LeftClickDown();
    void LeftClickUp();
    void RightClickDown();
    void RightClickUp();
    void Scroll(float offset);


    SubdivModel PrepareMesh(GL_Mesh *mesh);
    GL_Mesh* ApplySubdivision(SubdivModel& model);
private:
    clock_t t;
    float deltaTime;
    float elapsedTime;
        
    GL_Camera cam;

    GL_Shader MeshShader;
    GL_Shader WireframeShader;

    std::vector<GL_Mesh*> Meshes;
    std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    bool specularTextureSet=false;
    bool normalTextureSet=false;

    bool wireframe=false;
    bool solid=true;
   
};
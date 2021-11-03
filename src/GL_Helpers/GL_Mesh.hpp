#pragma once

#include "Common.h"

#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Camera.hpp"
#include "GL_Material.hpp"
class GL_Mesh {
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tan;
        glm::vec3 bitan;
        glm::vec2 uv;
    }; 

    GL_Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> triangles);
    GL_Mesh();
    
    void Unload();

    void Render(const GL_Camera &camera, GLuint shader);
    void RenderDepthOnly(const glm::mat4& viewProjection, GLuint shader);
    void RenderTo3DTexture(const glm::mat4& viewProjection, GLuint shader);
    void RenderShader(GLuint shader);
    // void Render();

    void Init();
    void Destroy();
    // std::shared_ptr<GL_Material> material;
    
    bool inited;

    void Translate(glm::vec3 dPos);
    void Rotate(glm::vec3 dRot);
    void Scale(glm::vec3 dScale);

    void SetPos(glm::vec3 pos);
    void SetRot(glm::vec3 rot);
    void SetScale(glm::vec3 scale);

    void RecalculateMatrices();

    glm::vec3 GetMinBoundingBox();
    glm::vec3 GetMaxBoundingBox();
    void RecalculateBoundingBox();

    void UpdateGPUBuffers();
    
    void SetModelMatrix(glm::mat4 _modelMatrix) {
        this->modelMatrix = _modelMatrix;
        this->invModelMatrix = glm::inverse(modelMatrix);
    }

    glm::vec3 GetScale() {return scale;}
    glm::vec3 GetPosition() {return position;}

    glm::mat4 modelMatrix;
    glm::mat4 invModelMatrix;


    // GL_Shader shader;
    GL_Material *material;

    //CPU buffers
    std::vector<Vertex> vertices;
    std::vector<unsigned int> triangles;

    glm::vec3 bbMin, bbMax;

    bool visible=true;
private:    
    //GPU buffers
    unsigned int vertexBuffer;
    unsigned int elementBuffer;
    unsigned int vertexArrayObject;


    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};


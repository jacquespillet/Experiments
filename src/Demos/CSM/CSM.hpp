#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class CSM : public Demo {
public : 
    CSM();
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

private:
    clock_t t;
    float deltaTime;
    float elapsedTime;
        
    GL_Camera cam;

    GL_Shader MeshShader;

    std::vector<GL_Mesh*> Meshes;
    std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    std::vector<glm::mat4> lightViewMatrices;
    
    void InitCSM();
    // int numCascades=4;
    GLsizei depthMapResolution = 2048;
    GLuint lightDepthMap;
    GLuint lightFramebuffer;
    GLuint matricesUBO;
    GL_Shader shadowMapShader;

    std::vector<float> shadowCascadeLevels;
    std::vector<glm::mat4> lightMatrices;
    std::vector<glm::vec4> ComputeWorldSpaceCorners(const glm::mat4& projectionMatrix, const glm::mat4 &viewMatrix);
    glm::mat4 ComputeLightVPMatrix(std::vector<glm::vec4>& corners);
    void ComputeLightSpaceMatrices();
    
    int numCascades=4;
    float zMultiplicator = 10.0f;
    float bias = 0.0005f;
    bool viewCascades=false;
    
};
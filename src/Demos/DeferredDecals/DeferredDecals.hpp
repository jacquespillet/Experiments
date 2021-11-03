#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"
#include "GL_Helpers/Util.hpp"
class DeferredDecals : public Demo {
public : 
    DeferredDecals();
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
    std::vector<AABB> aabbs;
    std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    void InitGeometryBuffer();
    GLuint framebuffer = (GLuint)-1;
    GLuint normalTexture = (GLuint)-1;
    GLuint positionTexture = (GLuint)-1;
    GLuint colorTexture = (GLuint)-1;
    GLuint depthTexture = (GLuint)-1;
    
    void InitDecalFramebuffer();
    GLuint decalFramebuffer = (GLuint)-1;
    GLuint decalRBO;
    

    void InitQuad();
    GL_Mesh screenspaceQuad;
    GL_Shader quadShader;
    
    void InitDecal();
    GL_Shader decalShader;
    std::vector<GL_Mesh*> decals;
    GL_Mesh *decalBox;

    void InstantiateDecal();

    GL_Texture decalTexture;

    float decalSize = 1;
};
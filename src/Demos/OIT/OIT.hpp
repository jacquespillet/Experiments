#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class OIT : public Demo {
public : 
    OIT();
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

    std::vector<GL_Mesh*> SolidMeshes;
    std::vector<GL_Mesh*> TransparentMeshes;
    std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    void InitOIT();
    GLuint opaqueFramebuffer;
    GLuint opaqueTexture;
    GLuint opaqueDepthTexture;
    GLuint transparentFramebuffer;
    GLuint accumulationTexture;
    GLuint revealTexture;
    GL_Mesh screenSpaceQuad;
    GL_Shader accumulationShader;
    GL_Shader compositionShader;
    GL_Shader resolveShader;

    float alpha = 0.1f;
};
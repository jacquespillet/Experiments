#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class SSSS : public Demo {
public : 
    SSSS();
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
    GL_Shader DepthShader;

    GL_Mesh* Mesh;
    GL_Material* Material;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    void LoadFramebuffer();
    GLuint framebuffer;
    GLuint depthTexture;
    GLuint linearDepthTexture;

    void LoadBlurElements();
    GLuint blurTexture;
    GLint blurComputeShader;
    int blurPasses=1;
    void BlurTexture();
    
    float distortion = 0.897f;
    float scale = 1.256f;
    int power = 1;
};
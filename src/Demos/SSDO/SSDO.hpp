#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class SSDO : public Demo {
public : 
    SSDO();
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

    std::vector<GL_Mesh*> Meshes;
    std::vector<GL_Material*> Materials;
    GL_Mesh* dragon;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    void InitGeometryBuffer();
    GLuint framebuffer;
    GLuint colorTexture;
    GLuint normalTexture;
    GLuint positionTexture;
    GLuint depthTexture;
    GL_Shader renderGBufferShader;
    GL_Shader resolveGBufferShader;
    GL_Mesh screenSpaceQuad;

    void InitSSAO();
    GLuint sampleKernelBuffer;
    GLuint rotationVectorsTexture;
    GLuint ssaoFramebuffer;
    GLuint ssaoTexture;
    GL_Shader ssaoShader;
    GLuint blurFramebuffer;
    GLuint blurredSsaoTexture;
    GL_Shader blurShader;

    void LoadHDR();
    GLuint envTexture;

    int kernelSize = 64;
    float strength = 3;
    float radius = 0.615f;
    float indirectRadiance = 10.0f;
    bool renderSSAO=true;
    bool ssaoEnabled=true;
};
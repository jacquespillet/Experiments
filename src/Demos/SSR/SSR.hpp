#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class SSR : public Demo {
public : 
    SSR();
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

    void InitSSR();
    GLuint ssrFramebuffer;
    GLuint ssrTexture;
    GL_Shader ssrShader;
    GL_Texture noiseTexture;
    
    GLint blurComputeShader;
    GLuint blurTexture;

    int blurCount = 6;
    float bias = 2.0f;

};
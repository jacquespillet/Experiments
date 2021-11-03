#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class PathTracer : public Demo {
public : 
    PathTracer();
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

    GL_Mesh screenSpaceQuad;
    GL_Shader renderingShader;
    GL_Shader textureShader;

    GLuint framebuffer, rbo;
    GLuint pingPongTextures[2];
    int pingPongInx=0;

    uint32_t frame=0;

    glm::vec3 lightDirection;
    bool lightDirectionChanged=false;
};
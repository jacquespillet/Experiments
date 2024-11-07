#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class SVGF : public Demo {
public : 
    SVGF();
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
    GL_Shader textureShader;

    GLuint texture;
    GLuint textureNormal;
    GLuint textureFiltered;
    GLuint textureAA;

    uint32_t frame=0;
    float time=0;

    glm::vec3 lightDirection;
    bool lightDirectionChanged=false;

    GLint pathTraceShader;
    GLint filterShader;
    GLint taaShader;
    bool compute=true;

    int output=0;
};
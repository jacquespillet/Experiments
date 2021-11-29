#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class KuwaharaFilter : public Demo {
public : 
    KuwaharaFilter();
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

    GL_Shader MeshShader;
    GL_Mesh* Quad;
    GL_Texture texture;
    
    void ReloadK0();
    int k0Size = 32;
    GLuint K0Texture;
    GLuint K0TextureView;
    GL_Texture K0Colorizer;

    int radius=1;
    float smoothing = 0.5f;
    float q = 8;
    bool doDecay = true;

    const char* modes[4] = { "Original","Kuwahara", "Generalized Kuwahara", "Anisotropic Kuwahara"};
    int currentMode = 1;
};
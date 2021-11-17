#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class ShellFur : public Demo {
public : 
    ShellFur();
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

    GL_Material *Material;
    GL_Mesh *Mesh;
    // std::vector<GL_Mesh*> Meshes;
    // std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;


    void InitFur();
    void Reload();
    int furTexSize = 256;
    int numLayers = 10;
    float length=0.01f;
    float density = 0.5;
    float tiling=15;
    GLuint furTexture;
    GL_Texture furColor;
};
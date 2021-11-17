#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class DispMapping : public Demo {
public : 
    DispMapping();
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
    GL_Mesh* Mesh;
    GL_Material* Material;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;
    
    float strength = 0.05f;

    GL_Texture texTest;
    void GenerateMipMaps(uint8_t *data, int width, int height);
};
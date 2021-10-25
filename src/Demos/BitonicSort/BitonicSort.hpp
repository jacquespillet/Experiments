#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class BitonicSort : public Demo {
public : 
    BitonicSort();
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

    void showValuesOnConsole();
    void Reset();
    void Sort();

private:
    clock_t t;
    float deltaTime;
    float elapsedTime;
        
    void InitBuffers();


    GLint sortShader;
    GLint matrixTransposeShader;

    GLuint inBuffer;
    GLuint tmpBuffer;
    int bufferSize = 8192;

    uint32_t BITONIC_BLOCK_SIZE = 512;
    uint32_t TRANSPOSE_BLOCK_SIZE = 16;

    // struct particle
    // {
    //     glm::uvec4 index;
    // };

};
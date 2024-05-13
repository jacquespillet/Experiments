
#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"
#include <complex>

typedef std::complex<double> complex;
class Ocean_FFT : public Demo {
public : 
    Ocean_FFT();
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
    float deltaTime=0;
    float elapsedTime=0;

    GL_Camera cam;

    uint32_t resolution = 512;
    float windSpeed = 40.0f; 
    float g = 9.81f;
    float L = 1000;
    float A = 4;
    glm::vec2 windDirection;
    glm::vec3 lightDirection;
    float planeSize = 20;
    float metallic = 0.26f;
    float roughness = 0.3f;
    float lightIntensity = 0.7f;
    float ambient = 1;
    float height = 2.227;

    std::vector<glm::vec4> h0kBuffer;
    std::vector<glm::vec4> h0MinuskBuffer;
    GLuint h0kTex, h0MinuskTex;
    void InitInitialHkTextures();


    GLint butterflyTextureComputeShader;
    GLuint indicesBuffer;
    GLuint butterflyTexture;
    void InitButterflyTexture();

    
    GLint ifftComputeShader;
    GLuint pingPongTexture;
    void InitFFTTextures();
    void IFFTGPU();

    
    GLint displacementComputeShader;
    GLuint displacementTexture;
    GLint waveComputeShader;
    GLint normalsComputeShader;
    GLuint hktTexture;
    GLuint normalTexture;
    void InitDisplacementTextures();


    GL_Shader oceanShader;
    GL_Mesh *oceanMesh;
};
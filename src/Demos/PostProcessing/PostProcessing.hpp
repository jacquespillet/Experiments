#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

struct PostProcess
{
    PostProcess(std::string shaderFileName);
    virtual void Process(GLuint textureIn, GLuint textureOut, int width, int height);
    virtual void SetUniforms();
    std::string shaderFileName;
    GLint shader;
};

struct PostProcessStack
{
    std::vector<PostProcess*> postProcesses;
    GLuint Process(GLuint textureIn, GLuint textureOut, int width, int height);
};


struct GrayScalePostProcess : public PostProcess
{
    GrayScalePostProcess();
    void SetUniforms() override;
};

class PostProcessing : public Demo {
public : 
    PostProcessing();
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

    void InitGeometryBuffer();
    GLuint framebuffer;
    GLuint colorTexture;
    GLuint normalTexture;
    GLuint positionTexture;
    GLuint depthTexture;
    GL_Shader renderGBufferShader;
    GL_Shader resolveGBufferShader;
    GL_Mesh screenSpaceQuad;
    GLuint postProcessTexture;

    glm::vec3 lightDirection;

    PostProcessStack postProcessStack;

    bool lightDirectionChanged=false;

    bool specularTextureSet=false;
    bool normalTextureSet=false;
};
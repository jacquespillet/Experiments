#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

struct PostProcess
{
    PostProcess(std::string name, std::string shaderFileName);
    virtual void Process(GLuint textureIn, GLuint textureOut, int width, int height);
    virtual void SetUniforms();
    virtual void RenderGui();
    std::string shaderFileName;
    std::string name;
    GLint shader;

    bool active=true;
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
    void RenderGui() override;
    float saturation = 0.0f;
};

struct ContrastBrightnessPostProcess : public PostProcess
{
    ContrastBrightnessPostProcess();
    void SetUniforms() override;
    void RenderGui() override;
    float contrastIntensity=1;
    float brightnessIntensity=1;
    glm::vec3 contrast = glm::vec3(1.0f);
    glm::vec3 brightness = glm::vec3(0.0f);
};

struct ChromaticAberationPostProcess : public PostProcess
{
    ChromaticAberationPostProcess();
    void SetUniforms() override;
    void RenderGui() override;
    glm::vec3 offsets = glm::vec3(0,0,0);
    glm::vec2 direction = glm::vec2(1,1);
    bool aroundMouse=false;
};

struct PixelizePostProcess : public PostProcess
{
    PixelizePostProcess();
    void SetUniforms() override;
    void RenderGui() override;
    int pixelSize=5;
};

struct ToneMappingPostProcess : public PostProcess
{
    ToneMappingPostProcess();
    void SetUniforms() override;
    void RenderGui() override;
    float exposure = 1;
    int type;
};

struct DepthOfFieldPostProcess : public PostProcess
{
    DepthOfFieldPostProcess(GLuint positionsTexture,int width, int height);
    void SetUniforms() override;
    void RenderGui() override;
    void Process(GLuint textureIn, GLuint textureOut, int width, int height) override;

    void Blit(GLuint source, GLuint target, int targetWidth, int targetHeight);

    float focusRange=35.0f;
    float focusDistance = 5.0f;
    float bokehSize = 4 ;

    GLuint positionTexture;

    GLuint bokehTextureIn;
    GLuint bokehTextureOut;

    GLint blitShader;
    GLint cocShader;
    GLint prefilterCocShader;
    GLint combineShader;
    
    GLuint cocTexture;
    GLuint prefilteredCocTexture;
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
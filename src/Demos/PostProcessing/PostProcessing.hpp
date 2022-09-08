#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class PostProcessing;
struct PostProcess
{
    PostProcess(std::string name, std::string shaderFileName, bool enabled);
    virtual void Process(GLuint textureIn, GLuint textureOut, int width, int height);
    virtual void SetUniforms();
    virtual void RenderGui();
    std::string shaderFileName;
    std::string name;
    GLint shader;
    PostProcessing *demo;
    bool enabled=true;
};
struct PostProcessStack
{
    PostProcessStack(PostProcessing* demo);
    std::vector<PostProcess*> postProcesses;
    GLuint Process(GLuint textureIn, GLuint textureOut, int width, int height);
    PostProcessing *demo;
    void AddProcess(PostProcess *Process);
};


struct GrayScalePostProcess : public PostProcess
{
    GrayScalePostProcess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    float saturation = 0.0f;
};

struct ContrastBrightnessPostProcess : public PostProcess
{
    ContrastBrightnessPostProcess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    float contrastIntensity=1;
    float brightnessIntensity=1;
    glm::vec3 contrast = glm::vec3(1.0f);
    glm::vec3 brightness = glm::vec3(0.0f);
};

struct ChromaticAberationPostProcess : public PostProcess
{
    ChromaticAberationPostProcess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    glm::vec3 offsets = glm::vec3(0,0,0);
    glm::vec2 direction = glm::vec2(1,1);
    bool aroundMouse=false;
};

struct PixelizePostProcess : public PostProcess
{
    PixelizePostProcess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    int pixelSize=5;
};

struct GodRaysPostProcess : public PostProcess
{
    GodRaysPostProcess(glm::vec3 *lightPosition, glm::mat4 *viewMatrix, glm::mat4 *projectionMatrix, bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    void Process(GLuint textureIn, GLuint textureOut, int width, int height) override;

    glm::vec3 *lightPosition;
    glm::mat4 *viewMatrix;
    glm::mat4 *projectionMatrix;

    float density = 1.0f;
    float weight=0.01f;

    float decay = 0.99f;
    int numSamples = 64;
};

struct ToneMappingPostProcess : public PostProcess
{
    ToneMappingPostProcess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    float exposure = 1;
    int type;
};

struct SharpenPostProcess : public PostProcess
{
    SharpenPostProcess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    float strength = 1;
};

struct VignettePostProcess : public PostProcess
{
    VignettePostProcess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    float roundness = 0.2f;
    float opacity = 0.2f;
};

struct GrainPostProcess : public PostProcess
{
    GrainPostProcess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    float strength=0.1f;
    float seed = 95486;
    int multiplier = 10000;
    bool dynamic=true;
};

struct CRTPostPRocess : public PostProcess
{
    CRTPostPRocess(bool enabled=true);
    void SetUniforms() override;
    void RenderGui() override;
    glm::vec2 curvature = glm::vec2(3,3);
    glm::vec2 scanLineOpacity = glm::vec2(0.5,0.5);
    glm::vec2 resolution = glm::vec2(1,1);
    float brightness=1;
};

struct DepthOfFieldPostProcess : public PostProcess
{
    DepthOfFieldPostProcess(GLuint positionsTexture,int width, int height,bool enabled=true);
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

    float elapsedTime;
private:
    clock_t t;
    float deltaTime;
        
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

    glm::vec3 lightPosition;
    float lightIntensity;

    PostProcessStack postProcessStack;

    bool specularTextureSet=false;
    bool normalTextureSet=false;
};
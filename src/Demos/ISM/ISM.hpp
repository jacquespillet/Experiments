#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"


class ISM : public Demo {
public : 

    struct point {
        glm::vec3 position;
        glm::vec2 barycentric;
        uint32_t index;
    };

    struct pointcloud
    {
        std::vector<point> points;
        GLuint VBO;
        GLuint VAO;
        GL_Shader renderShader;
    };

    struct ImperfectShadowMap
    {
        GLuint framebuffer;
        struct textures {
            GLuint positionTexture;
            GLuint normalTexture;
            GLuint fluxTexture;
        } textures;
        GLuint depthTexture;
        GL_Shader renderShader;
        glm::mat4 depthViewProjectionMatrix;
        int width=4096;
        int height=4096;
        float orthoSize = 2;
    };

    struct GeometryBuffer
    {
        GL_Shader renderShader;
        GLuint framebuffer;
        GLuint depthStencilAttachment;
        struct textures {
            GLuint colorTexture;
            GLuint normalTexture;
            GLuint positionTexture;
        } textures;
        int width = 512;
        int height = 512;
    };

    struct DeferredRenderer
    {
        GeometryBuffer geometryBuffer;
        GL_Mesh screenSpaceQuad;
        GL_Shader renderingShader;
    };

    ISM();
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

    std::vector<GL_Mesh*> Meshes;
    std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;
    bool lightDirectionChanged=false;

    // void GeneratePointCloud();
    // void RenderPointCloud();
    // pointcloud pointCloud;

    bool CreateReflectiveShadowMap();
    void DrawSceneToReflectiveShadowMap();
    ImperfectShadowMap ReflectiveShadowMap;
    
    uint32_t numSamples = 256;
    std::vector<glm::vec2> samples;
    GLuint randomTexture;
    void CreateSamplingPattern();
    GL_Shader sphereShader;
    GL_Mesh sphere;
    void CreateSphere();
    
    void CreateGeometryBuffer();
    // GeometryBuffer geometryBuffer;
    void DrawSceneToGeometryBuffer();
    void CreateDeferredRenderer();
    DeferredRenderer deferredRenderer;

    // bool renderScene=true;
    // bool renderPointCloud=true;
};
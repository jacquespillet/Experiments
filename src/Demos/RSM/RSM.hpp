#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class RSM : public Demo {
public : 
    struct ReflectiveShadowMap
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
    RSM();
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

    GLuint randomTexture;
    void CreateRandomTexture();
    bool InitReflectiveShadowMap();
    void DrawSceneToReflectiveShadowMap();
    ReflectiveShadowMap ReflectiveShadowMap;

    void CreateLowResGeometryBuffer();
    GeometryBuffer lowResGeometryBuffer;
    void DrawSceneToGeometryBuffer();

    const int randomTextureSize = 512;
    float maxDistance = 0.012f;
    float intensity = 7.5;
    int numSamples = 400;
    bool showDirect=false;
    bool showIndirect=true;
};
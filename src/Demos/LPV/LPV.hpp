#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class LPV : public Demo {
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
        int width=256;
        int height=256;
    };


    struct ShadowMap
    {
        GLuint depthFramebuffer;
        GL_Texture depthTexture;
        GL_Shader shadowShader;
        glm::mat4 depthViewProjectionMatrix;
        int resolution=2048;
    };

    LPV();
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
    float orthoSize = 40;

    GL_Shader MeshShader;

    std::vector<GL_Mesh*> Meshes;
    std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    bool InitReflectiveShadowMap();
    void DrawSceneToReflectiveShadowMap();
    ReflectiveShadowMap ReflectiveShadowMap;

    bool InitShadowMap();
    void DrawSceneToShadowMap();
    ShadowMap shadowMap;

    glm::vec3 bbMin;
    glm::vec3 bbMax;
    glm::vec3 size;

    int texture3DResolution = 32;
    GLuint lightInjectionR, lightInjectionG, lightInjectionB;
    GLuint lightInjectionFramebuffer;
    GL_Shader lightInjectionShader;
    GLuint VPLVAO, VPLVBO;
    void InitLightInjectionTextures();
    void InitLightInjectionBuffers(); 
    void ClearLightInjectionTextures();
    void InjectLight();   


    GLuint lightPropagationR, lightPropagationG, lightPropagationB;
    GL_Shader lightPropagationShader;
    GLuint propagationVAO, propagationVBO;
    std::vector<GLuint> propagationTexturesRed;
    std::vector<GLuint> propagationTexturesGreen;
    std::vector<GLuint> propagationTexturesBlue;
    std::vector<GLuint> propagationFramebuffers;
    int numPropagationSteps=7;
    void InitLightPropagationTextures();
    void InitLightPropagationBuffers();
    void ClearLightPropagationTextures();
    void PropagateLight();


    bool displayGI=true;
    bool displayShadows=true;
    bool displayDirectDiffuse=true;
    float ambient=0.01f;
    float specular=0.1f;
    float GIIntensity=1.0f;
    
};
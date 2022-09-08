#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"


class SVO : public Demo {
public : 
    SVO();
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

    struct ShadowMap
    {
        GLuint depthFramebuffer;
        GL_Texture depthTexture;
        GL_Shader shadowShader;
        glm::mat4 depthViewProjectionMatrix;
        int resolution=4096;
    };

    struct VoxelScene
    {
        GL_Shader voxelizationShader;
        GL_Texture3D voxelTexture;
        const int voxelDimensions = 512;
        const float voxelGridWorldSize = 150.0f;
        glm::mat4 projX, projY, projZ;
        bool voxelInitialized=false;
    };

    clock_t t;
    float deltaTime;
    float elapsedTime;
        
    GL_Camera cam;

    GL_Shader MeshShader;

    std::vector<GL_Mesh*> Meshes;
    std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;

    //Shadow map
    bool InitShadowMap();
    void DrawSceneToShadowMap();
    ShadowMap shadowMap;

    //Voxel scene
    void Init3DTex();
    void voxelizeScene();
    void BuildOctree();
    VoxelScene voxelScene;


    bool showOcclusion=true;
    bool showIndirectDiffuse=true;
    bool showDirectDiffuse=true;
    bool showAmbient=true;
    bool showSpecular=true;

    float directAmbient=0.02f;
    float specularity=0.2f;

    bool lightDirectionChanged=false;

};
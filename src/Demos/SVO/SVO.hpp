#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"


struct DispatchIndirectArgs
{
    GLuint numGroupsX, numGroupsY, numGroupsZ;
};

struct BuildInfo
{
    GLuint fragmentCount;
    GLuint voxelResolution;
    GLuint allocBegin;
    GLuint allocNum;
};

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
    clock_t t;
    float deltaTime;
    float elapsedTime;
        
    GL_Camera cam;

    GL_Shader MeshShader;
    bool renderMesh=false;

    std::vector<GL_Mesh*> Meshes;
    std::vector<GL_Material*> Materials;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    bool specularTextureSet=false;
    bool normalTextureSet=false;

    void SynchronizeGPU();

    GL_Shader renderShader;
    GL_Mesh *quad;

    struct
    {
        int resolution;
        GL_Shader shader;

        int fragmentNum;

        struct
        {
            GLuint buffer;
        } FragmentList;

        struct
        {
            GLuint buffer;
            GLuint *mappedMemory;
        } Counter;

        struct
        {
            GLuint rbo;
            GLuint fbo;
        } Framebuffer;
    } Voxelizer;

    struct
    {
        GLint tagNodeShader;
        GLint allocNodeShader;
        GLint modifyArgsShader;

        
        struct
        {
            GLuint buffer;
            GLuint *mappedMemory;
        } Counter;

        GLuint allocIndirectBuffer;
        GLuint buildInfoBuffer;
    } OctreeBuilder;

    struct
    {
        int levels;
        GLuint buffer;
    } Octree;
};
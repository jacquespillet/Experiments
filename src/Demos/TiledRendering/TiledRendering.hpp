#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

struct PointLight
{
    glm::vec3 position;
    float radius;
    glm::vec4 color;
    glm::vec4 currentPosition;
};

class TiledRendering : public Demo {
public : 
    TiledRendering();
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

    
    glm::vec3 bbMin, bbMax;

    void InitDepthPrepassBuffers();
    GL_Shader depthPrepassShader;
    GLuint depthPrepassFramebuffer;
    GLuint depthTexture;

    void InitLights();
    std::vector<PointLight> lights;
    GLuint lightBuffer;

    //Forward+
    void InitForwardPlus();
    GLint tileComputeShader;
    GLuint tileLightsBuffer;
    int tileSize = 16;
    int numTilesX;
    int numTilesY;
    int numTiles;
    void ComputeTiles();

    //Deferred
    void InitDeferred();
    GLuint deferredFramebuffer;
    GLuint gcolorTexture;
    GLuint gpositionTexture;
    GLuint gdepthTexture;
    GLuint gnormalTexture;
    GL_Shader renderGBufferShader;
    GL_Shader resolveGBufferShader;
    GL_Mesh screenSpaceQuad;
    void RenderGBuffer();
    void ResolveGBuffer();

    //Forward clustered
    struct Cluster
    {
        glm::vec3 min;
        int active;
        glm::vec3 max;
        float pad1;
    };
    struct GridIndex {
        int startIndex;
        int size;
        int pad[2];
    };
    void InitForwardClustered();
    GLint buildClustersComputeShader;
    GLint clusteredLightCullingComputeShader;
    GLint findActiveClustersComputeShader;
    GLint buildClusterListComputeShader;
    GLint activeClusteredLightCullingComputeShader;
    GLuint clustersBuffer;
    GLuint clusterLightIndicesBuffer;
    GLuint clusterGridIndicesBuffer;
    GLuint clusterLightCountBuffer;
    GLuint activeClustersBuffer;
    GLuint activeCounterBuffer;

    GLint animateLightsShader;
    void AnimateLights();
    
    int clusterX = 16;
    int clusterY = 9;
    int clusterZ = 24;
    int clusterPixelSizeX;
    int totalClusters;
    bool prepassClusters=true;
    void FindActiveClusters();
    void BuildClusters();
    void ComputeClusters();
    void ComputeActiveClusters();

    int numLights = 1024;
    bool animated=true;
    const char* modes[4] = { "Forward","Forward+", "Deferred", "Forward Clustered"};
    int currentMode = 1;
    
};
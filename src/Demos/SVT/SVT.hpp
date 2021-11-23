#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"
#include <unordered_map>

class SVT : public Demo {
public : 
    SVT();
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

    GL_Mesh *Mesh;
    GL_Material *Mat;

    glm::vec3 lightDirection;

    bool lightDirectionChanged=false;

    int virtualTextureWidth, virtualTextureHeight;

    void BuildPages();
    int pageSize = 64;
    int numPagesX;
    int numPagesY;
    int numMipmaps;

    void BuildVisibilityFramebuffer();
    int downSampleFactor=8;
    int visibilityFramebufferWidth;
    int visibilityFramebufferHeight;
    GLuint visibilityFramebuffer;
    GL_Shader visibilityShader;
    GLuint visibilityTexture;
    GLuint rbo;

    void LoadVisiblePages();
    struct rgba {uint8_t r, g, b, a;};
    std::vector<rgba> pixmap; //used for readback
    
    struct PageInfo {
        uint64_t frame; 
        uint32_t tablePosition;
    };

    //Key : pageID
    //Value : second: frame at which it was added, and position in the physical texture where it is
    std::vector<std::unordered_map<uint32_t, PageInfo>> presentPages; 
    
    void BuildPhysicalTextures();
    const int pagesPerLine = 20;
    int physicalTextureSizeX = pageSize * pagesPerLine;
    int physicalTextureSizeY = pageSize * pagesPerLine;
    GLuint physicalTexture;
    std::vector<std::vector<rgba>> pageTable;
    std::vector<std::vector<rgba>> pageTableFilled;
    GLuint pageTableTexture;

    uint32_t lastUsedIndex=0;


    uint64_t frame;

    float sampleMipmap=0;

    int limitAddPerFrame = 1;

    bool doSpread=true;
};
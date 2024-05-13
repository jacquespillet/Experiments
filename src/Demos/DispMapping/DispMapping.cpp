#include "DispMapping.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
DispMapping::DispMapping() {
}

void DispMapping::GenerateMipMaps(uint8_t *data, int width, int height)
{
    int initialWidth = width;
    int numMipmaps = (int)std::log2(width);
    std::vector<std::vector<uint8_t>> mipmaps(numMipmaps);
    mipmaps[0] = std::vector<uint8_t>(data, data + width * width);
    for(int i=1; i<numMipmaps; i++)
    {
        width /= 2;
        mipmaps[i].resize(width * width);
        for(int y=0; y<width; y++)
        {
            for(int x=0; x<width; x++)
            {
                int xCoord0 = x * 2;
                int xCoord1 = x * 2 + 1;
                int yCoord0 = y * 2;
                int yCoord1 = y * 2 + 1;

                uint8_t pix0 = mipmaps[i-1][yCoord0 * (width * 2) + xCoord0];
                uint8_t pix1 = mipmaps[i-1][yCoord0 * (width * 2) + xCoord1];
                uint8_t pix2 = mipmaps[i-1][yCoord1 * (width * 2) + xCoord0];
                uint8_t pix3 = mipmaps[i-1][yCoord1 * (width * 2) + xCoord1];

                uint8_t value = std::min(std::min(pix0, pix1), std::min(pix2, pix3));
                mipmaps[i][y * width + x] = value;
            }
        }
        
    }

    for(int i=0; i<numMipmaps; i++)
    {
        glTexImage2D( GL_TEXTURE_2D, i, GL_RED, initialWidth, initialWidth, 0, GL_RED, GL_UNSIGNED_BYTE, mipmaps[i].data());
        initialWidth /= 2;
    }
}

void DispMapping::Load() {

    MeshShader = GL_Shader("shaders/DispMapping/MeshShader.vert", "", "shaders/DispMapping/MeshShader.frag");

    // Mesh = MeshFromFile("resources/models/suzanne/Sphere.obj", false, 0);
    // Mesh = MeshFromFile("resources/models/suzanne/cube.obj", false, 0);
    std::vector<GL_Mesh::Vertex> vertices = 
    {
        {
            glm::vec3(-1.0, -1.0, 0), 
            glm::vec3(0, 0, 1), 
            glm::vec3(1, 0, 0), 
            glm::vec3(0, 1, 0),
            glm::vec2(0, 0)
        },
        {
            glm::vec3(-1.0, 1.0, 0), 
            glm::vec3(0, 0, 1), 
            glm::vec3(1, 0, 0), 
            glm::vec3(0, 1, 0),
            glm::vec2(0, 1)
        },
        {
            glm::vec3(1.0, 1.0, 0), 
            glm::vec3(0, 0, 1), 
            glm::vec3(1, 0, 0), 
            glm::vec3(0, 1, 0),
            glm::vec2(1, 1)
        },
        {
            glm::vec3(1.0, -1.0, 0), 
            glm::vec3(0, 0, 1), 
            glm::vec3(1, 0, 0), 
            glm::vec3(0, 1, 0),
            glm::vec2(1, 0)
        }
    };

    std::vector<uint32_t> triangles = {0,2,1,3,2,0};
    Mesh = new GL_Mesh(vertices, triangles);
    // Mesh = MeshFromFile("resources/models/suzanne/cube.obj", false, 0);
    // CalculateTangents(Mesh->vertices, Mesh->triangles);

    Material = new GL_Material();
    Mesh->material = Material;
    
    // Material->LoadTexture("resources/textures/Displacement/color.jpg", GL_Material::DIFFUSE);
    // Material->LoadTexture("resources/textures/Displacement/height.png", GL_Material::HEIGHT);
    // Material->LoadTexture("resources/textures/Displacement/normal.jpg", GL_Material::NORMAL);
    Material->LoadTexture("resources/textures/Displacement/color_2.png", GL_Material::DIFFUSE);
    Material->LoadTexture("resources/textures/Displacement/height_2.png", GL_Material::HEIGHT);
    Material->LoadTexture("resources/textures/Displacement/normal_2.png", GL_Material::NORMAL);
    // Material->LoadTexture("resources/textures/Displacement/color_3.jpg", GL_Material::DIFFUSE);
    // Material->LoadTexture("resources/textures/Displacement/height_3.png", GL_Material::HEIGHT);
    // Material->LoadTexture("resources/textures/Displacement/normal_3.jpg", GL_Material::NORMAL);
    // Material->LoadTexture("resources/textures/Displacement/color_4.jpg", GL_Material::DIFFUSE);
    // Material->LoadTexture("resources/textures/Displacement/height_4.png", GL_Material::HEIGHT);
    // Material->LoadTexture("resources/textures/Displacement/normal_4.jpg", GL_Material::NORMAL);
    
    TextureCreateInfo info = {};
    // info.generateMipmaps=false;
    texTest = GL_Texture("resources/textures/Displacement/height_2.png", info);
    glBindTexture(GL_TEXTURE_2D, texTest.glTex);
    // GenerateMipMaps(Material->heightTexture.Data(), Material->heightTexture.width, Material->heightTexture.height);
    GenerateMipMaps(texTest.Data(), texTest.width, texTest.height);
    glBindTexture(GL_TEXTURE_2D, 0);

    lightDirection = glm::normalize(glm::vec3(0, -1, -1));

    cam = GL_Camera(glm::vec3(0, 0, 0));  
    cam.SetDistance(2);
}

void DispMapping::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::SliderFloat("Strength", &strength, 0, 0.2f);
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void DispMapping::Render() {
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

    glUseProgram(MeshShader.programShaderObject);
    glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
    glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
    glUniformMatrix4fv(glGetUniformLocation(MeshShader.programShaderObject, "inverseModel"), 1, GL_FALSE, glm::value_ptr(glm::inverse(Mesh->modelMatrix)));
    glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "strength"), strength);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, texTest.glTex);
    glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "texTest"), 6);

    Mesh->Render(cam, MeshShader.programShaderObject);
}

void DispMapping::Unload() {
    Material->Unload();
    delete Material;

    Mesh->Unload();
    delete Mesh;
}


void DispMapping::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void DispMapping::LeftClickDown() {
    cam.mousePressEvent(0);
}

void DispMapping::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void DispMapping::RightClickDown() {
    cam.mousePressEvent(1);
}

void DispMapping::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void DispMapping::Scroll(float offset) {
    cam.Scroll(offset);
}

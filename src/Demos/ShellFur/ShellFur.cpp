#include "ShellFur.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
ShellFur::ShellFur() {
}

void ShellFur::InitFur()
{
    int totalSize = furTexSize * furTexSize;
    struct rgba {
        uint8_t r, g, b, a;
    };
    //Rotation vectors
    std::vector<rgba> furTexData(totalSize);
    for (int i = 0; i < totalSize; i++)
    {
        furTexData[i] ={0,0,0,0};

        float rand = RandomFloat(0, 1);
        if(rand > density)
        {
            float hairLength = RandomFloat(0, 255);
            furTexData[i] ={(uint8_t)hairLength,0,0,255};
        }
    }  

    glGenTextures(1, &furTexture);
    glBindTexture(GL_TEXTURE_2D, furTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, furTexSize, furTexSize, 0, GL_RGBA,  GL_UNSIGNED_BYTE, furTexData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    furColor = GL_Texture("resources/textures/fur.jpg", {});
}

void ShellFur::Reload()
{
    int totalSize = furTexSize * furTexSize;
    struct rgba {
        uint8_t r, g, b, a;
    };
    //Rotation vectors
    std::vector<rgba> furTexData(totalSize);
    for (int i = 0; i < totalSize; i++)
    {
        furTexData[i] ={0,0,0,0};

        float rand = RandomFloat(0, 1);
        if(rand < density)
        {
            float hairLength = RandomFloat(0, 255);
            furTexData[i] ={(uint8_t)hairLength,0,0,255};
        }
    }  

    glBindTexture(GL_TEXTURE_2D, furTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, furTexSize, furTexSize, 0, GL_RGBA,  GL_UNSIGNED_BYTE, furTexData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ShellFur::Load() {

    MeshShader = GL_Shader("shaders/ShellFur/MeshShader.vert", "", "shaders/ShellFur/MeshShader.frag");
    
    //Mesh = MeshFromFile("resources/models/head/head_smooth.obj",false, 0);
    Mesh = MeshFromFile("resources/models/suzanne/Suzanne.obj", false, 0);
    Material = new GL_Material();
    Mesh->material = Material;

    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 1, 0));  

    InitFur();
}

void ShellFur::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::SliderInt("numLayers", &numLayers, 1, 30);
    ImGui::SliderFloat("length", &length, 0.001f, 0.3f);
    ImGui::SliderFloat("density", &density, 0.0, 1.0f);
    ImGui::SliderFloat("tiling", &tiling, 10.0f, 50.0f);
    
    if(ImGui::Button("Reload"))
    {
        Reload();
    }

    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void ShellFur::Render() {
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for(int i=0; i<numLayers; i++)
    {
        float layerFloat = (float) i / (float) numLayers;

        glUseProgram(MeshShader.programShaderObject);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, furTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "furTexture"), 0);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, furColor.glTex);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "furColor"), 1);
        
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "layerFloat"), layerFloat);
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "length"), length);
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "tiling"), tiling);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(lightDirection));

        Mesh->Render(cam, MeshShader.programShaderObject);
    }

}

void ShellFur::Unload() {
    Material->Unload();
    delete Material;
    
    Mesh->Unload();
    delete Mesh;

    MeshShader.Unload();
    furColor.Unload();
    glDeleteTextures(1, &furTexture);
}


void ShellFur::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void ShellFur::LeftClickDown() {
    cam.mousePressEvent(0);
}

void ShellFur::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void ShellFur::RightClickDown() {
    cam.mousePressEvent(1);
}

void ShellFur::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void ShellFur::Scroll(float offset) {
    cam.Scroll(offset);
}

#include "Template.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

Template::Template() {
}

void Template::Load() {

    MeshShader = GL_Shader("shaders/Template/MeshShader.vert", "", "shaders/Template/MeshShader.frag");

    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
    }

    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 10, 0));  

}

void Template::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::Checkbox("Normal Texture", &normalTextureSet);
    ImGui::Checkbox("specular Texture", &specularTextureSet);
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void Template::Render() {
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    for(int i=0; i<Meshes.size(); i++)
    {
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

        glUseProgram(MeshShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
        glm::vec3 test(1,2,3);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
                
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "normalTextureSet"), (int)normalTextureSet);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "specularTextureSet"), (int)specularTextureSet);
        
        Meshes[i]->Render(cam, MeshShader.programShaderObject);
    }
}

void Template::Unload() {
for(int i=0; i<Materials.size(); i++)
    {
        Materials[i]->Unload();
        delete Materials[i];
    }
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->Unload();
        delete Meshes[i];
    }
}


void Template::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void Template::LeftClickDown() {
    cam.mousePressEvent(0);
}

void Template::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void Template::RightClickDown() {
    cam.mousePressEvent(1);
}

void Template::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void Template::Scroll(float offset) {
    cam.Scroll(offset);
}

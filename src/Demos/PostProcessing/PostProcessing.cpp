#include "PostProcessing.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

PostProcessing::PostProcessing() {
}

void PostProcessing::Load() {

    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
    }

    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 10, 0));  

    InitGeometryBuffer();
}

void PostProcessing::InitGeometryBuffer()
{

    //Position
    glGenTextures(1, (GLuint*)&positionTexture);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Normal
    glGenTextures(1, (GLuint*)&normalTexture);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Color
    glGenTextures(1, (GLuint*)&colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Depth
    glGenTextures(1, (GLuint*)&depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, colorTexture, 0);
    unsigned int attachments[3] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, &attachments[0]);  

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

    glEnable(GL_DEPTH_TEST);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << windowWidth << "  " << windowHeight << std::endl;
    } else {
        std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    renderGBufferShader = GL_Shader("shaders/PostProcessing/renderGBuffer.vert", "", "shaders/PostProcessing/renderGBuffer.frag");    
    resolveGBufferShader = GL_Shader("shaders/PostProcessing/resolveGBuffer.vert", "", "shaders/PostProcessing/resolveGBuffer.frag"); 

    
    std::vector<GL_Mesh::Vertex> vertices = 
    {
        {
            glm::vec3(-1.0, -1.0, 0), 
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0),
            glm::vec2(0, 0)
        },
        {
            glm::vec3(-1.0, 1.0, 0), 
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0),
            glm::vec2(0, 1)
        },
        {
            glm::vec3(1.0, 1.0, 0), 
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0),
            glm::vec2(1, 1)
        },
        {
            glm::vec3(1.0, -1.0, 0), 
            glm::vec3(0), 
            glm::vec3(0), 
            glm::vec3(0),
            glm::vec2(1, 0)
        }
    };

    std::vector<uint32_t> triangles = {0,2,1,3,2,0};
    screenSpaceQuad = GL_Mesh(vertices, triangles);	      
}

void PostProcessing::RenderGUI() {
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

void PostProcessing::Render() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Render g buffer
    for(int i=0; i<Meshes.size(); i++)
    {
        glUseProgram(renderGBufferShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
        glUniform3fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(lightDirection));
        glm::mat4 modelView = cam.GetViewMatrix() * Meshes[i]->modelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelView));
        Meshes[i]->Render(cam, renderGBufferShader.programShaderObject);
    }      

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(resolveGBufferShader.programShaderObject);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "colorTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "positionTexture"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "normalTexture"), 2);

    screenSpaceQuad.RenderShader(resolveGBufferShader.programShaderObject);    
}

void PostProcessing::Unload() {
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


void PostProcessing::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void PostProcessing::LeftClickDown() {
    cam.mousePressEvent(0);
}

void PostProcessing::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void PostProcessing::RightClickDown() {
    cam.mousePressEvent(1);
}

void PostProcessing::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void PostProcessing::Scroll(float offset) {
    cam.Scroll(offset);
}

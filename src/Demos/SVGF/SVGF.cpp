#include "SVGF.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
SVGF::SVGF() {
}

void SVGF::Load() {

    glGenTextures(1, (GLuint*)&texture);
    glGenTextures(1, (GLuint*)&textureNormal);
    glGenTextures(1, (GLuint*)&textureFiltered);
    glGenTextures(1, (GLuint*)&textureAA);

    //0
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //1
    glBindTexture(GL_TEXTURE_2D, textureNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //2
    glBindTexture(GL_TEXTURE_2D, textureFiltered);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //3
    glBindTexture(GL_TEXTURE_2D, textureAA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
	glBindTexture(GL_TEXTURE_2D, 0);



    frame=0;
    lightDirection = glm::normalize(glm::vec3(-0.3, 0.9, -0.25));
    cam = GL_Camera(glm::vec3(0, 10, 0));  

    
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
    textureShader = GL_Shader("shaders/SVGF/texture.vert", "", "shaders/SVGF/texture.frag");
    CreateComputeShader("shaders/SVGF/image.compute", &pathTraceShader);
    CreateComputeShader("shaders/SVGF/Filter.compute", &filterShader);
    CreateComputeShader("shaders/SVGF/TAA.compute", &taaShader);
}

void SVGF::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }
    
    ImGui::Checkbox("Compute", &compute);

    ImGui::DragInt("Output", &output, 1, 0, 2);

    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void SVGF::Render() {
    time += 0.01f;

    frame++;
    {
        glUseProgram(pathTraceShader);
        glUniform1i(glGetUniformLocation(pathTraceShader, "width"), windowWidth);
        glUniform1i(glGetUniformLocation(pathTraceShader, "height"), windowHeight);
        glUniform1f(glGetUniformLocation(pathTraceShader, "time"), time);
        glUniform1ui(glGetUniformLocation(pathTraceShader, "frame"), frame);

        glUniformMatrix4fv(glGetUniformLocation(pathTraceShader, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(cam.GetModelMatrix()));

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
        glBindImageTexture(1, textureNormal, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
        
        int groupX = windowWidth / 32 + 1;
        int groupY = windowHeight / 32 + 1;
        glDispatchCompute(groupX, groupY, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);            
    }

    {
        glUseProgram(filterShader);
        glUniform1i(glGetUniformLocation(filterShader, "width"), windowWidth);
        glUniform1i(glGetUniformLocation(filterShader, "height"), windowHeight);
        glUniform1f(glGetUniformLocation(filterShader, "time"), time);
        glUniform1ui(glGetUniformLocation(filterShader, "frame"), frame);

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
        glBindImageTexture(1, textureNormal, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
        glBindImageTexture(2, textureFiltered, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);


        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(filterShader, "samplerColor"), 3);        

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textureNormal);
        glUniform1i(glGetUniformLocation(filterShader, "samplerNormal"), 4);        

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, textureFiltered); 
        glUniform1i(glGetUniformLocation(filterShader, "samplerFiltered"), 5);        
        
        int groupX = windowWidth / 32 + 1;
        int groupY = windowHeight / 32 + 1;
        glDispatchCompute(groupX, groupY, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);            
    }

    {
        glUseProgram(taaShader);
        glUniform1i(glGetUniformLocation(taaShader, "width"), windowWidth);
        glUniform1i(glGetUniformLocation(taaShader, "height"), windowHeight);
        glUniform1f(glGetUniformLocation(taaShader, "time"), time);
        glUniform1ui(glGetUniformLocation(taaShader, "frame"), frame);

        glBindImageTexture(0, textureFiltered, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
        glBindImageTexture(1, textureAA, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);


        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureFiltered);
        glUniform1i(glGetUniformLocation(taaShader, "samplerFiltered"), 3);        

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textureAA);
        glUniform1i(glGetUniformLocation(taaShader, "samplerAA"), 4);        

        int groupX = windowWidth / 32 + 1;
        int groupY = windowHeight / 32 + 1;
        glDispatchCompute(groupX, groupY, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);            
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(textureShader.programShaderObject);
    
    GLuint outputTex;
    if(output==0) outputTex = texture;
    else if(output==1) outputTex = textureFiltered;
    else if(output==2) outputTex = textureAA;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, outputTex);
    glUniform1i(glGetUniformLocation(textureShader.programShaderObject, "tex"), 0);
    
    screenSpaceQuad.RenderShader(textureShader.programShaderObject);
}

void SVGF::Unload() {
}


void SVGF::MouseMove(float x, float y) {
    bool changed = cam.mouseMoveEvent(x, y);
    if(changed) frame=0;
}

void SVGF::LeftClickDown() {
    cam.mousePressEvent(0);
}

void SVGF::LeftClickUp() {
    frame=0;
    cam.mouseReleaseEvent(0);
}

void SVGF::RightClickDown() {
    frame=0;
    cam.mousePressEvent(1);
}

void SVGF::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void SVGF::Scroll(float offset) {
    frame=0;
    cam.Scroll(offset);
}

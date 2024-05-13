#include "SH.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

SH::SH() {
}

void SH::InitBuffers()
{
    glGenVertexArrays(1, &pointsVAO);
    glGenBuffers(1, &pointsVBO);
    glBindVertexArray(pointsVAO);
    
    // bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    //set vertex attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(point), (void*)((uintptr_t)0));
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, true, sizeof(point), (void*)((uintptr_t)12));
	
    
    // //copy data to buffers
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(point), (uint8_t*)points.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);        //Unbind VAO
    glBindVertexArray(0);

    Recompute();
    
    pointsShader = GL_Shader("shaders/SH/points.vert", "", "shaders/SH/points.frag");
}

void SH::Recompute()
{
    points.resize(resolutionX * resolutionY);
    int index=0;
    for(int y=0; y<resolutionY; y++)
    {
        float theta = ((float)y / (float)resolutionY) * PI;
        for(int x=0; x<resolutionX; x++)
        {
            float phi = ((float)x / (float)resolutionX) * PI * 2.0f;
            float sh = (float)SphericalHarmonics(lValue, mValue, theta, phi);
            //float sh = (float)P(2, 2, theta);
            // std::cout << sh << std::endl;
            uint8_t r = 255;
            uint8_t g = 0;
            if(sh < 0) {
                r=0;
                g=255;
            }

            sh = std::abs(sh);
            
            float px = sh * std::cos(phi) * std::sin(theta);
            float py = sh * std::sin(phi) * std::sin(theta);
            float pz = sh * std::cos(theta);
            

            points[index++] = {
                glm::vec3(px, py, pz),
                r, g, 0, 254
            };
        }        
    }
    
    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(point), (uint8_t*)points.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SH::Load() {
    cam = GL_Camera(glm::vec3(0, 1, 0));      
    InitBuffers();
}

void SH::RenderGUI() {
    ImGui::Begin("Parameters : ");
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    // ImGui::SliderInt("m", &m, 1, 10);
    int localL = lValue;
    ImGui::SliderInt("l", &localL, 0, 10);
    if(localL != lValue)
    {
        lValue = localL;
        Recompute();
    }

    int localM = mValue;
    ImGui::SliderInt("m", &localM, 0, lValue);
    if(localM != mValue)
    {
        mValue = localM;
        Recompute();
    }


    ImGui::End();
}

void SH::Render() {
    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(pointsShader.programShaderObject);
    glm::mat4 modelViewProjectionMatrix = cam.GetProjectionMatrix() * cam.GetViewMatrix();
    
    glUniformMatrix4fv(glGetUniformLocation(pointsShader.programShaderObject, "modelViewProjectionMatrix"), 1, false, glm::value_ptr(modelViewProjectionMatrix));
    glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    // glDrawElements(GL_POINTS, (GLsizei)points.size(), GL_UNSIGNED_INT, (void*)0);
    glDrawArrays(GL_POINTS, 0, (GLsizei) points.size());
	glBindVertexArray(0);    
	glUseProgram(0);
}

void SH::Unload() {

}


void SH::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void SH::LeftClickDown() {
    cam.mousePressEvent(0);
}

void SH::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void SH::RightClickDown() {
    cam.mousePressEvent(1);
}

void SH::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void SH::Scroll(float offset) {
    cam.Scroll(offset);
}

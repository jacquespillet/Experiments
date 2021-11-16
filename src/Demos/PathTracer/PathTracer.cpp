#include "PathTracer.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

PathTracer::PathTracer() {
}

void PathTracer::Load() {

    glGenTextures(2, (GLuint*)&pingPongTextures);

    //0
    glBindTexture(GL_TEXTURE_2D, pingPongTextures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    

    //0
    glBindTexture(GL_TEXTURE_2D, pingPongTextures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongTextures[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, pingPongTextures[1], 0);
    unsigned int attachments[2] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1
    };
    glDrawBuffers(2, &attachments[0]);  

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

    glEnable(GL_DEPTH_TEST);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << windowWidth << "  " << windowHeight << std::endl;
    } else {
        std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


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
    renderingShader = GL_Shader("shaders/PathTracer/image.vert", "", "shaders/PathTracer/image.frag");
    textureShader = GL_Shader("shaders/PathTracer/texture.vert", "", "shaders/PathTracer/texture.frag");
    CreateComputeShader("shaders/PathTracer/image.compute", &computeShader);
}

void PathTracer::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void PathTracer::Render() {
    frame++;
    int oldPingPong = pingPongInx;
    pingPongInx = (pingPongInx+1) % 2;
    if(mode == FRAGMENT)
    {

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glDisable(GL_DEPTH_TEST);
        glUseProgram(renderingShader.programShaderObject);
        glUniform1i(glGetUniformLocation(renderingShader.programShaderObject, "width"), windowWidth);
        glUniform1i(glGetUniformLocation(renderingShader.programShaderObject, "height"), windowHeight);
        glUniform1ui(glGetUniformLocation(renderingShader.programShaderObject, "frame"), frame);
        glUniform1i(glGetUniformLocation(renderingShader.programShaderObject, "pingPongInx"), pingPongInx);

        glUniformMatrix4fv(glGetUniformLocation(renderingShader.programShaderObject, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(cam.GetModelMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingPongTextures[oldPingPong]);
        glUniform1i(glGetUniformLocation(renderingShader.programShaderObject, "oldTex"), 0);

        screenSpaceQuad.RenderShader(renderingShader.programShaderObject);
    }
    else
    {
        glUseProgram(computeShader);
        glUniform1i(glGetUniformLocation(computeShader, "width"), windowWidth);
        glUniform1i(glGetUniformLocation(computeShader, "height"), windowHeight);
        glUniform1ui(glGetUniformLocation(computeShader, "frame"), frame);
        glUniform1i(glGetUniformLocation(computeShader, "pingPongInx"), pingPongInx);

        glUniformMatrix4fv(glGetUniformLocation(computeShader, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(cam.GetModelMatrix()));

        glBindImageTexture(0, pingPongTextures[0], 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
        glBindImageTexture(1, pingPongTextures[1], 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingPongTextures[oldPingPong]);
        glUniform1i(glGetUniformLocation(computeShader, "oldTex"), 0);

        int groupX = windowWidth / 32 + 1;
        int groupY = windowHeight / 32 + 1;
        glDispatchCompute(groupX, groupY, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);            
    }

    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(textureShader.programShaderObject);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pingPongTextures[pingPongInx]);
    glUniform1i(glGetUniformLocation(textureShader.programShaderObject, "tex"), 0);
    screenSpaceQuad.RenderShader(textureShader.programShaderObject);
}

void PathTracer::Unload() {
}


void PathTracer::MouseMove(float x, float y) {
    bool changed = cam.mouseMoveEvent(x, y);
    if(changed) frame=0;
}

void PathTracer::LeftClickDown() {
    cam.mousePressEvent(0);
}

void PathTracer::LeftClickUp() {
    frame=0;
    cam.mouseReleaseEvent(0);
}

void PathTracer::RightClickDown() {
    frame=0;
    cam.mousePressEvent(1);
}

void PathTracer::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void PathTracer::Scroll(float offset) {
    frame=0;
    cam.Scroll(offset);
}

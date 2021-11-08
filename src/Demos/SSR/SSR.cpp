#include "SSR.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

SSR::SSR() {
    blurCount=1;
}

void SSR::Load() {
    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
        if(i==46)
        {
            Meshes[i]->material->roughness=0.1f;
        }
        else
        {
            Meshes[i]->material->roughness=1.0f;
        }
    }

    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 10, 0));  
    

    InitGeometryBuffer();
    InitSSR();
}

void SSR::InitSSR()
{
    //Color
    glGenTextures(1, (GLuint*)&ssrTexture);
    glBindTexture(GL_TEXTURE_2D, ssrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    
    glGenFramebuffers(1, &ssrFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ssrFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << windowWidth << "  " << windowHeight << std::endl;
    } else {
        std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
    }

    //Color
    glGenTextures(2, (GLuint*)&blurTexture);
    glBindTexture(GL_TEXTURE_2D, blurTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    
    CreateComputeShader("shaders/SSR/blur.compute", &blurComputeShader);

    ssrShader = GL_Shader("shaders/SSR/ssr.vert", "", "shaders/SSR/ssr.frag");

    TextureCreateInfo tci = {};
    noiseTexture = GL_Texture("resources/textures/ssr_noise.png", tci);
}

void SSR::InitGeometryBuffer()
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

    renderGBufferShader = GL_Shader("shaders/SSR/renderGBuffer.vert", "", "shaders/SSR/renderGBuffer.frag");    
    resolveGBufferShader = GL_Shader("shaders/SSR/resolveGBuffer.vert", "", "shaders/SSR/resolveGBuffer.frag"); 

    
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

void SSR::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::SliderInt("Blur passes", &blurCount, 0, 10);
    
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void SSR::Render() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Render g buffer
    // int i = renderMesh;
    for(int i=0; i<Meshes.size(); i++)
    {
        glUseProgram(renderGBufferShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
        glUniformMatrix4fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(Meshes[i]->modelMatrix));
        Meshes[i]->Render(cam, renderGBufferShader.programShaderObject);
    }      

    //Do SSR
    glBindFramebuffer(GL_FRAMEBUFFER, ssrFramebuffer);
    glUseProgram(ssrShader.programShaderObject);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glUniform1i(glGetUniformLocation(ssrShader.programShaderObject, "colorTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glUniform1i(glGetUniformLocation(ssrShader.programShaderObject, "positionTexture"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glUniform1i(glGetUniformLocation(ssrShader.programShaderObject, "normalTexture"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(glGetUniformLocation(ssrShader.programShaderObject, "depthTexture"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, noiseTexture.glTex);
    glUniform1i(glGetUniformLocation(ssrShader.programShaderObject, "noiseTexture"), 4);

    // uniform sampler2D noiseTexture;
    glUniform3fv(glGetUniformLocation(ssrShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));

    glm::mat4 vp = cam.GetProjectionMatrix() * cam.GetViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(ssrShader.programShaderObject, "vp"), 1, GL_FALSE, glm::value_ptr(vp));
    
    glUniform1f(glGetUniformLocation(ssrShader.programShaderObject, "zNear"), cam.GetNearPlane());
    glUniform1f(glGetUniformLocation(ssrShader.programShaderObject, "zFar"), cam.GetFarPlane());
    
    glUniform2fv(glGetUniformLocation(ssrShader.programShaderObject, "screenResolution"), 1, glm::value_ptr(glm::vec2(windowWidth, windowHeight)));
    glUniform2fv(glGetUniformLocation(ssrShader.programShaderObject, "invNoiseResolution"), 1, glm::value_ptr(glm::vec2(1.0f / (float)noiseTexture.width, 1.0f / (float)noiseTexture.height)));
    glUniform1f(glGetUniformLocation(ssrShader.programShaderObject, "downScale"), 1.0f);

    screenSpaceQuad.RenderShader(ssrShader.programShaderObject);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //resolve g buffer
    glm::mat4 inverseView = glm::inverse(cam.GetViewMatrix());


    //Blur ssr
    for (int i = 0; i < blurCount; i++)
    {
        int groupX = windowWidth/ 32+1;
        int groupY = windowHeight/ 32+1;
        glUseProgram(blurComputeShader);
        glUniform2fv(glGetUniformLocation(blurComputeShader, "invTexResolution"), 1, glm::value_ptr(glm::vec2(1.0f/windowWidth,1.0f/ windowHeight))); //program must be active
        glBindImageTexture(0, ssrTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA8);
        glBindImageTexture(1, blurTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA8);
    
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssrTexture);
        glUniform1i(glGetUniformLocation(blurComputeShader, "texASampler"), 0);
    
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, blurTexture);
        glUniform1i(glGetUniformLocation(blurComputeShader, "texBSampler"), 1);

        glUniform2iv(glGetUniformLocation(blurComputeShader, "direction"), 1, glm::value_ptr(glm::ivec2(1,0))); //program must be active
        glUniform1i(glGetUniformLocation(blurComputeShader, "pingpong"), 0); //program must be active
        glDispatchCompute(groupX, groupY, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        
        glUniform2iv(glGetUniformLocation(blurComputeShader, "direction"), 1, glm::value_ptr(glm::ivec2(0,1))); //program must be active
        glUniform1i(glGetUniformLocation(blurComputeShader, "pingpong"), 1); //program must be active
        glDispatchCompute(groupX, groupY, 1);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
        
        glUseProgram(0);
    }
    

    
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

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ssrTexture);
    glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "ssrTexture"), 3);

    glUniform3fv(glGetUniformLocation(resolveGBufferShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniformMatrix4fv(glGetUniformLocation(resolveGBufferShader.programShaderObject, "inverseViewMatrix"), 1, GL_FALSE, glm::value_ptr(inverseView));
    glUniform3fv(glGetUniformLocation(resolveGBufferShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));

    screenSpaceQuad.RenderShader(resolveGBufferShader.programShaderObject);
}

void SSR::Unload() {
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

    glDeleteFramebuffers(1,&framebuffer);
    glDeleteTextures(1, &colorTexture);
    glDeleteTextures(1, &normalTexture);
    glDeleteTextures(1, &positionTexture);
    glDeleteTextures(1, &depthTexture);
    renderGBufferShader.Unload();
    resolveGBufferShader.Unload();
    screenSpaceQuad.Unload();

    glDeleteFramebuffers(1, &ssrFramebuffer);
    glDeleteTextures(1, &ssrTexture);
    ssrShader.Unload();
    noiseTexture.Unload();
    
    glDeleteShader(blurComputeShader);
    glDeleteTextures(1, &blurTexture);

}


void SSR::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void SSR::LeftClickDown() {
    cam.mousePressEvent(0);
}

void SSR::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void SSR::RightClickDown() {
    cam.mousePressEvent(1);
}

void SSR::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void SSR::Scroll(float offset) {
    cam.Scroll(offset);
}

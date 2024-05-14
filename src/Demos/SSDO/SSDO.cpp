#include "SSDO.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

SSDO::SSDO() {
}

void SSDO::LoadHDR()
{
    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    float *data = stbi_loadf("resources/textures/SSDO/Blurred.hdr", &width, &height, &nChannels, 0);
    if (data)
    {
        glGenTextures(1, &envTexture);
        glBindTexture(GL_TEXTURE_2D, envTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); 

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load HDR image." << std::endl;
    }  
}

void SSDO::Load() {
    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
    }

    dragon = MeshFromFile("resources/models/dragon/dragon.obj", false, 0);
    dragon->SetPos(glm::vec3(0, 0, 0));
    Meshes.push_back(dragon);
    GL_Material *mat = new GL_Material();
    mat->diffuse = glm::vec3(1,0,0);
    dragon->material = mat;

    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 10, 0));  
    InitGeometryBuffer();
    InitSSAO();

    LoadHDR();

}

void SSDO::InitGeometryBuffer()
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB);

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

    renderGBufferShader = GL_Shader("shaders/SSDO/renderGBuffer.vert", "", "shaders/SSDO/renderGBuffer.frag");    
    resolveGBufferShader = GL_Shader("shaders/SSDO/resolveGBuffer.vert", "", "shaders/SSDO/resolveGBuffer.frag"); 

    
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

void SSDO::InitSSAO()
{
    //Kernel
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel(64);
    for (unsigned int i = 0; i < 64; ++i)
    {
        //TODO*(JAcques): Use RandomVEctor3()
        glm::vec3 sample(
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator)
        );
        sample  = glm::normalize(sample);
        float scale = (float)i / 64.0f; 
        scale   = Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel[i] =sample;
    }    
    
    glGenBuffers(1, &sampleKernelBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sampleKernelBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * sizeof(glm::vec3), ssaoKernel.data(), GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, sampleKernelBuffer);


    //Rotation vectors
    std::vector<glm::vec3> randomVectors(16);
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator) * 2.0 - 1.0, 
            0.0f); 
        randomVectors[i] =noise;
    }  

    glGenTextures(1, &rotationVectorsTexture);
    glBindTexture(GL_TEXTURE_2D, rotationVectorsTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, randomVectors.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  

    //Create framebuffer
    glGenFramebuffers(1, &ssaoFramebuffer);  
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFramebuffer);
    
    glGenTextures(1, &ssaoTexture);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);

    ssaoShader = GL_Shader("shaders/SSDO/SSDO.vert", "", "shaders/SSDO/SSDO.frag");
    blurShader = GL_Shader("shaders/SSDO/blur.vert", "", "shaders/SSDO/blur.frag");

    glGenTextures(1, &blurredSsaoTexture);
    glBindTexture(GL_TEXTURE_2D, blurredSsaoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glGenFramebuffers(1, &blurFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, blurFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurredSsaoTexture, 0);

}

void SSDO::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::SliderInt("kernel Size", &kernelSize, 0, 64);
    ImGui::SliderFloat("Strength", &strength, 1, 10);
    ImGui::SliderFloat("radius", &radius, 0, 10);
    ImGui::Checkbox("View SSDO", &renderSSAO);
    ImGui::Checkbox("enabled", &ssaoEnabled);
    
    ImGui::SliderFloat("indirectRadiance", &indirectRadiance, 0, 32);

        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void SSDO::Render() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //Render g buffer
    for(int i=0; i<Meshes.size(); i++)
    {
        glUseProgram(renderGBufferShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
        glm::mat4 modelView = cam.GetViewMatrix() * Meshes[i]->modelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelView));
        Meshes[i]->Render(cam, renderGBufferShader.programShaderObject);
    }        
    
    {

        //Do ssao
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFramebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(ssaoShader.programShaderObject);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, positionTexture);
        glUniform1i(glGetUniformLocation(ssaoShader.programShaderObject, "positionTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTexture);
        glUniform1i(glGetUniformLocation(ssaoShader.programShaderObject, "normalTexture"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, rotationVectorsTexture);
        glUniform1i(glGetUniformLocation(ssaoShader.programShaderObject, "noiseTexture"), 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, envTexture);
        glUniform1i(glGetUniformLocation(ssaoShader.programShaderObject, "envmapTexture"), 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glUniform1i(glGetUniformLocation(ssaoShader.programShaderObject, "colorTexture"), 4);

        
        glUniformMatrix4fv(glGetUniformLocation(ssaoShader.programShaderObject, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(cam.GetProjectionMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(ssaoShader.programShaderObject, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
        glUniform1i(glGetUniformLocation(ssaoShader.programShaderObject, "kernelSize"), kernelSize);
        glUniform1f(glGetUniformLocation(ssaoShader.programShaderObject, "strength"), strength);
        glUniform1f(glGetUniformLocation(ssaoShader.programShaderObject, "radius"), radius);
        glUniform1f(glGetUniformLocation(ssaoShader.programShaderObject, "indirectRadiance"), indirectRadiance);
        glUniform3fv(glGetUniformLocation(ssaoShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(lightDirection));
    
        glm::vec2 screenResolution(windowWidth, windowHeight);
        glUniform2fv(glGetUniformLocation(ssaoShader.programShaderObject, "screenResolution"), 1, glm::value_ptr(screenResolution));
        
        screenSpaceQuad.RenderShader(ssaoShader.programShaderObject);
        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Blur ssao
        glBindFramebuffer(GL_FRAMEBUFFER, blurFramebuffer);
        glUseProgram(blurShader.programShaderObject);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoTexture);
        glUniform1i(glGetUniformLocation(blurShader.programShaderObject, "noiseTexture"), 2);

        screenSpaceQuad.RenderShader(blurShader.programShaderObject);
        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    //resolve g buffer
    glm::mat4 inverseView = glm::inverse(cam.GetViewMatrix());

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
    glBindTexture(GL_TEXTURE_2D, blurredSsaoTexture);
    glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "ssaoTexture"), 3);
    
    glUniform3fv(glGetUniformLocation(resolveGBufferShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniformMatrix4fv(glGetUniformLocation(resolveGBufferShader.programShaderObject, "inverseViewMatrix"), 1, GL_FALSE, glm::value_ptr(inverseView));
    glUniform3fv(glGetUniformLocation(resolveGBufferShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));

    glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "renderSSAO"), (int)renderSSAO);
    glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "ssaoEnabled"), (int)ssaoEnabled);
    screenSpaceQuad.RenderShader(resolveGBufferShader.programShaderObject);
}

void SSDO::Unload() {
	dragon->Unload();
	dragon->material->Unload();
	delete dragon->material;

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

    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &colorTexture);
    glDeleteTextures(1, &normalTexture);
    glDeleteTextures(1, &positionTexture);
    glDeleteTextures(1, &depthTexture);
    glDeleteShader(renderGBufferShader.programShaderObject);
    glDeleteShader(resolveGBufferShader.programShaderObject);
    screenSpaceQuad.Unload();

    glDeleteBuffers(1, &sampleKernelBuffer);
    glDeleteFramebuffers(1, &ssaoFramebuffer);
    glDeleteFramebuffers(1, &blurFramebuffer);
    glDeleteTextures(1, &rotationVectorsTexture);
    glDeleteTextures(1, &ssaoTexture);
    glDeleteTextures(1, &blurredSsaoTexture);
    glDeleteShader(ssaoShader.programShaderObject);
    glDeleteShader(blurShader.programShaderObject);    

}


void SSDO::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void SSDO::LeftClickDown() {
    cam.mousePressEvent(0);
}

void SSDO::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void SSDO::RightClickDown() {
    cam.mousePressEvent(1);
}

void SSDO::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void SSDO::Scroll(float offset) {
    cam.Scroll(offset);
}

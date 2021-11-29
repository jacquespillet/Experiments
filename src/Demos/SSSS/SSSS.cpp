#include "SSSS.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

SSSS::SSSS() {
}

void SSSS::LoadFramebuffer()
{

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

    //linearDepth
	glGenTextures(1, (GLuint*)&linearDepthTexture);
    glBindTexture(GL_TEXTURE_2D, linearDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
	glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, linearDepthTexture, 0);
    unsigned int attachments[1] = {
        GL_COLOR_ATTACHMENT0
    };
    glDrawBuffers(1, &attachments[0]);  

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error creating framebuffer" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);    
}

void SSSS::LoadBlurElements()
{
    //linearDepth
	glGenTextures(1, (GLuint*)&blurTexture);
    glBindTexture(GL_TEXTURE_2D, blurTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    CreateComputeShader("shaders/SSSS/Blur.compute", &blurComputeShader);
}

void SSSS::BlurTexture()
{
    //Blur horiz from linearDepth to blurTexture


    //Blur vert form blurTexture to linearDepth
    int groupX = windowWidth/ 32+1;
    int groupY = windowHeight/ 32+1;
    glUseProgram(blurComputeShader);
    glUniform2fv(glGetUniformLocation(blurComputeShader, "invTexResolution"), 1, glm::value_ptr(glm::vec2(1.0f/windowWidth,1.0f/ windowHeight))); //program must be active
    glBindImageTexture(0, linearDepthTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_R32F);
    glBindImageTexture(1, blurTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_R32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, linearDepthTexture);
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

void SSSS::Load() {

    MeshShader = GL_Shader("shaders/SSSS/MeshShader.vert", "", "shaders/SSSS/MeshShader.frag");
    DepthShader = GL_Shader("shaders/SSSS/depthPass.vert", "", "shaders/SSSS/depthPass.frag");

    // Mesh = MeshFromFile("resources/models/head/head_smooth.obj",false, 0);
    Mesh = MeshFromFile("resources/models/buddha.obj",false, 0);
    Material = new GL_Material();
    // Material->LoadTexture("resources/models/head/head_color.jpg", GL_Material::TEXTURE_TYPE::DIFFUSE, false);
    // Material->LoadTexture("resources/models/head/head_normal.jpg", GL_Material::TEXTURE_TYPE::NORMAL, false);
    Mesh->material = Material;

    lightDirection = glm::normalize(glm::vec3(1, -1, 0));

    cam = GL_Camera(glm::vec3(0, 2, 0));  
    cam.SetDistance(2);

    LoadFramebuffer();
    LoadBlurElements();
}

void SSSS::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }
    
    ImGui::SliderFloat("distortion", &distortion, 0, 5);
    ImGui::SliderFloat("scale", &scale, 0, 5);
    ImGui::SliderInt("power", &power, 0, 32);;
    ImGui::SliderInt("blurPasses", &blurPasses, 0, 32);;


    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void SSSS::Render() {
    {
        glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDepthFunc(GL_GREATER);  
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        //First horiz blur
        glClearDepth(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

        glm::mat4 modelViewMatrix = cam.GetViewMatrix() * Mesh->modelMatrix;

        glUseProgram(DepthShader.programShaderObject);
        glUniformMatrix4fv(glGetUniformLocation(DepthShader.programShaderObject, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
        Mesh->Render(cam, DepthShader.programShaderObject);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    for(int i=0; i<blurPasses; i++)
    {
        BlurTexture();
    }
    
   {
        glDisable(GL_CULL_FACE);
        glClearDepth(std::numeric_limits<float>::max());
        glDepthFunc(GL_LESS);  
        //First horiz blur

        // glFrontFace(GL_CW);
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_FRONT);
        // glDepthFunc(GL_GREATER);  
        // glClearDepth(0);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glEnable(GL_DEPTH_TEST);
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

        glm::mat4 modelViewMatrix = cam.GetViewMatrix() * Mesh->modelMatrix;

        glUseProgram(MeshShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
        
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "distortion"), distortion);
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "scale"), scale);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "power"), power);

        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, linearDepthTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "depthTexture"), 6);


        glUniformMatrix4fv(glGetUniformLocation(MeshShader.programShaderObject, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewMatrix));
        Mesh->Render(cam, MeshShader.programShaderObject);

        // glDisable(GL_CULL_FACE);
        // glDepthFunc(GL_LESS);  
        // glClearDepth(std::numeric_limits<float>::max());
    }
}

void SSSS::Unload() {
    Material->Unload();
    delete Material;

    Mesh->Unload();
    delete Mesh;

}


void SSSS::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void SSSS::LeftClickDown() {
    cam.mousePressEvent(0);
}

void SSSS::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void SSSS::RightClickDown() {
    cam.mousePressEvent(1);
}

void SSSS::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void SSSS::Scroll(float offset) {
    cam.Scroll(offset);
}

#include "PostProcessing.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

PostProcess::PostProcess(std::string shaderFileName) : shaderFileName(shaderFileName) 
{
    CreateComputeShader(shaderFileName, &shader);
}

void PostProcess::SetUniforms()
{
    
}

void PostProcess::RenderGui()
{

}


void PostProcess::Process(GLuint textureIn, GLuint textureOut, int width, int height)
{
	glUseProgram(shader);
	SetUniforms();

	glUniform1i(glGetUniformLocation(shader, "textureIn"), 0); //program must be active
    glBindImageTexture(0, textureIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
	
    glUniform1i(glGetUniformLocation(shader, "textureOut"), 1); //program must be active
    glBindImageTexture(1, textureOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
     
    glDispatchCompute(width / 32, height / 32, 1);
	glUseProgram(0);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}


// 1 : 
//     read in, write out, return out
// 2 : 
//     read in, write out
//     read out, write in
//     return in
// 3 : 
//     read in, write out
//     read out, write in
//     read in, write out
//     return out
// 4 : 
//     read in, write out
//     read out, write in
//     read in, write out
//     read out, write in
//     return in


GLuint PostProcessStack::Process(GLuint textureIn, GLuint textureOut, int width, int height)
{

    for(int i=0; i<postProcesses.size(); i++)
    {
        if(i==0)
        {
            postProcesses[i]->Process(
                                    textureIn,
                                    textureOut,
                                    width, 
                                    height);
        }
        else
        {
            bool pairPass = i % 2 == 0;
            postProcesses[i]->Process(
                                    pairPass ? textureIn : textureOut, 
                                    pairPass ? textureOut : textureIn, 
                                    width, 
                                    height);
        }
    }

    return (postProcesses.size() % 2 == 0) ? textureIn : textureOut;
}

//------------------------------------------------------------------------
GrayScalePostProcess::GrayScalePostProcess() : PostProcess("shaders/PostProcessing/PostProcesses/GrayScale.compute")
{}

void GrayScalePostProcess::SetUniforms()
{
    glUniform1f(glGetUniformLocation(shader, "saturation"), saturation);
}

void GrayScalePostProcess::RenderGui()
{
    ImGui::Text("Gray Scale");
    ImGui::SliderFloat("Saturation", &saturation, 0, 1);
}

//------------------------------------------------------------------------
ContrastBrightnessPostProcess::ContrastBrightnessPostProcess() : PostProcess("shaders/PostProcessing/PostProcesses/ContrastBrightness.compute")
{}

void ContrastBrightnessPostProcess::SetUniforms()
{
    glUniform3fv(glGetUniformLocation(shader, "contrast"), 1, glm::value_ptr(contrast));
    glUniform3fv(glGetUniformLocation(shader, "brightness"), 1, glm::value_ptr(brightness));
    glUniform1f(glGetUniformLocation(shader, "brightnessIntensity"), brightnessIntensity);
    glUniform1f(glGetUniformLocation(shader, "contrastIntensity"), contrastIntensity);
}

void ContrastBrightnessPostProcess::RenderGui()
{
    ImGui::Text("Contrast Brightness");
    ImGui::DragFloat("contrast intensity", &contrastIntensity, 0.1f, 0);
    ImGui::ColorEdit3("contrast", glm::value_ptr(contrast));
    ImGui::DragFloat("brightness intensity", &brightnessIntensity, 0.1f, 0);
    ImGui::ColorEdit3("brightness", glm::value_ptr(brightness));
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
ChromaticAberationPostProcess::ChromaticAberationPostProcess() : PostProcess("shaders/PostProcessing/PostProcesses/ChromaticAberation.compute")
{}

void ChromaticAberationPostProcess::SetUniforms()
{
    glUniform3fv(glGetUniformLocation(shader, "offsets"), 1, glm::value_ptr(offsets));

    glm::vec2 normalizedDirection = glm::normalize(direction);
    glUniform2fv(glGetUniformLocation(shader, "direction"), 1, glm::value_ptr(normalizedDirection));
    
    glUniform1i(glGetUniformLocation(shader, "aroundMouse"), (bool)aroundMouse);
    
    ImGuiIO &io = ImGui::GetIO();

    glUniform2i(glGetUniformLocation(shader, "mousePos"), (int)io.MousePos.x, (int)io.MousePos.y);
}

void ChromaticAberationPostProcess::RenderGui()
{
    ImGui::Text("Chromatic Aberation");
    //ImGui::SliderFloat("yo", &saturation, 0, 1);
    ImGui::DragFloat3("Offsets", glm::value_ptr(offsets), 1, -20, 20);
    ImGui::DragFloat3("Direction", glm::value_ptr(direction ), 0.01f, -1, 1);

    ImGui::Checkbox("Around Mouse", &aroundMouse);
}

//------------------------------------------------------------------------

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

    postProcessStack.postProcesses.push_back(new GrayScalePostProcess());
    postProcessStack.postProcesses.push_back(new ContrastBrightnessPostProcess());
    postProcessStack.postProcesses.push_back(new ChromaticAberationPostProcess());
    
    //Color
    glGenTextures(1, (GLuint*)&postProcessTexture);
    glBindTexture(GL_TEXTURE_2D, postProcessTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
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

	ImGui::Separator();

    ImGui::Text("Post Processes");
    for(int i=0; i< postProcessStack.postProcesses.size(); i++)
    {
        postProcessStack.postProcesses[i]->RenderGui();
        ImGui::Separator();
    }
        
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

    GLuint outTexture = postProcessStack.Process(colorTexture, postProcessTexture, windowWidth, windowHeight);

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

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, outTexture);
    glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "postProcessedTexture"), 3);

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

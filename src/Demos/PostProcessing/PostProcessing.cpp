#include "PostProcessing.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

PostProcess::PostProcess(std::string name, std::string shaderFileName, bool enabled) :name(name), shaderFileName(shaderFileName), enabled(enabled)
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
     
    glDispatchCompute((width / 32) + 1, (height / 32) + 1, 1);
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
    std::vector<PostProcess*> activeProcesses;
    activeProcesses.reserve(postProcesses.size());
    for(int i=0; i<postProcesses.size(); i++)
    {
        if(postProcesses[i]->enabled) activeProcesses.push_back(postProcesses[i]);
    }

    for(int i=0; i<activeProcesses.size(); i++)
    {
        if(i==0)
        {
            activeProcesses[i]->Process(
                                    textureIn,
                                    textureOut,
                                    width, 
                                    height);
        }
        else
        {
            bool pairPass = i % 2 == 0;
            activeProcesses[i]->Process(
                                    pairPass ? textureIn : textureOut, 
                                    pairPass ? textureOut : textureIn, 
                                    width, 
                                    height);
        }
    }

    return (activeProcesses.size() % 2 == 0) ? textureIn : textureOut;
}

//------------------------------------------------------------------------
GrayScalePostProcess::GrayScalePostProcess(bool enabled) : PostProcess("GrayScale", "shaders/PostProcessing/PostProcesses/GrayScale.compute", enabled)
{}

void GrayScalePostProcess::SetUniforms()
{
    glUniform1f(glGetUniformLocation(shader, "saturation"), saturation);
}

void GrayScalePostProcess::RenderGui()
{
    ImGui::SliderFloat("Saturation", &saturation, 0, 1);
}

//------------------------------------------------------------------------
ContrastBrightnessPostProcess::ContrastBrightnessPostProcess(bool enabled) : PostProcess("ContrastBrightness", "shaders/PostProcessing/PostProcesses/ContrastBrightness.compute", enabled)
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
    ImGui::DragFloat("contrast intensity", &contrastIntensity, 0.1f, 0);
    ImGui::ColorEdit3("contrast", glm::value_ptr(contrast));
    ImGui::DragFloat("brightness intensity", &brightnessIntensity, 0.1f, 0);
    ImGui::ColorEdit3("brightness", glm::value_ptr(brightness));
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
ChromaticAberationPostProcess::ChromaticAberationPostProcess(bool enabled) : PostProcess("Chromatic Aberation", "shaders/PostProcessing/PostProcesses/ChromaticAberation.compute", enabled)
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
    //ImGui::SliderFloat("yo", &saturation, 0, 1);
    ImGui::DragFloat3("Offsets", glm::value_ptr(offsets), 1, -20, 20);
    ImGui::DragFloat3("Direction", glm::value_ptr(direction ), 0.01f, -1, 1);

    ImGui::Checkbox("Around Mouse", &aroundMouse);
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
PixelizePostProcess::PixelizePostProcess(bool enabled) : PostProcess("Pixelize", "shaders/PostProcessing/PostProcesses/Pixelize.compute", enabled)
{}

void PixelizePostProcess::SetUniforms()
{
    glUniform1i(glGetUniformLocation(shader, "pixelSize"), pixelSize);
}

void PixelizePostProcess::RenderGui()
{
    ImGui::SliderInt("Pixel Size", &pixelSize, 1, 32);
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
GodRaysPostProcess::GodRaysPostProcess(glm::vec3 *lightPosition, glm::mat4 *viewMatrix, glm::mat4 *projectionMatrix, bool enabled) : 
                        PostProcess("GodRays", "shaders/PostProcessing/PostProcesses/GodRays.compute", enabled),
                        lightPosition(lightPosition), viewMatrix(viewMatrix), projectionMatrix(projectionMatrix)
{}

void GodRaysPostProcess::SetUniforms()
{
    glm::vec4 lightViewPos = *projectionMatrix * *viewMatrix * glm::vec4(*lightPosition, 1.0f);
    lightViewPos /= lightViewPos.w;
    lightViewPos  = lightViewPos * 0.5f + 0.5f;
    glUniform3fv(glGetUniformLocation(shader, "lightViewPosition"), 1, glm::value_ptr(lightViewPos));

    glUniform1f(glGetUniformLocation(shader, "density"), density);
    glUniform1f(glGetUniformLocation(shader, "weight"), weight);
    glUniform1f(glGetUniformLocation(shader, "decay"), decay);
    glUniform1i(glGetUniformLocation(shader, "numSamples"), numSamples);

    
}

void GodRaysPostProcess::RenderGui()
{
    ImGui::DragInt("numSamples", &numSamples, 1, 0, 1024);
    ImGui::DragFloat("density", &density, 0.01f, 0, 100);
    ImGui::DragFloat("weight", &weight, 0.01f, 0, 10);
    ImGui::DragFloat("decay", &decay, 0.01f, 0, 1);
}

void GodRaysPostProcess::Process(GLuint textureIn, GLuint textureOut, int width, int height) 
{
	glUseProgram(shader);
	SetUniforms();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureIn);
    glUniform1i(glGetUniformLocation(shader, "textureIn"), 0); //program must be active
	
    glUniform1i(glGetUniformLocation(shader, "textureOut"), 1); //program must be active
    glBindImageTexture(1, textureOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
     
    glDispatchCompute((width / 32) + 1, (height / 32) + 1, 1);
	glUseProgram(0);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
ToneMappingPostProcess::ToneMappingPostProcess(bool enabled) : PostProcess("ToneMapping", "shaders/PostProcessing/PostProcesses/ToneMapping.compute", enabled)
{}

void ToneMappingPostProcess::SetUniforms()
{
    glUniform1f(glGetUniformLocation(shader, "exposure"), exposure);
    glUniform1i(glGetUniformLocation(shader, "type"), type);
}

void ToneMappingPostProcess::RenderGui()
{
    ImGui::DragFloat("Exposure", &exposure,0.01f, 0.01f, 10.0f);
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
DepthOfFieldPostProcess::DepthOfFieldPostProcess(GLuint positionTexture, int width, int height, bool enabled) : PostProcess("DepthOfField", "shaders/PostProcessing/PostProcesses/DepthOfField.compute", enabled), positionTexture(positionTexture)
{
    glGenTextures(1, (GLuint*)&cocTexture);
    glBindTexture(GL_TEXTURE_2D, cocTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, (GLuint*)&prefilteredCocTexture);
    glBindTexture(GL_TEXTURE_2D, prefilteredCocTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width/2, height/2, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 
    glGenTextures(1, (GLuint*)&bokehTextureIn);
    glBindTexture(GL_TEXTURE_2D, bokehTextureIn);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width/2, height/2, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
 
    glGenTextures(1, (GLuint*)&bokehTextureOut);
    glBindTexture(GL_TEXTURE_2D, bokehTextureOut);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width/2, height/2, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    
    CreateComputeShader("shaders/PostProcessing/PostProcesses/DepthOfFieldCoc.compute", &cocShader);
    CreateComputeShader("shaders/PostProcessing/PostProcesses/Blit.compute", &blitShader);
    CreateComputeShader("shaders/PostProcessing/PostProcesses/DepthOfFieldPrefilterCoc.compute", &prefilterCocShader);
    CreateComputeShader("shaders/PostProcessing/PostProcesses/DepthOfFieldCombine.compute", &combineShader);
}

void DepthOfFieldPostProcess::SetUniforms()
{
}

void DepthOfFieldPostProcess::RenderGui()
{
    ImGui::DragFloat("FocusRange", &focusRange, 0.1f, 0, 50);
    ImGui::DragFloat("FocusDistance", &focusDistance, 0.1f, 0.01f, 50);
    ImGui::DragFloat("BokehSize", &bokehSize, 0.1f, 0, 32);
}

void DepthOfFieldPostProcess::Blit(GLuint source, GLuint target, int targetWidth, int targetHeight)
{
    //Coc pass
	{
        glUseProgram(blitShader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source);
        glUniform1i(glGetUniformLocation(blitShader, "source"), 0); //program must be active
        
        glUniform1i(glGetUniformLocation(blitShader, "target"), 1); //program must be active
        glBindImageTexture(1, target, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F); 
        
        glDispatchCompute((targetWidth / 32) + 1, (targetHeight / 32) + 1, 1);
        glUseProgram(0);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }    
}

void DepthOfFieldPostProcess::Process(GLuint textureIn, GLuint textureOut, int width, int height)
{
    //Coc pass
	{
        glUseProgram(cocShader);

        glUniform1i(glGetUniformLocation(cocShader, "positionTexture"), 0); //program must be active
        glBindImageTexture(0, positionTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        
        glUniform1i(glGetUniformLocation(cocShader, "textureOut"), 1); //program must be active
        glBindImageTexture(1, cocTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F); 
        
        glUniform1f(glGetUniformLocation(cocShader, "focusRange"), focusRange); //program must be active
        glUniform1f(glGetUniformLocation(cocShader, "focusDistance"), focusDistance); //program must be active
        glUniform1f(glGetUniformLocation(cocShader, "bokehSize"), bokehSize);

        glDispatchCompute((width / 32) + 1, (height / 32) + 1, 1);
        glUseProgram(0);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    //Prefilter coc
	{
        glUseProgram(prefilterCocShader);

        glUniform1i(glGetUniformLocation(prefilterCocShader, "cocTexture"), 0); //program must be active
        glBindImageTexture(0, cocTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
        
        glUniform1i(glGetUniformLocation(prefilterCocShader, "prefilteredCoc"), 1); //program must be active
        glBindImageTexture(1, prefilteredCocTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R16F); 
        
        glDispatchCompute((width/2) / 32 + 1, (height/2) / 32 + 1, 1);
        glUseProgram(0);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    //Blit textureIn into bokehTexture
    Blit(textureIn, bokehTextureIn, width/2, height/2);

    //Compute shader that takes the first texture as a sampler
	{
        glUseProgram(shader);

        glUniform1i(glGetUniformLocation(shader, "textureIn"), 0); //program must be active
        glBindImageTexture(0, bokehTextureIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        
        glUniform1i(glGetUniformLocation(shader, "prefilteredCocTexture"), 1); //program must be active
        glBindImageTexture(1, prefilteredCocTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16F);
        
        glUniform1i(glGetUniformLocation(shader, "textureOut"), 2); //program must be active
        glBindImageTexture(2, bokehTextureOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F); 
        
        glUniform1f(glGetUniformLocation(shader, "bokehSize"), bokehSize);

        glDispatchCompute(((width/2) / 32) + 1, ((height/2) / 32) + 1, 1);
        glUseProgram(0);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

	{
        glUseProgram(combineShader);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bokehTextureOut);
        glUniform1i(glGetUniformLocation(combineShader, "bokehTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, prefilteredCocTexture);
        glUniform1i(glGetUniformLocation(combineShader, "cocTexture"), 1);
        
        glUniform1i(glGetUniformLocation(combineShader, "textureIn"), 1); //program must be active
        glBindImageTexture(1, textureIn, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        
        glUniform1i(glGetUniformLocation(combineShader, "textureOut"), 2); //program must be active
        glBindImageTexture(2, textureOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F); 
        
        glDispatchCompute((width / 32) + 1, (height / 32) + 1, 1);
        glUseProgram(0);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    // Blit(bokehTextureOut, textureOut, width, height);
    //Writes the output at half the size

    //bokeh pass at half size

    //blit bokeh pass in textureOut
    //compute shader that will sample the low res texture, and with bilinear filtering it will blur it

    //Bokeh pass

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

    lightPosition = glm::normalize(glm::vec3(0, 46, 0));

    cam = GL_Camera(glm::vec3(0, 10, 0));

    InitGeometryBuffer();



    postProcessStack.postProcesses.push_back(new GrayScalePostProcess(false));
    postProcessStack.postProcesses.push_back(new ContrastBrightnessPostProcess());
    postProcessStack.postProcesses.push_back(new ChromaticAberationPostProcess());
    postProcessStack.postProcesses.push_back(new DepthOfFieldPostProcess(positionTexture, windowWidth, windowHeight));
    postProcessStack.postProcesses.push_back(new PixelizePostProcess(false));
    postProcessStack.postProcesses.push_back(new GodRaysPostProcess(&lightPosition, cam.GetViewMatrixPtr(), cam.GetProjectionMatrixPtr()));
    postProcessStack.postProcesses.push_back(new ToneMappingPostProcess());
    
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

    ImGui::DragFloat3("Light position", glm::value_ptr(lightPosition));

	ImGui::Separator();

    ImGui::Text("Post Processes");
	ImGui::Separator();
    // for(int i=0; i< postProcessStack.postProcesses.size(); i++)
    // {
    //     postProcessStack.postProcesses[i]->RenderGui();
    //     ImGui::Separator();
    // }

    static PostProcess *selectedPostProcess=nullptr;
    for (int n = 0; n < postProcessStack.postProcesses.size(); n++)
    {
        PostProcess *item = postProcessStack.postProcesses[n];
        
        ImGui::PushID(n);
        ImGui::Checkbox("", &postProcessStack.postProcesses[n]->enabled);
        ImGui::PopID();
        ImGui::SameLine();
        bool isSelected=false;
        if(ImGui::Selectable(item->name.c_str(), &isSelected))
        {
            selectedPostProcess = item;
        }

        if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
        {
            int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
            if (n_next >= 0 && n_next < postProcessStack.postProcesses.size())
            {
                postProcessStack.postProcesses[n] = postProcessStack.postProcesses[n_next];
                postProcessStack.postProcesses[n_next] = item;
                ImGui::ResetMouseDragDelta();
            }
        }

        if (isSelected)
            ImGui::SetItemDefaultFocus();    
    }    

	ImGui::Separator();
    
    if(selectedPostProcess != nullptr)
    {
        selectedPostProcess->RenderGui();
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
        glUniform3fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "lightPosition"), 1, glm::value_ptr(lightPosition));
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

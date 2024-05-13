#include "RSM.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
bool RSM::InitReflectiveShadowMap()
{
    ReflectiveShadowMap.renderShader = GL_Shader("shaders/rsm/gBuffer.vert", "", "shaders/rsm/gBuffer.frag");
    
	glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 projectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -500, 500);
	ReflectiveShadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;

	glGenTextures(4, (GLuint*)&ReflectiveShadowMap.textures);

    //Position
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ReflectiveShadowMap.width, ReflectiveShadowMap.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Normal
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ReflectiveShadowMap.width, ReflectiveShadowMap.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Flux
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.fluxTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ReflectiveShadowMap.width, ReflectiveShadowMap.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Depth
	glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, ReflectiveShadowMap.width, ReflectiveShadowMap.height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB);
    
	glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &ReflectiveShadowMap.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, ReflectiveShadowMap.framebuffer);

    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ReflectiveShadowMap.textures.positionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, ReflectiveShadowMap.textures.normalTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, ReflectiveShadowMap.textures.fluxTexture, 0);
    unsigned int attachments[3] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, &attachments[0]);  

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ReflectiveShadowMap.depthTexture, 0);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error creating framebuffer" << std::endl;
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    return true;
}

void RSM::DrawSceneToReflectiveShadowMap() {
	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

	// Bind shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, ReflectiveShadowMap.framebuffer);
	
	glViewport(0,0, ReflectiveShadowMap.width, ReflectiveShadowMap.height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for(int i=0; i<Meshes.size(); i++) {
        glUseProgram(MeshShader.programShaderObject);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, Meshes[i]->material->diffuseTexture.glTex);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "diffuseTexture"), 0);

		Meshes[i]->RenderDepthOnly(ReflectiveShadowMap.depthViewProjectionMatrix, ReflectiveShadowMap.renderShader.programShaderObject);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

void RSM::DrawSceneToGeometryBuffer() {
	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

	// Bind shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, lowResGeometryBuffer.framebuffer);
	
	glViewport(0,0, lowResGeometryBuffer.width, lowResGeometryBuffer.height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(lowResGeometryBuffer.renderShader.programShaderObject);


    glActiveTexture(GL_TEXTURE0 + 7);
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.depthTexture);
    glUniform1i(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "rsmDepth"), 7);

    glActiveTexture(GL_TEXTURE0 + 8);
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.positionTexture);
    glUniform1i(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "rsmPosition"), 8);

    glActiveTexture(GL_TEXTURE0 + 9);
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.normalTexture);
    glUniform1i(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "rsmNormal"), 9);

    glActiveTexture(GL_TEXTURE0 + 10);
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.fluxTexture);
    glUniform1i(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "rsmFlux"), 10);
    

    glActiveTexture(GL_TEXTURE0 + 11);
    glBindTexture(GL_TEXTURE_2D, randomTexture);
    glUniform1i(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "randomTexture"), 11);
    
    glUniform1f(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "maxDistance"), maxDistance);
    glUniform1f(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "intensity"), intensity);
    glUniform1i(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "numSamples"), numSamples);

    glUniform1i(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "showDirect"), (int)showDirect);
    glUniform1i(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "showIndirect"), (int)showIndirect);
    
    glUniformMatrix4fv(glGetUniformLocation(lowResGeometryBuffer.renderShader.programShaderObject, "shadowMapViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(ReflectiveShadowMap.depthViewProjectionMatrix));


    for(int i=0; i<Meshes.size(); i++) {
		Meshes[i]->Render(cam, lowResGeometryBuffer.renderShader.programShaderObject);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

void RSM::CreateRandomTexture() {
	std::default_random_engine eng;
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    eng.seed((unsigned int) std::time(0));
	glm::vec3* randomData = new glm::vec3[randomTextureSize];
	for (int i = 0; i < randomTextureSize; ++i) {
		float r1 = dist(eng);
		float r2 = dist(eng);
		randomData[i].x = r1 * std::sin(2 * PI * r2);
		randomData[i].y = r1 * std::cos(2 * PI * r2);
		randomData[i].z = r1 * r1;
	}
	glGenTextures(1, &randomTexture);
	glBindTexture(GL_TEXTURE_2D, randomTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, randomTextureSize, 1, 0, GL_RGB, GL_FLOAT, randomData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	delete[] randomData;
}

void RSM::CreateLowResGeometryBuffer()
{
    glGenTextures(3, (GLuint*)&lowResGeometryBuffer.textures);

    //Position
    glBindTexture(GL_TEXTURE_2D, lowResGeometryBuffer.textures.positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, lowResGeometryBuffer.width, lowResGeometryBuffer.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Normal
    glBindTexture(GL_TEXTURE_2D, lowResGeometryBuffer.textures.normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, lowResGeometryBuffer.width, lowResGeometryBuffer.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Flux
    glBindTexture(GL_TEXTURE_2D, lowResGeometryBuffer.textures.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, lowResGeometryBuffer.width, lowResGeometryBuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
	glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &lowResGeometryBuffer.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, lowResGeometryBuffer.framebuffer);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lowResGeometryBuffer.textures.positionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lowResGeometryBuffer.textures.normalTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, lowResGeometryBuffer.textures.colorTexture, 0);
    unsigned int attachments[3] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, &attachments[0]);  

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &lowResGeometryBuffer.depthStencilAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, lowResGeometryBuffer.depthStencilAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, lowResGeometryBuffer.width, lowResGeometryBuffer.height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, lowResGeometryBuffer.depthStencilAttachment); // now actually attach it

    glEnable(GL_DEPTH_TEST);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << lowResGeometryBuffer.width << "  " << lowResGeometryBuffer.height << std::endl;
    } else {
        std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    lowResGeometryBuffer.renderShader = GL_Shader("shaders/rsm/rsmGBuffer.vert", "", "shaders/rsm/rsmGBuffer.frag");
}



RSM::RSM() {
}

void RSM::Load() {

    MeshShader = GL_Shader();
    std::ifstream stream("shaders/RSM/MeshShader.vert");
    std::stringstream vertBuffer;
    vertBuffer << stream.rdbuf();
    MeshShader.vertSrc= vertBuffer.str();

    stream = std::ifstream ("shaders/RSM/MeshShader.frag");
    std::stringstream fragBuffer;
    fragBuffer << stream.rdbuf();
    MeshShader.fragSrc= fragBuffer.str();

    stream.close();
    std::cout << "StandardShaders:Compile: Compiling unlitMeshShader" << std::endl; 
    MeshShader.Compile();      

    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
    }

    lightDirection = glm::normalize(glm::vec3(-0.3, 0.9, -0.25));

    cam = GL_Camera(glm::vec3(0, 10, 0));  

    InitReflectiveShadowMap();
    CreateRandomTexture();
    glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -500, 500);
    ReflectiveShadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;
    DrawSceneToReflectiveShadowMap();

    CreateLowResGeometryBuffer();
}

void RSM::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::SliderFloat("Maximum distance", &maxDistance, 0.0f, 0.2f);
    ImGui::SliderFloat("intensity", &intensity, 0.0f, 30.0f);
    ImGui::SliderInt("numSamples", &numSamples, 1, 512);
    
    ImGui::Checkbox("show direct", &showDirect);
    ImGui::Checkbox("show indirect", &showIndirect);
    
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void RSM::Render() {
    if(lightDirectionChanged)
    {
        glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
        glm::mat4 projectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -500, 500);
        ReflectiveShadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;
        DrawSceneToReflectiveShadowMap();
    }

    DrawSceneToGeometryBuffer();

    //glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    for(int i=0; i<Meshes.size(); i++)
    {
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

        glUseProgram(MeshShader.programShaderObject);

        glActiveTexture(GL_TEXTURE0 + 7);
        glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.depthTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "rsmDepth"), 7);

        glActiveTexture(GL_TEXTURE0 + 8);
        glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.positionTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "rsmPosition"), 8);

        glActiveTexture(GL_TEXTURE0 + 9);
        glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.normalTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "rsmNormal"), 9);

        glActiveTexture(GL_TEXTURE0 + 10);
        glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.fluxTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "rsmFlux"), 10);

        glActiveTexture(GL_TEXTURE0 + 11);
        glBindTexture(GL_TEXTURE_2D, randomTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "randomTexture"), 11);

        glActiveTexture(GL_TEXTURE0 + 12);
        glBindTexture(GL_TEXTURE_2D, lowResGeometryBuffer.textures.normalTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "lowResNormals"), 12);

        glActiveTexture(GL_TEXTURE0 + 13);
        glBindTexture(GL_TEXTURE_2D, lowResGeometryBuffer.textures.positionTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "lowResPositions"), 13);
        
        glActiveTexture(GL_TEXTURE0 + 14);
        glBindTexture(GL_TEXTURE_2D, lowResGeometryBuffer.textures.colorTexture);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "lowResRSM"), 14);
        
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "maxDistance"), maxDistance);
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "intensity"), intensity);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "numSamples"), numSamples);

        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "showDirect"), (int)showDirect);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "showIndirect"), (int)showIndirect);
        
        glUniform2f(glGetUniformLocation(MeshShader.programShaderObject, "windowSize"), (float)windowWidth, (float)windowHeight);
    
        glUniformMatrix4fv(glGetUniformLocation(MeshShader.programShaderObject, "shadowMapViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(ReflectiveShadowMap.depthViewProjectionMatrix));

        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
        glm::vec3 test(1,2,3);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
                
        Meshes[i]->Render(cam, MeshShader.programShaderObject);
    }
}

void RSM::Unload() {
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

    MeshShader.Unload();
    glDeleteTextures(1, &randomTexture);
    
    //ReflectiveShadowMap
    glDeleteFramebuffers(1, &ReflectiveShadowMap.framebuffer);
    glDeleteTextures(3, (GLuint*)&ReflectiveShadowMap.textures);
    glDeleteTextures(1, &ReflectiveShadowMap.depthTexture);
    ReflectiveShadowMap.renderShader.Unload();
    
    //lowResGeometryBuffer
    lowResGeometryBuffer.renderShader.Unload();
    glDeleteFramebuffers(1, &lowResGeometryBuffer.framebuffer);
    glDeleteRenderbuffers(1, &lowResGeometryBuffer.depthStencilAttachment);
    glDeleteTextures(3, (GLuint*)&lowResGeometryBuffer.textures);
}


void RSM::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void RSM::LeftClickDown() {
    cam.mousePressEvent(0);
}

void RSM::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void RSM::RightClickDown() {
    cam.mousePressEvent(1);
}

void RSM::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void RSM::Scroll(float offset) {
    cam.Scroll(offset);
}

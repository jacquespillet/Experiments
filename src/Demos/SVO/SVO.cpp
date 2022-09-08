#include "SVO.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>

#include "imgui.h"
bool SVO::InitShadowMap()
{
    shadowMap.shadowShader = GL_Shader("shaders/SVO/shadow.vert", "", "shaders/SVO/shadow.frag");

	// Create framebuffer for shadow map
	glGenFramebuffers(1, &shadowMap.depthFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.depthFramebuffer);

	// Depth texture
	shadowMap.depthTexture.width = shadowMap.depthTexture.height = shadowMap.resolution;

	glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 projectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -500, 500);
	shadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;

	glGenTextures(1, &shadowMap.depthTexture.glTex);
	glBindTexture(GL_TEXTURE_2D, shadowMap.depthTexture.glTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMap.depthTexture.width, shadowMap.depthTexture.height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMap.depthTexture.glTex, 0);
	
    // No color target
	glDrawBuffer(GL_NONE);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error creating framebuffer" << std::endl;
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}


void SVO::DrawSceneToShadowMap() {
	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

	// Bind shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.depthFramebuffer);
	
	glViewport(0,0, shadowMap.depthTexture.width, shadowMap.depthTexture.height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(int i=0; i<Meshes.size(); i++) {
		Meshes[i]->RenderDepthOnly(shadowMap.depthViewProjectionMatrix, shadowMap.shadowShader.programShaderObject);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

void SVO::Init3DTex()
{
    voxelScene.voxelTexture.size = voxelScene.voxelDimensions;  
    glEnable(GL_TEXTURE_3D);
    
    if(!voxelScene.voxelInitialized)
    {
        glGenTextures(1, &voxelScene.voxelTexture.glTex);
        glBindTexture(GL_TEXTURE_3D, voxelScene.voxelTexture.glTex);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Create projection matrices used to project stuff onto each axis in the voxelization step
        float size = voxelScene.voxelGridWorldSize;
        // left, right, bottom, top, zNear, zFar
        glm::mat4 projectionMatrix = glm::ortho(-size*0.5f, size*0.5f, -size*0.5f, size*0.5f, size*0.5f, size*1.5f);
        voxelScene.projX = projectionMatrix * glm::lookAt(glm::vec3(size, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        voxelScene.projY = projectionMatrix * glm::lookAt(glm::vec3(0, size, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
        voxelScene.projZ = projectionMatrix * glm::lookAt(glm::vec3(0, 0, size), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    }
    else
    {
        glBindTexture(GL_TEXTURE_3D, voxelScene.voxelTexture.glTex);
    }

	// Fill 3D texture with empty values
	int numVoxels = voxelScene.voxelTexture.size * voxelScene.voxelTexture.size * voxelScene.voxelTexture.size;
	GLubyte* data = new GLubyte[numVoxels*4];
	for(int i = 0; i < numVoxels*4 ; i++) {
        data[i]=0;
	}

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, voxelScene.voxelTexture.size, voxelScene.voxelTexture.size, voxelScene.voxelTexture.size, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	delete[] data;

	glGenerateMipmap(GL_TEXTURE_3D);
}

void SVO::voxelizeScene() {
    
}

void SVO::BuildOctree()
{
    
}

SVO::SVO() {
    voxelScene.voxelInitialized=false;
}

void SVO::Load() {
    MeshShader = GL_Shader();
    std::ifstream stream("shaders/SVO/MeshShader.vert");
    std::stringstream vertBuffer;
    vertBuffer << stream.rdbuf();
    MeshShader.vertSrc= vertBuffer.str();

    stream = std::ifstream ("shaders/SVO/MeshShader.frag");
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

    InitShadowMap();
    DrawSceneToShadowMap();

    voxelScene.voxelizationShader = GL_Shader("shaders/SVO/voxelization.vert", "shaders/SVO/voxelization.geom", "shaders/SVO/voxelization.frag");
    
    Init3DTex();
    voxelizeScene();
    BuildOctree();
    voxelScene.voxelInitialized=true;

    cam = GL_Camera(glm::vec3(0, 50, 0));  
}

void SVO::RenderGUI() {
    lightDirectionChanged=false;
    // std::cout << "HERE !" << std::endl;
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }
    

    ImGui::SliderFloat("ambient", &directAmbient, 0.0f, 1.0f);
    ImGui::SliderFloat("specular", &specularity, 0.0f, 1.0f);
    
    ImGui::Checkbox("show Occlusion", &showOcclusion);
    ImGui::Checkbox("show direct ambient", &showAmbient);
    ImGui::Checkbox("show indirect diffuse", &showIndirectDiffuse);
    ImGui::Checkbox("show direct diffuse", &showDirectDiffuse);
    ImGui::Checkbox("show specular", &showSpecular);
    
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void SVO::Render() {
    // if(lightDirectionChanged)
    // {
    //     glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
    //     glm::mat4 projectionMatrix = glm::ortho	<float>(-120, 120, -120, 120, -500, 500);
    //     shadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;
    //     DrawSceneToShadowMap();

    //     voxelizeScene();
    // }
    
    // //glEnable(GL_CULL_FACE);
    // glEnable(GL_DEPTH_TEST);
	// glDepthMask(GL_TRUE);
	// glDisable(GL_BLEND);

    // for(int i=0; i<Meshes.size(); i++)
    // {
    //     glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

    //     glUseProgram(MeshShader.programShaderObject);
        
    //     glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
    //     glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
        
    //     glUniformMatrix4fv(glGetUniformLocation(MeshShader.programShaderObject, "shadowMapViewProjectionMatrix"), 1, false, glm::value_ptr(shadowMap.depthViewProjectionMatrix));
        
    //     glActiveTexture(GL_TEXTURE0 + 7);
    //     glBindTexture(GL_TEXTURE_2D, shadowMap.depthTexture.glTex);
    //     glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "shadowMap"), 7);


    //     glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "VoxelGridWorldSize"), voxelScene.voxelGridWorldSize);
    //     glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "VoxelDimensions"), voxelScene.voxelDimensions);
        
    //     glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "showOcclusion"), (int)showOcclusion);
    //     glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "showAmbient"), (int)showAmbient);
    //     glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "showIndirectDiffuse"), (int)showIndirectDiffuse);
    //     glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "showDirectDiffuse"), (int)showDirectDiffuse);
    //     glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "showSpecular"), (int)showSpecular);
    //     glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "directAmbient"), directAmbient);
    //     glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "specularity"), specularity);


    //     glActiveTexture(GL_TEXTURE0 + 8);
    //     glBindTexture(GL_TEXTURE_3D, voxelScene.voxelTexture.glTex);
    //     glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "VoxelTexture"), 8);


    //     Meshes[i]->Render(cam, MeshShader.programShaderObject);
    // }
}

void SVO::Unload() {
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

    glDeleteTextures(1, &shadowMap.depthTexture.glTex);
    glDeleteTextures(1, &voxelScene.voxelTexture.glTex);
 

    voxelScene.voxelInitialized=false;
}


void SVO::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void SVO::LeftClickDown() {
    cam.mousePressEvent(0);
}

void SVO::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void SVO::RightClickDown() {
    cam.mousePressEvent(1);
}

void SVO::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void SVO::Scroll(float offset) {
    cam.Scroll(offset);
}

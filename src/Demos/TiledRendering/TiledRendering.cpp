#include "TiledRendering.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

TiledRendering::TiledRendering() {
}

//Common
void TiledRendering::InitDepthPrepassBuffers()
{
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

    glGenFramebuffers(1, &depthPrepassFramebuffer);  
    glBindFramebuffer(GL_FRAMEBUFFER, depthPrepassFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    depthPrepassShader = GL_Shader("shaders/TiledRendering/depthPrepass.vert", "", "shaders/TiledRendering/depthPrepass.frag");
}

void TiledRendering::InitLights()
{
    lights.resize(numLights);
    glm::vec3 minPosition =bbMin/2.0f;
    glm::vec3 maxPosition =bbMax/2.0f;
    glm::vec3 minColor(1, 1, 1);
    glm::vec3 maxColor(10, 10, 10);
    
    for(int i=0; i<numLights; i++)
    {
        lights[i].position = RandomVec3(minPosition, maxPosition);
        lights[i].currentPosition = glm::vec4(lights[i].position, 1.0);
        lights[i].color = glm::vec4(RandomVec3(minColor, maxColor), 1);
        lights[i].radius = RandomFloat(3, 10);
    }

    glGenBuffers(1, &lightBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numLights * sizeof(PointLight), lights.data(), GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
	
}

void TiledRendering::Load() {

    MeshShader = GL_Shader("shaders/TiledRendering/MeshShader.vert", "", "shaders/TiledRendering/MeshShader.frag");
    
    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    bbMin = glm::vec3(std::numeric_limits<float>::max());
    bbMax = glm::vec3(-std::numeric_limits<float>::max());
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
        glm::vec3 meshBbMin = Meshes[i]->GetMinBoundingBox();
        glm::vec3 meshBbMax = Meshes[i]->GetMaxBoundingBox();
        bbMin = glm::min(bbMin, meshBbMin);
        bbMax = glm::max(bbMax, meshBbMax);
    }
    
    cam = GL_Camera(glm::vec3(0, 10, 0));  

    InitDepthPrepassBuffers();
    InitLights();

    InitForwardPlus();
    InitForwardClustered();
    InitDeferred();

    CreateComputeShader("shaders/TiledRendering/AnimateLights.compute", &animateLightsShader);
    t = clock();
}

void TiledRendering::Unload() {
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
    
    depthPrepassShader.Unload();
    glDeleteFramebuffers(1, &depthPrepassFramebuffer);
    glDeleteTextures(1, &depthTexture);
    glDeleteBuffers(1, &lightBuffer);


    //Tiled
    glDeleteShader(tileComputeShader);
    glDeleteBuffers(1, &tileLightsBuffer);
    

    //Clustered
    glDeleteShader(buildClustersComputeShader);
    glDeleteShader(clusteredLightCullingComputeShader);
    glDeleteShader(findActiveClustersComputeShader);
    glDeleteShader(buildClusterListComputeShader);
    glDeleteShader(activeClusteredLightCullingComputeShader);
    glDeleteBuffers(1, &clustersBuffer);
    glDeleteBuffers(1, &clusterLightIndicesBuffer);
    glDeleteBuffers(1, &clusterGridIndicesBuffer);
    glDeleteBuffers(1, &clusterLightCountBuffer);
    glDeleteBuffers(1, &activeClustersBuffer);
    glDeleteBuffers(1, &activeCounterBuffer);
}

//Forward+
void TiledRendering::InitForwardPlus()
{
    numTilesX = (windowWidth + (windowWidth % 16)) / 16;
	numTilesY = (windowHeight + (windowHeight % 16)) / 16;
	numTiles = numTilesX * numTilesY;

    CreateComputeShader("shaders/TiledRendering/computeTiles.compute", &tileComputeShader);
    glGenBuffers(1, &tileLightsBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, tileLightsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numTiles * sizeof(int) * 1024, 0, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind    
}

void TiledRendering::InitForwardClustered()
{
    totalClusters = clusterX * clusterY * clusterZ;
    glGenBuffers(1, &clustersBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clustersBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, totalClusters * sizeof(Cluster), 0, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    glGenBuffers(1, &clusterLightIndicesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clusterLightIndicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, totalClusters * numLights * sizeof(int), 0, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    glGenBuffers(1, &clusterGridIndicesBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clusterGridIndicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, totalClusters * sizeof(GridIndex), 0, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    glGenBuffers(1, &clusterLightCountBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clusterLightCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int), 0, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    glGenBuffers(1, &activeClustersBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, activeClustersBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, totalClusters * sizeof(uint32_t), 0, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

    glGenBuffers(1, &activeCounterBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, activeCounterBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * sizeof(uint32_t), 0, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, lightBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, tileLightsBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, clustersBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, clusterGridIndicesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, clusterLightIndicesBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, clusterLightCountBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, activeClustersBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, activeCounterBuffer);
    
    
    clusterPixelSizeX = (int)std::ceilf((float)windowWidth / (float)clusterX);
    
    CreateComputeShader("shaders/TiledRendering/BuildClusters.compute", &buildClustersComputeShader);
    CreateComputeShader("shaders/TiledRendering/ClusteredLightCulling.compute", &clusteredLightCullingComputeShader);
    CreateComputeShader("shaders/TiledRendering/FindActiveClusters.compute", &findActiveClustersComputeShader);
    CreateComputeShader("shaders/TiledRendering/BuildClusterList.compute", &buildClusterListComputeShader);
    CreateComputeShader("shaders/TiledRendering/ActiveClusteredLightCulling.compute", &activeClusteredLightCullingComputeShader);
}

void TiledRendering::ComputeTiles()
{
    glUseProgram(tileComputeShader);

	
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(glGetUniformLocation(tileComputeShader, "prepassDepthMap"), 0);


    glUniform1i(glGetUniformLocation(tileComputeShader, "numLights"), numLights);
    glUniformMatrix4fv(glGetUniformLocation(tileComputeShader, "viewMatrix"),1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(tileComputeShader, "projectionMatrix"),1, GL_FALSE, glm::value_ptr(cam.GetProjectionMatrix()));

    glUniform2i(glGetUniformLocation(tileComputeShader, "screenResolution"), windowWidth, windowHeight);
    
    glDispatchCompute(numTilesX, numTilesY, 1);

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glUseProgram(0);
}

//forward Clustered
void TiledRendering::ComputeClusters()
{
    glUseProgram(clusteredLightCullingComputeShader);
    
    glUniform1i(glGetUniformLocation(clusteredLightCullingComputeShader, "numLights"), numLights);
    glUniformMatrix4fv(glGetUniformLocation(clusteredLightCullingComputeShader, "viewMatrix"),1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
    
    glDispatchCompute(1, 1, 6);
    glUseProgram(0);  
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void TiledRendering::ComputeActiveClusters()
{
    //Resets counter    
    uint32_t i = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, clusterLightCountBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t), &i, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(activeClusteredLightCullingComputeShader);
    
    glUniform1i(glGetUniformLocation(activeClusteredLightCullingComputeShader, "numLights"), numLights);
    glUniformMatrix4fv(glGetUniformLocation(activeClusteredLightCullingComputeShader, "viewMatrix"),1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
    
    glBindBuffer( GL_DISPATCH_INDIRECT_BUFFER, activeCounterBuffer);
    glDispatchComputeIndirect(0);
    glUseProgram(0);  
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void TiledRendering::BuildClusters()
{
    glUseProgram(buildClustersComputeShader);
    
    
    glUniform1ui(glGetUniformLocation(buildClustersComputeShader, "clusterPixelSizeX"), clusterPixelSizeX);

    glm::mat4 inverseProj = glm::inverse(cam.GetProjectionMatrix());
    glUniformMatrix4fv(glGetUniformLocation(buildClustersComputeShader, "inverseProjection"),1, GL_FALSE, glm::value_ptr(inverseProj));
    
    glm::vec2 screenResolution(windowWidth, windowHeight);
    glUniform2fv(glGetUniformLocation(buildClustersComputeShader, "screenResolution"),1, glm::value_ptr(screenResolution));

    glUniform1f(glGetUniformLocation(buildClustersComputeShader, "zNear"),cam.GetNearPlane());
    glUniform1f(glGetUniformLocation(buildClustersComputeShader, "zFar"),cam.GetFarPlane());
    
    glDispatchCompute(clusterX, clusterY, clusterZ);
    glUseProgram(0);    
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void TiledRendering::FindActiveClusters()
{
    glUseProgram(findActiveClustersComputeShader);    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(glGetUniformLocation(tileComputeShader, "prepassDepthMap"), 0);
    float zNear = cam.GetNearPlane();
    float zFar = cam.GetFarPlane();
    float scale = (float)clusterZ / std::log2f(zFar / zNear) ;
    float bias    = -((float)clusterZ * std::log2f(zNear) / std::log2f(zFar / zNear)) ;
    glUniform1f(glGetUniformLocation(findActiveClustersComputeShader, "scale"), scale);
    glUniform1f(glGetUniformLocation(findActiveClustersComputeShader, "bias"), bias);
    glUniform1f(glGetUniformLocation(findActiveClustersComputeShader, "zNear"),zNear);
    glUniform1f(glGetUniformLocation(findActiveClustersComputeShader, "zFar"),zFar);
    
    glUniform1ui(glGetUniformLocation(findActiveClustersComputeShader, "tileSizeInPx"), clusterPixelSizeX);
    glUniform3uiv(glGetUniformLocation(findActiveClustersComputeShader, "numClusters"),1, glm::value_ptr(glm::uvec3(clusterX, clusterY, clusterZ)));

    glm::vec2 screenResolution(windowWidth, windowHeight);
    glUniform2fv(glGetUniformLocation(findActiveClustersComputeShader, "screenResolution"),1, glm::value_ptr(screenResolution));
    
    int groupX = windowWidth / 32 + 1;
    int groupY = windowHeight / 32 + 1;
    glDispatchCompute(groupX, groupY, 1);
    glUseProgram(0);    
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    //Resets counter    
    uint32_t i[3] = {0, 1, 1};
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, activeCounterBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * sizeof(uint32_t), &i, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    //Build list
    glUseProgram(buildClusterListComputeShader);    
    glUniform1ui(glGetUniformLocation(buildClusterListComputeShader, "numClusters"), totalClusters);
    
    int groups = totalClusters / 1024 + 1;
    glDispatchCompute(groups, 1, 1);
    glUseProgram(0);    
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

}

//Deferred
void TiledRendering::InitDeferred()
{
    //Position
    glGenTextures(1, (GLuint*)&gpositionTexture);
    glBindTexture(GL_TEXTURE_2D, gpositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Normal
    glGenTextures(1, (GLuint*)&gnormalTexture);
    glBindTexture(GL_TEXTURE_2D, gnormalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Color
    glGenTextures(1, (GLuint*)&gcolorTexture);
    glBindTexture(GL_TEXTURE_2D, gcolorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Depth
    glGenTextures(1, (GLuint*)&gdepthTexture);
	glBindTexture(GL_TEXTURE_2D, gdepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB);

	glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &deferredFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, deferredFramebuffer);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gpositionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gnormalTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gcolorTexture, 0);
    unsigned int attachments[3] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, &attachments[0]);  

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gdepthTexture, 0);

    glEnable(GL_DEPTH_TEST);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << windowWidth << "  " << windowHeight << std::endl;
    } else {
        std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    renderGBufferShader = GL_Shader("shaders/TiledRendering/renderGBuffer.vert", "", "shaders/TiledRendering/renderGBuffer.frag");    
    resolveGBufferShader = GL_Shader("shaders/TiledRendering/resolveGBuffer.vert", "", "shaders/TiledRendering/resolveGBuffer.frag"); 

    
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

//Rendering
void TiledRendering::RenderGUI() {
    ImGui::Begin("Parameters : ");

    // std::string dtText= std::to_string(deltaTime);
    std::string label = "DeltaTime (ms):";
    std::string dts = std::to_string(deltaTime * 1000);
    ImGui::Text(  (label+dts).c_str() );
    
    label = "FPS :";
    std::string fpsS = std::to_string(1.0f / deltaTime);
    ImGui::Text(  (label+fpsS).c_str() );

    static const char* mode = modes[currentMode];
    if (ImGui::BeginCombo("##combo", mode)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < IM_ARRAYSIZE(modes); n++)
		{
			bool is_selected = (mode == modes[n]); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(modes[n], is_selected))
			{
				mode = modes[n];
				if(mode != nullptr) {
					currentMode = n;
				}
			}
			if (is_selected){
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
		}
		ImGui::EndCombo();
	}

    if(currentMode==3)
    {
        ImGui::Checkbox("ActiveClusters", &prepassClusters);
    }

    ImGui::Checkbox("Animate Lights", &animated);

    ImGui::SliderInt("number of lights", &numLights, 0, 1023);

    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void TiledRendering::AnimateLights()
{
    glUseProgram(animateLightsShader);
    
    glUniform1f(glGetUniformLocation(animateLightsShader, "time"), elapsedTime);
    // glUniformMatrix4fv(glGetUniformLocation(animateLightsShader, "viewMatrix"),1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
    
    glDispatchCompute(1, 1, 1);
    glUseProgram(0);  
    glMemoryBarrier(GL_ALL_BARRIER_BITS);    
}

void TiledRendering::Render() {
    clock_t newTime = clock();
    clock_t delta = newTime - t;
    deltaTime = ((float)delta)/CLOCKS_PER_SEC;
    elapsedTime += deltaTime;

    if(animated)
    {
        AnimateLights();
    }

    //All forward
    if(currentMode == 0 || currentMode==1 || currentMode ==3)
    {
        //If tiled or clustered
        if(currentMode==1 || currentMode==3)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, depthPrepassFramebuffer);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(depthPrepassShader.programShaderObject);
            for(int i=0; i<Meshes.size(); i++)
            {
                Meshes[i]->Render(cam, depthPrepassShader.programShaderObject);
            }
            glUseProgram(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        if(currentMode==1)
        {
            ComputeTiles();
        }

        if(currentMode==3)
        {
            BuildClusters();
            if(prepassClusters)
            {
                FindActiveClusters();
                ComputeActiveClusters();
            }
            else
            {
                ComputeClusters();
            }
        }

        //glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        for(int i=0; i<Meshes.size(); i++)
        {
            glUseProgram(MeshShader.programShaderObject);
            
            
            if(currentMode==1)
            {
                glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "numTilesX"), numTilesX);
            }
            if(currentMode==3)
            {
                float zNear = cam.GetNearPlane();
                float zFar = cam.GetFarPlane();
                float scale = (float)clusterZ / std::log2f(zFar / zNear) ;
                float bias    = -((float)clusterZ * std::log2f(zNear) / std::log2f(zFar / zNear)) ;
                glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "scale"), scale);
                glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "bias"), bias);
                glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "zFar"), zFar);
                glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "zNear"), zNear);
                glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "tileSizeInPx"), clusterPixelSizeX);
                glUniform3uiv(glGetUniformLocation(MeshShader.programShaderObject, "numClusters"),1, glm::value_ptr(glm::uvec3(clusterX, clusterY, clusterZ)));
                
            }
            
            glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "mode"), (int)currentMode);
            glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "numLights"), (int)numLights);

            glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
            Meshes[i]->Render(cam, MeshShader.programShaderObject);
        }
    }
    else //Deferred
    {
        glBindFramebuffer(GL_FRAMEBUFFER, deferredFramebuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //Render g buffer
        for(int i=0; i<Meshes.size(); i++)
        {
            glUseProgram(renderGBufferShader.programShaderObject);
            glUniform3fv(glGetUniformLocation(renderGBufferShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
            Meshes[i]->Render(cam, renderGBufferShader.programShaderObject);
        }        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
        glUseProgram(resolveGBufferShader.programShaderObject);
    
        glUniform3fv(glGetUniformLocation(resolveGBufferShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gcolorTexture);
        glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "colorTexture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gpositionTexture);
        glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "positionTexture"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gnormalTexture);
        glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "normalTexture"), 2);
        glUniform1i(glGetUniformLocation(resolveGBufferShader.programShaderObject, "numLights"), (int)numLights);

        screenSpaceQuad.RenderShader(resolveGBufferShader.programShaderObject);
    }

    
    t = clock();
}


void TiledRendering::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void TiledRendering::LeftClickDown() {
    cam.mousePressEvent(0);
}

void TiledRendering::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void TiledRendering::RightClickDown() {
    cam.mousePressEvent(1);
}

void TiledRendering::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void TiledRendering::Scroll(float offset) {
    cam.Scroll(offset);
}

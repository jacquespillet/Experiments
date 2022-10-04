#include "SVO.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

SVO::SVO() {
}

void SVO::SynchronizeGPU()
{
    GLsync syncFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    GLenum waitReturn = GL_UNSIGNALED;
    while(waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED)
    {
        waitReturn = glClientWaitSync(syncFence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
    }
    glDeleteSync(syncFence);
}

inline static GLuint group_x_64(unsigned x) { return (x >> 6u) + ((x & 0x3fu) ? 1u : 0u); }

void SVO::Load() {

    quad = GetQuad();
    renderShader = GL_Shader("shaders/SVO/quad.vert", "", "shaders/SVO/octreeTracer.frag");
    MeshShader = GL_Shader("shaders/Template/MeshShader.vert", "", "shaders/Template/MeshShader.frag");

    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    
    // MeshesFromFile("D:/Boulot/2021/RTGI/resources/models/suzanne/Original.obj", &Meshes, &Materials);
    // MeshesFromFile("D:/Boulot/2021/RTGI/resources/models/suzanne/Original.obj", &Meshes, &Materials);
    // Meshes[1]->SetPos(glm::vec3(1,0,0));
    
    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(-std::numeric_limits<float>::max());
    for(int i=0; i<Meshes.size(); i++)
    {
        for(int j=0; j<Meshes[i]->vertices.size(); j++)
        {
            min = glm::min(Meshes[i]->vertices[j].position, min);
            max = glm::max(Meshes[i]->vertices[j].position, max);
        }
    }
    glm::vec3 size = max - min;
    float extent = std::max(size.x, std::max(size.y, size.z));
    glm::vec3 inverseSize(1.0f / extent);

    glm::vec3 center = (max + min) * 0.5f;

    // for(int i=0; i<Meshes.size(); i++)
    // {
    //     for(int j=0; j<Meshes[i]->vertices.size(); j++)
    //     {
    //         Meshes[i]->vertices[j].position = (Meshes[i]->vertices[j].position - center) * inverseSize;
    //     }
    //     Meshes[i]->RebuildBuffers();
    // }


    for(int i=0; i<Meshes.size(); i++)
    {
        glm::vec3 pos = Meshes[i]->GetPosition() * inverseSize;
        Meshes[i]->SetPos(pos);
        // std::cout << glm::to_string(inverseSize) << std::endl;
        Meshes[i]->SetScale(inverseSize);
    }

    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 0, 0));
    cam.SetTarget(glm::vec3(1.5));
    cam.SetDistance(0.01f);

    int octreeLevels=10;

    //Voxelize
    {
        //Init voxelizer
        int resolution= 1 << octreeLevels;

        //Counter buffer
        glCreateBuffers(1, &Voxelizer.Counter.buffer);
        GLbitfield flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glNamedBufferStorage(Voxelizer.Counter.buffer, sizeof(GLuint), nullptr, flags);
        Voxelizer.Counter.mappedMemory = (GLuint*)glMapNamedBufferRange(Voxelizer.Counter.buffer, 0, sizeof(GLuint), flags);
        *Voxelizer.Counter.mappedMemory = 0;
        
        //Load shader
        Voxelizer.shader = GL_Shader("shaders/SVO/voxelizer.vert","shaders/SVO/voxelizer.geom","shaders/SVO/voxelizer.frag");
        glUseProgram(Voxelizer.shader.programShaderObject);
        glUniform1i(glGetUniformLocation(Voxelizer.shader.programShaderObject, "uVoxelResolution"), resolution);
        glUseProgram(0);

        //load framebuffers and textures
        glCreateRenderbuffers(1, &Voxelizer.Framebuffer.rbo);
        glNamedRenderbufferStorageMultisample(Voxelizer.Framebuffer.rbo, 8, GL_R8, resolution, resolution);
        
        glCreateFramebuffers(1, &Voxelizer.Framebuffer.fbo);
        glNamedFramebufferRenderbuffer(Voxelizer.Framebuffer.fbo, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, Voxelizer.Framebuffer.rbo);
        
        //Voxelize the scene
        glBindFramebuffer(GL_FRAMEBUFFER, Voxelizer.Framebuffer.fbo);
        glViewport(0, 0, resolution, resolution);

        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, Voxelizer.Counter.buffer);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);

        glUseProgram(Voxelizer.shader.programShaderObject);

        //Reset counter to 0
        SynchronizeGPU();
        *Voxelizer.Counter.mappedMemory=0;

        //
        glUniform1i(glGetUniformLocation(Voxelizer.shader.programShaderObject, "uCountOnly"), 1);

        for(int i=0; i<Meshes.size(); i++)
        {    
            glActiveTexture(GL_TEXTURE0);
	        glBindTexture(GL_TEXTURE_2D, Meshes[i]->material->diffuseTexture.glTex);
            glUniform1i(glGetUniformLocation(Voxelizer.shader.programShaderObject, "DiffuseTexture"), 0);
            glUniformMatrix4fv(glGetUniformLocation(Voxelizer.shader.programShaderObject, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(Meshes[i]->modelMatrix));
            Meshes[i]->Render();
        }
        
        //
        SynchronizeGPU();
        Voxelizer.fragmentNum= *Voxelizer.Counter.mappedMemory;

        //
        glCreateBuffers(1, &Voxelizer.FragmentList.buffer);
        glNamedBufferStorage(Voxelizer.FragmentList.buffer, Voxelizer.fragmentNum * sizeof(GLuint) * 2, nullptr, 0);


        //
        SynchronizeGPU();
        *Voxelizer.Counter.mappedMemory=0;

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Voxelizer.FragmentList.buffer);

        glUniform1i(glGetUniformLocation(Voxelizer.shader.programShaderObject, "uCountOnly"), 0);

        for(int i=0; i<Meshes.size(); i++)
        {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Meshes[i]->material->diffuseTexture.glTex);
			glUniform1i(glGetUniformLocation(Voxelizer.shader.programShaderObject, "DiffuseTexture"), 0);

			glUniformMatrix4fv(glGetUniformLocation(Voxelizer.shader.programShaderObject, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(Meshes[i]->modelMatrix));
            Meshes[i]->Render();
        }

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);    

        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glViewport(0, 0, windowWidth, windowHeight);
    }
    
    //Build octree
    {
        //Init
        CreateComputeShader("shaders/SVO/tagNode.comp", &OctreeBuilder.tagNodeShader);
        CreateComputeShader("shaders/SVO/allocNode.comp", &OctreeBuilder.allocNodeShader);
        CreateComputeShader("shaders/SVO/modifyArgs.comp", &OctreeBuilder.modifyArgsShader);


        //Counter buffer
        glCreateBuffers(1, &OctreeBuilder.Counter.buffer);
        GLbitfield flags = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glNamedBufferStorage(OctreeBuilder.Counter.buffer, sizeof(GLuint), nullptr, flags);
        OctreeBuilder.Counter.mappedMemory = (GLuint*)glMapNamedBufferRange(OctreeBuilder.Counter.buffer, 0, sizeof(GLuint), flags);
        *OctreeBuilder.Counter.mappedMemory = 0;

        glCreateBuffers(1, &OctreeBuilder.allocIndirectBuffer);
        glNamedBufferStorage(OctreeBuilder.allocIndirectBuffer, sizeof(DispatchIndirectArgs), nullptr, GL_MAP_WRITE_BIT); 

        glCreateBuffers(1, &OctreeBuilder.buildInfoBuffer);
        glNamedBufferStorage(OctreeBuilder.buildInfoBuffer, sizeof(BuildInfo), nullptr, GL_MAP_WRITE_BIT);


        //Build
        Octree.levels = octreeLevels;

        DispatchIndirectArgs *commandMap;
        commandMap = (DispatchIndirectArgs*) glMapNamedBufferRange(OctreeBuilder.allocIndirectBuffer, 0, sizeof(DispatchIndirectArgs), GL_MAP_WRITE_BIT);
        commandMap->numGroupsX=0;
        commandMap->numGroupsY=commandMap->numGroupsZ = 1;
        glUnmapNamedBuffer(OctreeBuilder.allocIndirectBuffer);

        BuildInfo *buildInfo;
        buildInfo = (BuildInfo*) glMapNamedBufferRange(OctreeBuilder.buildInfoBuffer, 0, sizeof(BuildInfo), GL_MAP_WRITE_BIT);
        buildInfo->fragmentCount = Voxelizer.fragmentNum;
        buildInfo->voxelResolution = 1u << (GLuint)octreeLevels;
        buildInfo->allocBegin = buildInfo->allocNum = 0;
        glUnmapNamedBuffer(OctreeBuilder.buildInfoBuffer);


        int octreeNodeCount = std::max(1000000, Voxelizer.fragmentNum << 2);
        octreeNodeCount = std::min(octreeNodeCount, 500000000);
        glCreateBuffers(1, &Octree.buffer);
        glNamedBufferStorage(Octree.buffer, octreeNodeCount * sizeof(GLuint), nullptr, GL_MAP_WRITE_BIT);

        GLuint *octreeMap;
        octreeMap = (GLuint*) glMapNamedBufferRange(Octree.buffer, 0, octreeNodeCount * sizeof(GLuint), GL_MAP_WRITE_BIT);
        std::fill(octreeMap, octreeMap + octreeNodeCount, 0u);
        glUnmapNamedBuffer(Octree.buffer);

        SynchronizeGPU();
        *OctreeBuilder.Counter.mappedMemory=0;

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Voxelizer.FragmentList.buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Octree.buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, OctreeBuilder.allocIndirectBuffer);
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, OctreeBuilder.allocIndirectBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, OctreeBuilder.buildInfoBuffer);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, OctreeBuilder.Counter.buffer);
        
        GLuint fragListGroupX = group_x_64(Voxelizer.fragmentNum);

        for(int cur=1;cur<=octreeLevels; cur++)
        {
            glUseProgram(OctreeBuilder.tagNodeShader);
            glDispatchCompute(fragListGroupX, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

            if(cur != octreeLevels)
            {
                glUseProgram(OctreeBuilder.modifyArgsShader);
                glDispatchCompute(1,1,1);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
            
                glUseProgram(OctreeBuilder.allocNodeShader);
                glDispatchComputeIndirect(0);
                glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
            }
        }        
    }
}

void SVO::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::Checkbox("Render mesh", &renderMesh);
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void SVO::Render() {
    if(renderMesh)
    {
        glEnable(GL_DEPTH_TEST);
        for(int i=0; i<Meshes.size(); i++)
        {
            glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

            glUseProgram(MeshShader.programShaderObject);
            glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
            glm::vec3 test(1,2,3);
            glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
                    
            glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "normalTextureSet"), (int)normalTextureSet);
            glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "specularTextureSet"), (int)specularTextureSet);
            
            Meshes[i]->Render(cam, MeshShader.programShaderObject);
        }
    }
    else
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Octree.buffer);

        glViewport(0, 0,windowWidth, windowHeight);
        glUseProgram(renderShader.programShaderObject);
        glUniform1i(glGetUniformLocation(renderShader.programShaderObject, "uViewType"), 0);
        glUniform1i(glGetUniformLocation(renderShader.programShaderObject, "uBeamEnable"), 0);
        
        glUniform1i(glGetUniformLocation(renderShader.programShaderObject, "width"), windowWidth);
        glUniform1i(glGetUniformLocation(renderShader.programShaderObject, "height"), windowHeight);
        
        glUniformMatrix4fv(glGetUniformLocation(renderShader.programShaderObject, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(renderShader.programShaderObject, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(cam.GetProjectionMatrix()));
        glUniform3fv(glGetUniformLocation(renderShader.programShaderObject, "camPosition"), 1, glm::value_ptr(cam.worldPosition));
        
        quad->Render();
    }
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

#include "SVO.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

#define BUILD_CPU 0

glm::uvec4 LeafToUvec4(uint32_t val) { return glm::uvec4( (val & 0xffu), ((val >> 8u) & 0xffu), ((val >> 16u) & 0xffu), ((val >> 24u) & 0x3fu) ); }
uint32_t Uvec4ToLeaf(glm::uvec4 vec) { return (std::min(vec.w, 0x3fu) << 24u) | (vec.x & 0xffu) | ((vec.y & 0xffu) << 8u) | ((vec.z & 0xffu) << 16u) | 0xC0000000u; }
uint32_t group_x_64(uint32_t x) { return (x >> 6u) + (((x & 0x3fu) > 0u) ? 1u : 0u); }

uint32_t atomicCompSwap(uint32_t *mem, uint32_t *compare, uint32_t *data)
{
    uint32_t ret = *mem;
    if(*compare == *mem)
    {
        *mem = *data;
    }

    return ret;
}

void SVO::octreeBuilder::TagNode(uint32_t count, std::vector<Voxel> &voxelList)
{
    for(uint32_t i=0; i<count; i++)
    {
        //This shader goes for each voxel generated in the list, and tags wether we should split it further or not.
        if(i >= buildInfo->fragmentCount) continue;
        Voxel voxel = voxelList[i];

        //For the first stage, this is 2 ^ 10 : amount of voxels in one axis.
        uint32_t levelDimension = buildInfo->voxelResolution;
        glm::uvec3 voxelPosition = voxel.position;
        glm::bvec3 levelComparison;

        uint32_t index = 0u;
        uint32_t current = 0u;
        
        do {
            levelDimension >>= 1; //divide by 2
            
            //Check if the current voxel position is greater than the level dimensions
            levelComparison = greaterThanEqual(voxelPosition, glm::uvec3(levelDimension)); //If 
            
            //
            index = current + (uint32_t(levelComparison.x) | (uint32_t(levelComparison.y) << 1u) | (uint32_t(levelComparison.z) << 2u));

            current = octree[index] & 0x3fffffffu;

            voxelPosition -= glm::uvec3(levelComparison) * levelDimension;
        } while(current != 0u && levelDimension > 1u);

        octree[index] |= 0x80000000u;
        if(levelDimension == 1u) {
            //octree[index] = 0xC0000000u | (ufragment.y & 0xffffffu); //termination tag and color data

            //atomic moving average
            uint32_t prev_val = 0;
            uint32_t current_val=0;
            uint32_t new_val = 0xC1000000u | (voxel.color & 0xffffffu);
            
            glm::uvec4 rgba = LeafToUvec4(new_val);
            while( ( current_val = atomicCompSwap(&octree[index], &prev_val, &new_val) ) != prev_val ) {
                prev_val = current_val;
                glm::uvec4 prev_rgba = LeafToUvec4(prev_val);
                prev_rgba *= glm::vec4(prev_rgba.w, prev_rgba.w, prev_rgba.w, 1);
                glm::uvec4 current_rgba = prev_rgba + rgba;
                current_rgba /= glm::vec4(current_rgba.w, current_rgba.w, current_rgba.w, 1);
                new_val = Uvec4ToLeaf(current_rgba);
            }
        }        
    }
}

void SVO::octreeBuilder::ModifyArgs()
{
	buildInfo->allocBegin += buildInfo->allocNum;
	buildInfo->allocNum = ( counter << 3u ) - buildInfo->allocBegin + 8u;
	commandMap->numGroupsX = buildInfo->allocNum;
}

void SVO::octreeBuilder::AllocNode()
{
    for(uint32_t i=0; i<commandMap->numGroupsX; i++)
    {
        if(i >= buildInfo->allocNum) continue;
        uint32_t idx = i + buildInfo->allocBegin;
        if((octree[idx] & 0x80000000u) > 0u)
            octree[idx] = (((counter++) + 1u) << 3u) | 0x80000000u;
    }
}

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
        glUniform1i(glGetUniformLocation(Voxelizer.shader.programShaderObject, "voxelRes"), resolution);
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
        glUniform1i(glGetUniformLocation(Voxelizer.shader.programShaderObject, "doVoxelize"), 0);

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
        glNamedBufferStorage(Voxelizer.FragmentList.buffer, Voxelizer.fragmentNum * sizeof(GLuint) * 4, nullptr, 0);


        //
        SynchronizeGPU();
        *Voxelizer.Counter.mappedMemory=0;

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Voxelizer.FragmentList.buffer);

        glUniform1i(glGetUniformLocation(Voxelizer.shader.programShaderObject, "doVoxelize"), 1);

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

        OctreeBuilder.commandMap = (DispatchIndirectArgs*) glMapNamedBufferRange(OctreeBuilder.allocIndirectBuffer, 0, sizeof(DispatchIndirectArgs), GL_MAP_WRITE_BIT);
        OctreeBuilder.commandMap->numGroupsX=0;
        OctreeBuilder.commandMap->numGroupsY=OctreeBuilder.commandMap->numGroupsZ = 1;
        glUnmapNamedBuffer(OctreeBuilder.allocIndirectBuffer);

        OctreeBuilder.buildInfo = (BuildInfo*) glMapNamedBufferRange(OctreeBuilder.buildInfoBuffer, 0, sizeof(BuildInfo), GL_MAP_WRITE_BIT);
        OctreeBuilder.buildInfo->fragmentCount = Voxelizer.fragmentNum;
        OctreeBuilder.buildInfo->voxelResolution = 1u << (GLuint)octreeLevels;
        OctreeBuilder.buildInfo->allocBegin = OctreeBuilder.buildInfo->allocNum = 0;
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
        

#if BUILD_CPU
        OctreeBuilder.octree.resize(octreeNodeCount);
        std::vector<Voxel> voxelList(Voxelizer.fragmentNum);
        SynchronizeGPU();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, Voxelizer.FragmentList.buffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, Voxelizer.fragmentNum * sizeof(Voxel), voxelList.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        SynchronizeGPU();
        for(int cur=1;cur<=octreeLevels; cur++)
        {
            OctreeBuilder.TagNode(Voxelizer.fragmentNum, voxelList);
            if(cur != octreeLevels)
            {
                OctreeBuilder.ModifyArgs();
                OctreeBuilder.AllocNode();
            }
        }    

        SynchronizeGPU();
        glDeleteBuffers(1, &Octree.buffer);
        glGenBuffers(1, &Octree.buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, Octree.buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, OctreeBuilder.octree.size() * sizeof(uint32_t), OctreeBuilder.octree.data(), GL_STATIC_DRAW); 
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, Octree.buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Octree.buffer);

        SynchronizeGPU();
#else
        GLuint fragListGroupX = Voxelizer.fragmentNum / 1024 + 1;
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
#endif
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

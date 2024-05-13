#include "DeferredDecals.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
DeferredDecals::DeferredDecals() {
}

void DeferredDecals::Load() {
    InitGeometryBuffer();
    InitQuad();
    InitDecal();
    InitDecalFramebuffer();

    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);   
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05, 0.05, 0.05));
        glm::vec3 meshBbMin = Meshes[i]->GetMinBoundingBox();
        glm::vec3 meshBbMax = Meshes[i]->GetMaxBoundingBox();
        aabbs.push_back({{meshBbMin, meshBbMax}});
    }

    lightDirection = glm::normalize(glm::vec3(-0.3, 0.9, -0.25));

    cam = GL_Camera(glm::vec3(0, 1, 0));  
    // cam.SetDistance(0.5f);
    cam.SetSphericalPosition(glm::vec3(0, 0, 1));
}

void DeferredDecals::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat("decalSize", &decalSize, 0, 10);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;


    ImGui::End();
}

void DeferredDecals::Render() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for(int i=0; i<Meshes.size(); i++)
    {
        // int i = rendermesh;
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

        glUseProgram(MeshShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
        glm::vec3 test(1,2,3);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
                
        Meshes[i]->Render(cam, MeshShader.programShaderObject);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, decalFramebuffer);
	
    glDisable(GL_DEPTH_TEST);  
    
    glUseProgram(decalShader.programShaderObject);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glUniform1i(glGetUniformLocation(decalShader.programShaderObject, "positionTexture"), 0);
    
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, decalTexture.glTex);
    glUniform1i(glGetUniformLocation(decalShader.programShaderObject, "decalTexture"), 1);
    
    glm::mat4 modelViewProjectionMatrix = cam.GetProjectionMatrix() * cam.GetViewMatrix() * decalBox->modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(decalShader.programShaderObject, "modelViewProjectionMatrix"),1, GL_FALSE,  glm::value_ptr(modelViewProjectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(decalShader.programShaderObject, "invModel"),1, GL_FALSE,  glm::value_ptr(decalBox->invModelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(decalShader.programShaderObject, "model"),1, GL_FALSE,  glm::value_ptr(decalBox->modelMatrix));
    
    decalBox->RenderShader(decalShader.programShaderObject);
    for(int i=0; i<decals.size(); i++)
    {
        glUseProgram(decalShader.programShaderObject);
        modelViewProjectionMatrix = cam.GetProjectionMatrix() * cam.GetViewMatrix() * decals[i]->modelMatrix;
        glUniformMatrix4fv(glGetUniformLocation(decalShader.programShaderObject, "modelViewProjectionMatrix"),1, GL_FALSE,  glm::value_ptr(modelViewProjectionMatrix));
        glUniformMatrix4fv(glGetUniformLocation(decalShader.programShaderObject, "invModel"),1, GL_FALSE,  glm::value_ptr(decals[i]->invModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(decalShader.programShaderObject, "model"),1, GL_FALSE,  glm::value_ptr(decals[i]->modelMatrix));
        
        decals[i]->RenderShader(decalShader.programShaderObject);
    }
    
    
    glEnable(GL_DEPTH_TEST);  
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(quadShader.programShaderObject);
    
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glUniform1i(glGetUniformLocation(quadShader.programShaderObject, "colorTexture"), 0);
    
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glUniform1i(glGetUniformLocation(quadShader.programShaderObject, "positionTexture"), 1);
    
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glUniform1i(glGetUniformLocation(quadShader.programShaderObject, "normalTexture"), 2);

    glUniform3fv(glGetUniformLocation(quadShader.programShaderObject, "eyePos"), 1, glm::value_ptr(cam.worldPosition));

    screenspaceQuad.RenderShader(quadShader.programShaderObject);
}

void DeferredDecals::InitGeometryBuffer()
{
    MeshShader = GL_Shader("shaders/DeferredDecals/MeshShader.vert", "", "shaders/DeferredDecals/MeshShader.frag");
    
	glGenTextures(1, (GLuint*)&normalTexture);
	glGenTextures(1, (GLuint*)&positionTexture);
	glGenTextures(1, (GLuint*)&colorTexture);
	glGenTextures(1, (GLuint*)&depthTexture);

    //Position
    glBindTexture(GL_TEXTURE_2D, positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Normal
    glBindTexture(GL_TEXTURE_2D, normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Flux
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Depth
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,  GL_COMPARE_R_TO_TEXTURE_ARB);
    
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

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error creating framebuffer " << windowWidth << " " << windowHeight << std::endl;
	}
    else
    {
        std::cout << "Framebuffer OK " << std::endl;
    }

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
}

void DeferredDecals::InitDecalFramebuffer()
{    
    glGenFramebuffers(1, &decalFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, decalFramebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, &attachments[0]);  

    glGenRenderbuffers(1, &decalRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, decalRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); 
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, decalRBO); 

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error creating framebuffer " << windowWidth << " " << windowHeight << std::endl;
	}
    else
    {
        std::cout << "Framebuffer OK " << std::endl;
    }

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredDecals::InitQuad()
{

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
    screenspaceQuad = GL_Mesh(vertices, triangles);	
	quadShader = GL_Shader("shaders/DeferredDecals/quad.vert", "", "shaders/DeferredDecals/quad.frag");    
}

void DeferredDecals::InitDecal()
{
    decalBox = MeshFromFile("resources/models/Plane/Plane.obj", false, 0);
    decalBox->SetPos(glm::vec3(0.5, 0.5, 0));
    decalShader = GL_Shader("shaders/DeferredDecals/decal.vert", "", "shaders/DeferredDecals/decal.frag");    

    TextureCreateInfo infos = {};
    decalTexture = GL_Texture("resources/textures/decal_3.png", infos);
    // decalTexture = GL_Texture("resources/textures/normal.jpg", infos);
}

void DeferredDecals::Unload() {
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
    
    glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &normalTexture);
    glDeleteTextures(1, &positionTexture);
    glDeleteTextures(1, &colorTexture);
    glDeleteTextures(1, &depthTexture);

    glDeleteFramebuffers(1, &decalFramebuffer);
    glDeleteRenderbuffers(1, &decalRBO);

    screenspaceQuad.Unload();
    quadShader.Unload();
    decalShader.Unload();
    decalBox->Unload();
    delete decalBox;
    for(int i=0; i<decals.size(); i++)
    {
        decals[i]->Unload();
        delete decals[i];
    }

    decalTexture.Unload();


}

void DeferredDecals::InstantiateDecal()
{
    GL_Mesh *newDecal = MeshFromFile("resources/models/Plane/Plane.obj", false, 0);
    newDecal->SetModelMatrix(decalBox->modelMatrix);
    decals.push_back(newDecal);
}

void DeferredDecals::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
    glm::vec3 rayOrigin, rayDirection;
    glm::vec2 ndc(x / windowWidth, y / windowHeight);
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    cam.GetScreenRay(ndc, aspectRatio, rayOrigin, rayDirection);

    glm::vec3 invDir = 1.0f / rayDirection; 
    int sign[3];
    sign[0] = (invDir.x < 0); 
    sign[1] = (invDir.y < 0); 
    sign[2] = (invDir.z < 0); 
  
    float closestHit = std::numeric_limits<float>::max();
    glm::vec3 closestNormal;
    
    // std::cout << aabbs.size() << std::endl;
    for(int i=0; i<aabbs.size(); i++)
    {
        bool intersects = RayAABBIntersection(rayOrigin, invDir, sign, aabbs[i]);
        if(intersects)
        {
            for(int j=0; j<Meshes[i]->triangles.size(); j+=3)
            {
                uint32_t i0 = Meshes[i]->triangles[j + 0];
                uint32_t i1 = Meshes[i]->triangles[j + 1];
                uint32_t i2 = Meshes[i]->triangles[j + 2];
                glm::vec3 v0 = Meshes[i]->modelMatrix * glm::vec4(Meshes[i]->vertices[i0].position, 1);
                glm::vec3 v1 = Meshes[i]->modelMatrix * glm::vec4(Meshes[i]->vertices[i1].position, 1);
                glm::vec3 v2 = Meshes[i]->modelMatrix * glm::vec4(Meshes[i]->vertices[i2].position, 1);
                float distance;
                glm::vec2 uv;
            
                bool hit = RayTriangleIntersection(rayOrigin, rayDirection, v0, v1, v2, &distance, &uv);
                
                if(hit && distance < closestHit)
                {
                    closestHit=distance;
                    closestNormal = (1.0f - uv.x - uv.y) * Meshes[i]->vertices[i0].normal + uv.x * Meshes[i]->vertices[i1].normal + uv.y * Meshes[i]->vertices[i2].normal;
                }
            }                
        }
    }

    glm::vec3 hitPos = rayOrigin + closestHit * rayDirection;
    glm::vec3 up = glm::vec3(0,1,0);
    if(glm::dot(up, closestNormal)> 0.9) up = glm::vec3(1, 0, 0);
    glm::mat4 modelMatrix =  glm::inverse(glm::lookAtLH(hitPos, hitPos + closestNormal, up)) * glm::scale(glm::mat4(1.0), glm::vec3(decalSize));
    decalBox->SetModelMatrix(modelMatrix);
}

void DeferredDecals::LeftClickDown() {
    cam.mousePressEvent(0);

    InstantiateDecal();
}

void DeferredDecals::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void DeferredDecals::RightClickDown() {
    cam.mousePressEvent(1);
}

void DeferredDecals::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void DeferredDecals::Scroll(float offset) {
    cam.Scroll(offset);
}

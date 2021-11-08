#include "OIT.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

OIT::OIT() {
}

void OIT::Load() {

    MeshShader = GL_Shader("shaders/OIT/MeshShader.vert", "", "shaders/OIT/MeshShader.frag");

    std::vector<GL_Mesh*> Meshes;
    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
        if(i>=63 && i<=72)
        {
            TransparentMeshes.push_back(Meshes[i]);
        }
        else
        {
            SolidMeshes.push_back(Meshes[i]);
        }
    }

    lightDirection = glm::normalize(glm::vec3(0, -1, 1));

    cam = GL_Camera(glm::vec3(0, 10, 0));  
    
    InitOIT();
}

void OIT::InitOIT()
{
    accumulationShader = GL_Shader("shaders/OIT/Accumulation.vert", "", "shaders/OIT/Accumulation.frag");
    compositionShader = GL_Shader("shaders/OIT/Composite.vert", "", "shaders/OIT/Composite.frag");
    resolveShader = GL_Shader("shaders/OIT/Resolve.vert", "", "shaders/OIT/Resolve.frag");
    {
        glGenTextures(1, &opaqueTexture);
        glBindTexture(GL_TEXTURE_2D, opaqueTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(1, &opaqueDepthTexture);
        glBindTexture(GL_TEXTURE_2D, opaqueDepthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, windowWidth, windowHeight,
                    0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindTexture(GL_TEXTURE_2D, 0);

        glGenFramebuffers(1, &opaqueFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebuffer);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, opaqueTexture, 0);
        unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, &attachments[0]);  
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, opaqueDepthTexture, 0);    

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << windowWidth << "  " << windowHeight << std::endl;
        } else {
            std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
        }
    }

    {
        glGenTextures(1, &accumulationTexture);
        glBindTexture(GL_TEXTURE_2D, accumulationTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(1, &revealTexture);
        glBindTexture(GL_TEXTURE_2D, revealTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenFramebuffers(1, &transparentFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, transparentFramebuffer);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, accumulationTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, revealTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, opaqueDepthTexture, 0); // opaque framebuffer's depth texture
        unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
        glDrawBuffers(2, &attachments[0]);  
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
            std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << windowWidth << "  " << windowHeight << std::endl;
        } else {
            std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

        
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

void OIT::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::SliderFloat("Alpha", &alpha, 0, 1);
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void OIT::Render() {
    //Render solids
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for(int i=0; i<SolidMeshes.size(); i++)
    {
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

        glUseProgram(MeshShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
        glm::vec3 test(1,2,3);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
                        
        SolidMeshes[i]->Render(cam, MeshShader.programShaderObject);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    //Render transparents
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunci(0, GL_ONE, GL_ONE); // accumulation blend target
    glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR); //reveal  blend target
    glBlendEquation(GL_FUNC_ADD);

    // // bind transparent framebuffer to render transparent objects
    glBindFramebuffer(GL_FRAMEBUFFER, transparentFramebuffer);
    // // use a four component float array or a glm::vec4(0.0)
    glm::vec4 zero(0);
    glm::vec4 one(1);
    glClearBufferfv(GL_COLOR, 0, glm::value_ptr(zero)); 
    glClearBufferfv(GL_COLOR, 1, glm::value_ptr(one));
    
    for(int i=0; i<TransparentMeshes.size(); i++)
    {
        glUseProgram(accumulationShader.programShaderObject);
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);
 
        glUniform1f(glGetUniformLocation(accumulationShader.programShaderObject, "a"), alpha);
        
        TransparentMeshes[i]->Render(cam, accumulationShader.programShaderObject);
    }

    // Composite
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glBindFramebuffer(GL_FRAMEBUFFER, opaqueFramebuffer);

    // use composite shader
    glUseProgram(compositionShader.programShaderObject);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, accumulationTexture);
    glUniform1i(glGetUniformLocation(compositionShader.programShaderObject, "accum"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, revealTexture);
    glUniform1i(glGetUniformLocation(compositionShader.programShaderObject, "reveal"), 1);
    
    screenSpaceQuad.RenderShader(compositionShader.programShaderObject);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // draw screen quad
    glUseProgram(resolveShader.programShaderObject);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opaqueTexture);
    glUniform1i(glGetUniformLocation(resolveShader.programShaderObject, "tex"), 0);
    screenSpaceQuad.RenderShader(resolveShader.programShaderObject);
}

void OIT::Unload() {
for(int i=0; i<Materials.size(); i++)
    {
        Materials[i]->Unload();
        delete Materials[i];
    }
    for(int i=0; i<SolidMeshes.size(); i++)
    {
        SolidMeshes[i]->Unload();
        delete SolidMeshes[i];
    }
    for(int i=0; i<TransparentMeshes.size(); i++)
    {
        TransparentMeshes[i]->Unload();
        delete TransparentMeshes[i];
    }
}


void OIT::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void OIT::LeftClickDown() {
    cam.mousePressEvent(0);
}

void OIT::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void OIT::RightClickDown() {
    cam.mousePressEvent(1);
}

void OIT::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void OIT::Scroll(float offset) {
    cam.Scroll(offset);
}

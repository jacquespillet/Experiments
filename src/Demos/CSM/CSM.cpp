#include "CSM.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

CSM::CSM() {
}

std::vector<glm::vec4> CSM::ComputeWorldSpaceCorners(const glm::mat4& projectionMatrix, const glm::mat4 &viewMatrix)
{
    glm::mat4 invVP = glm::inverse(projectionMatrix * viewMatrix);
    std::vector<glm::vec4> corners;

    for(int x=-1; x<=1; x+=2)
    {
        for(int y=-1; y<=1; y+=2)
        {
            for(int z=-1; z<=1; z+=2)
            {
                glm::vec4 pt = invVP * glm::vec4((float)x, (float)y, (float)z, 1.0);
				pt /= pt.w;
				corners.push_back(pt);
            }
        }
    }

    return corners;
}

glm::mat4 CSM::ComputeLightVPMatrix(std::vector<glm::vec4>& corners)
{
    glm::vec3 center(0);
    for(int i=0; i<corners.size(); i++)
    {
        center += glm::vec3(corners[i]);
    }
    center /= corners.size();

    glm::mat4 lightView = glm::lookAt(center + lightDirection, center, glm::vec3(0,1,0));
    
    //The projection is an axis aligned bounding box in view space.
    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(std::numeric_limits<float>::min());
    for(int i=0; i<corners.size(); i++)
    {
        glm::vec3 cornerViewSpace = lightView * corners[i];
        min = glm::min(min, cornerViewSpace);
        max = glm::max(max, cornerViewSpace);
    }

    if(min.z < 0.0f) {
        min.z *= zMultiplicator;
    }
    else
    {
        min.z /= zMultiplicator;
    }
    if(max.z < 0.0f) {
        max.z /= zMultiplicator;
    }
    else
    {
        max.z *= zMultiplicator;
    }

    glm::mat4 lightProjection = glm::ortho(min.x, max.x, min.y, max.y, min.z, max.z);

    return lightProjection * lightView;
}

void CSM::ComputeLightSpaceMatrices()
{
    lightMatrices.clear();
    std::vector<glm::vec4> corners;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            float near = cam.GetNearPlane();
            float far = shadowCascadeLevels[i];
			float fov = cam.GetFov();
            glm::mat4 perspective = glm::perspective( glm::radians(fov), (float)windowWidth / (float)windowHeight, near,far);
            corners = ComputeWorldSpaceCorners(perspective, cam.GetViewMatrix());
            lightMatrices.push_back(ComputeLightVPMatrix(corners));
        }
        else if (i < shadowCascadeLevels.size())
        {
            
            float near = shadowCascadeLevels[i - 1];
            float far = shadowCascadeLevels[i];
            glm::mat4 perspective = glm::perspective( glm::radians(cam.GetFov()), (float)windowWidth / (float)windowHeight, near,far);
            corners = ComputeWorldSpaceCorners(perspective, cam.GetViewMatrix());
            lightMatrices.push_back(ComputeLightVPMatrix(corners));
        }
        else
        {
            float near = shadowCascadeLevels[i - 1];
            float far = cam.GetFarPlane();
            glm::mat4 perspective = glm::perspective( glm::radians(cam.GetFov()), (float)windowWidth / (float)windowHeight, near,far);
            corners = ComputeWorldSpaceCorners(perspective, cam.GetViewMatrix());
            lightMatrices.push_back(ComputeLightVPMatrix(corners));
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    for (size_t i = 0; i < lightMatrices.size(); ++i)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &lightMatrices[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}


void CSM::InitCSM()
{
    shadowCascadeLevels = { cam.GetFarPlane() / 50.0f, cam.GetFarPlane() / 25.0f, cam.GetFarPlane() / 10.0f, cam.GetFarPlane() / 2.0f };
    
    glGenTextures(1, &lightDepthMap);
    glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMap);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
    depthMapResolution, depthMapResolution,
    int(shadowCascadeLevels.size()) + 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glm::vec4 borderColor(1,1,1,1);
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(borderColor));

    
    glGenFramebuffers(1, &lightFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, lightFramebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, lightDepthMap, 0);
    // glFramebufferTexture(GL_FRAMEBUFFER, GL_TEXTURE_2D_ARRAY, lightDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer error" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shadowMapShader = GL_Shader("shaders/CSM/shadowMap.vert", "shaders/CSM/shadowMap.geom", "shaders/CSM/shadowMap.frag");

    glGenBuffers(1, &matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 16, nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, matricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    ComputeLightSpaceMatrices();
}

void CSM::Load() {

    MeshShader = GL_Shader("shaders/CSM/MeshShader.vert", "", "shaders/CSM/MeshShader.frag");

    lightDirection = glm::normalize(glm::vec3(-0.2f, 0.8f, 1.0f));
    cam = GL_Camera(glm::vec3(0, 10, 0));  
    InitCSM();

    MeshesFromFile("resources/models/rungholt/rungholt.obj", &Meshes, &Materials);
    // MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);

    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.1f, 0.1f, 0.1f));
    }

}

void CSM::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }

    ImGui::SliderFloat("zMult", &zMultiplicator, 1, 100);
    ImGui::SliderFloat("bias", &bias, 0.0f, 0.1f);
    ImGui::Checkbox("View cascades", &viewCascades);
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void CSM::Render() {
    // if(lightDirectionChanged)
    {
        // getLightSpaceMatrices();
        ComputeLightSpaceMatrices();
    }
    //Render depth map
    glUseProgram(shadowMapShader.programShaderObject);
    glBindFramebuffer(GL_FRAMEBUFFER, lightFramebuffer);
    glViewport(0, 0, depthMapResolution, depthMapResolution);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);  // peter panning
    for(int i=0; i<Meshes.size(); i++)
    {
        glUseProgram(shadowMapShader.programShaderObject);
        glUniformMatrix4fv(glGetUniformLocation(shadowMapShader.programShaderObject, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(Meshes[i]->modelMatrix));
        Meshes[i]->RenderShader(shadowMapShader.programShaderObject);
    }
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, windowWidth, windowHeight);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    for(int i=0; i<Meshes.size(); i++)
    {
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

        glUseProgram(MeshShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
                
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "cascadeCount"), (int)shadowCascadeLevels.size());
        for (size_t j = 0; j < shadowCascadeLevels.size(); ++j)
        {
            std::string name = "cascadePlaneDistances[" + std::to_string(j) + "]";
            glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, name.c_str()), shadowCascadeLevels[j]);
        }
        glUniformMatrix4fv(glGetUniformLocation(MeshShader.programShaderObject, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
        
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "bias"), bias);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "viewCascades"), (int)viewCascades);
        
        glActiveTexture(GL_TEXTURE0 + 8);
        glBindTexture(GL_TEXTURE_2D_ARRAY, lightDepthMap);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "shadowMap"), 8);
        
        Meshes[i]->Render(cam, MeshShader.programShaderObject);
    }
}

void CSM::Unload() {
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

    glDeleteTextures(1, &lightDepthMap);
    glDeleteFramebuffers(1, &lightFramebuffer);
    glDeleteBuffers(1, &matricesUBO);
    shadowMapShader.Unload();

}


void CSM::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void CSM::LeftClickDown() {
    cam.mousePressEvent(0);
}

void CSM::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void CSM::RightClickDown() {
    cam.mousePressEvent(1);
}

void CSM::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void CSM::Scroll(float offset) {
    cam.Scroll(offset);
}

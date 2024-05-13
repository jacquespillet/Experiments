#include "ISM.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

ISM::ISM() {
}

bool ISM::CreateReflectiveShadowMap()
{
    ReflectiveShadowMap.renderShader = GL_Shader("shaders/ISM/rsm.vert", "", "shaders/ISM/rsm.frag");
    
	glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 projectionMatrix = glm::ortho	<float>(-ReflectiveShadowMap.orthoSize, ReflectiveShadowMap.orthoSize, -ReflectiveShadowMap.orthoSize, ReflectiveShadowMap.orthoSize, -500, 500);
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

    CreateSamplingPattern();
    
    return true;
}

float RandomUnilateral()
{
    return (float)std::rand() / (float)RAND_MAX; 
}

void ISM::CreateSamplingPattern()
{
    uint32_t width = (uint32_t)std::sqrt(numSamples);

    std::vector<glm::vec2> localSamples(numSamples, glm::vec2(0,0));
    samples = std::vector<glm::vec2> (numSamples, glm::vec2(0,0));
    
    float cellSize = 1.0f / (width);

    for(uint32_t i=0; i<width;i++) {
        for(uint32_t j=0; j<width;j++) {
            samples[i * width + j].x = i * cellSize + RandomUnilateral() * cellSize;
            samples[i * width + j].y = j * cellSize + RandomUnilateral() * cellSize;
        }
    }

        
    if(width * width < numSamples)
    {
        for(uint32_t i=width * width-1; i<numSamples; i++)
        {
            samples[i].x = RandomUnilateral();
            samples[i].y = RandomUnilateral();
        }
    }

    for(uint32_t SingleSampleIndex=0; SingleSampleIndex<numSamples; SingleSampleIndex++) {        
        // samples[SingleSampleIndex].x = (samples[SingleSampleIndex+0].x);
        // samples[SingleSampleIndex].y = (samples[SingleSampleIndex+0].y);
        samples[SingleSampleIndex].x = RandomUnilateral();
        samples[SingleSampleIndex].y = RandomUnilateral();
    }

    glGenTextures(1, &randomTexture);
	glBindTexture(GL_TEXTURE_2D, randomTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, numSamples, 1, 0, GL_RG, GL_FLOAT, samples.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void ISM::CreateSphere()
{
    sphereShader = GL_Shader("shaders/ISM/indirect.vert", "", "shaders/ISM/indirect.frag");
    float radius = 1.0f;
    int numSlices = 32;
    std::vector<uint32_t> triangles;
    std::vector<GL_Mesh::Vertex> vertices;
    for(int x=0, inx = 0; x<=numSlices; x++) {
        for(int y=0; y<=numSlices; y++, inx++) {
            float xAngle = ((float)x / (float)numSlices) * (float)PI;
            float yAngle = ((float)y / (float)numSlices) * (float)TWO_PI;
            
            float posx = radius * std::sin(xAngle) * std::cos(yAngle);
            float posz = radius * std::sin(xAngle) * std::sin(yAngle);
            float posy = radius * std::cos(xAngle);

            vertices.push_back(
                {
                    glm::vec3(posx, posy, posz),
                    glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(0,0,0),
                    glm::vec2(0,0)
                }
            );

            if(y < numSlices && x < numSlices) {
                triangles.push_back(inx + 1);
                triangles.push_back(inx);
                triangles.push_back(inx + numSlices+1);

                triangles.push_back(inx + numSlices + 1);
                triangles.push_back(inx);
                triangles.push_back(inx + numSlices);
            } else if(x < numSlices){ // If last of the row
                triangles.push_back(inx + 1);
                triangles.push_back(inx);
                triangles.push_back(inx - numSlices);

                triangles.push_back(inx + 1);
                triangles.push_back(inx);
                triangles.push_back(inx + numSlices);
            }

        }        
    }

    sphere = GL_Mesh(vertices, triangles);
}

void ISM::DrawSceneToReflectiveShadowMap() {
    glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 projectionMatrix = glm::ortho	<float>(-ReflectiveShadowMap.orthoSize, ReflectiveShadowMap.orthoSize, -ReflectiveShadowMap.orthoSize, ReflectiveShadowMap.orthoSize, -500, 500);
    ReflectiveShadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;        
    
	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

	// Bind shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, ReflectiveShadowMap.framebuffer);
	
	glViewport(0,0, ReflectiveShadowMap.width, ReflectiveShadowMap.height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for(int i=0; i<Meshes.size(); i++) {
        glUseProgram(ReflectiveShadowMap.renderShader.programShaderObject);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, Meshes[i]->material->diffuseTexture.glTex);
        glUniform1i(glGetUniformLocation(ReflectiveShadowMap.renderShader.programShaderObject, "diffuseTexture"), 0);
        
        glUniform3fv(glGetUniformLocation(ReflectiveShadowMap.renderShader.programShaderObject, "mat_diffuse"),1, glm::value_ptr(Meshes[i]->material->diffuse));
        
        
		Meshes[i]->RenderDepthOnly(ReflectiveShadowMap.depthViewProjectionMatrix, ReflectiveShadowMap.renderShader.programShaderObject);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

void ISM::CreateGeometryBuffer()
{
    deferredRenderer.geometryBuffer.width = windowWidth;
    deferredRenderer.geometryBuffer.height = windowHeight;

    glGenTextures(3, (GLuint*)&deferredRenderer.geometryBuffer.textures);

    //Position
    glBindTexture(GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, deferredRenderer.geometryBuffer.width, deferredRenderer.geometryBuffer.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Normal
    glBindTexture(GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, deferredRenderer.geometryBuffer.width, deferredRenderer.geometryBuffer.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Flux
    glBindTexture(GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, deferredRenderer.geometryBuffer.width, deferredRenderer.geometryBuffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
	glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &deferredRenderer.geometryBuffer.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, deferredRenderer.geometryBuffer.framebuffer);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.positionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.normalTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.colorTexture, 0);
    unsigned int attachments[3] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, &attachments[0]);  

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &deferredRenderer.geometryBuffer.depthStencilAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, deferredRenderer.geometryBuffer.depthStencilAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, deferredRenderer.geometryBuffer.width, deferredRenderer.geometryBuffer.height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, deferredRenderer.geometryBuffer.depthStencilAttachment); // now actually attach it

    glEnable(GL_DEPTH_TEST);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cout << "Framebuffer:Constructor: ERROR:: Framebuffer is not complete!" << deferredRenderer.geometryBuffer.width << "  " << deferredRenderer.geometryBuffer.height << std::endl;
    } else {
        std::cout << "Framebuffer:Constructor:  Framebuffer OK!"<<std::endl;
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    deferredRenderer.geometryBuffer.renderShader = GL_Shader("shaders/ISM/gBuffer.vert", "", "shaders/ISM/gBuffer.frag");
}

void ISM::CreateDeferredRenderer()
{
    CreateGeometryBuffer();

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
    deferredRenderer.screenSpaceQuad = GL_Mesh(vertices, triangles);
    deferredRenderer.renderingShader = GL_Shader("shaders/ISM/deferred.vert", "", "shaders/ISM/deferred.frag");
}


void ISM::DrawSceneToGeometryBuffer() {
	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

	// Bind shadow map framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, deferredRenderer.geometryBuffer.framebuffer);
	
	glViewport(0,0, deferredRenderer.geometryBuffer.width, deferredRenderer.geometryBuffer.height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(int i=0; i<Meshes.size(); i++) {
        glUseProgram(deferredRenderer.geometryBuffer.renderShader.programShaderObject);
        glActiveTexture(GL_TEXTURE0 + 6);
        glBindTexture(GL_TEXTURE_2D, Meshes[i]->material->diffuseTexture.glTex);
        glUniform1i(glGetUniformLocation(deferredRenderer.geometryBuffer.renderShader.programShaderObject, "diffuseTexture"), 6);
        glUniform3fv(glGetUniformLocation(deferredRenderer.geometryBuffer.renderShader.programShaderObject, "mat_diffuse"),1, glm::value_ptr(Meshes[i]->material->diffuse));
		Meshes[i]->Render(cam, deferredRenderer.geometryBuffer.renderShader.programShaderObject);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

void ISM::Load() {

    MeshShader = GL_Shader("shaders/ISM/MeshShader.vert", "", "shaders/ISM/MeshShader.frag");

    MeshesFromFile("resources/models/Cornell/CornellBox.obj", &Meshes, &Materials);

    lightDirection = glm::normalize(glm::vec3(0.3, 0.2, 1));

    cam = GL_Camera(glm::vec3(0, 0, 0));  

    CreateReflectiveShadowMap();
    CreateDeferredRenderer();
    CreateSphere();
}

void ISM::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }
    // ImGui::Checkbox("Render scene", &renderScene);
    // ImGui::Checkbox("Render pointcloud", &renderPointCloud);

        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::End();
}

void ISM::Render() {
    //if(lightDirectionChanged)
    {
        DrawSceneToReflectiveShadowMap();
        DrawSceneToGeometryBuffer();
    }
    glUseProgram(deferredRenderer.renderingShader.programShaderObject);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.positionTexture);
    glUniform1i(glGetUniformLocation(deferredRenderer.renderingShader.programShaderObject, "positionTexture"), 0);
    
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.normalTexture);
    glUniform1i(glGetUniformLocation(deferredRenderer.renderingShader.programShaderObject, "normalTexture"), 1);
		
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, deferredRenderer.geometryBuffer.textures.colorTexture);
    glUniform1i(glGetUniformLocation(deferredRenderer.renderingShader.programShaderObject, "colorTexture"), 2);
    
    glUniform3fv(glGetUniformLocation(deferredRenderer.renderingShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(deferredRenderer.renderingShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
    
    deferredRenderer.screenSpaceQuad.RenderShader(deferredRenderer.renderingShader.programShaderObject);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, deferredRenderer.geometryBuffer.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
    0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glm::mat4 viewProjectionMatrix = cam.GetProjectionMatrix() * cam.GetViewMatrix();
    for(uint32_t i=0; i<numSamples; i++)
    {
        glUseProgram(sphereShader.programShaderObject);
        
        glUniformMatrix4fv(glGetUniformLocation(sphereShader.programShaderObject, "viewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
        glUniform1i(glGetUniformLocation(sphereShader.programShaderObject, "index"), i);
        
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, randomTexture);
        glUniform1i(glGetUniformLocation(sphereShader.programShaderObject, "randomTexture"), 0);
        
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.positionTexture);
        glUniform1i(glGetUniformLocation(sphereShader.programShaderObject, "rsmPositionTexture"), 1);
        
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.fluxTexture);
        glUniform1i(glGetUniformLocation(sphereShader.programShaderObject, "rsmFluxTexture"), 2);
        
        sphere.RenderShader(sphereShader.programShaderObject);
    }
    
    // To account for this, we splat a quadrilateral in screen space at the po-
    // sition of the point light. The splat must be big enough to cover all
    // fragments that can receive significant light. This size can be eas-
    // ily computed from the intensity of the pixel light and its distance
    // to the camera. Details on this computation and tighter bounds are
    // described in the next section.


    
    
    
    // if(renderPointCloud)
    // {
    //     RenderPointCloud();
    // }

    // if(renderScene)
    // {
    //     //glEnable(GL_CULL_FACE);
    //     glEnable(GL_DEPTH_TEST);
    //     for(int i=0; i<Meshes.size(); i++)
    //     {
    //         glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

    //         glUseProgram(MeshShader.programShaderObject);
    //         glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
    //         glm::vec3 test(1,2,3);
    //         glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));
            
    //         Meshes[i]->Render(cam, MeshShader.programShaderObject);
    //     }
    // }
}

void ISM::Unload() {
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

    //TODO(Jacques): Delete textures
    //TODO(Jacques): Delete Framebuffers
}


void ISM::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void ISM::LeftClickDown() {
    cam.mousePressEvent(0);
}

void ISM::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void ISM::RightClickDown() {
    cam.mousePressEvent(1);
}

void ISM::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void ISM::Scroll(float offset) {
    cam.Scroll(offset);
}

#include "LPV.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>
bool LPV::InitReflectiveShadowMap()
{
    ReflectiveShadowMap.renderShader = GL_Shader("shaders/LPV/gBuffer.vert", "", "shaders/LPV/gBuffer.frag");
    
	glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 projectionMatrix = glm::ortho	<float>(-orthoSize, orthoSize, -orthoSize, orthoSize, -500, 500);
	ReflectiveShadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;

	glGenTextures(4, (GLuint*)&ReflectiveShadowMap.textures);

    //Position
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ReflectiveShadowMap.width, ReflectiveShadowMap.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Normal
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ReflectiveShadowMap.width, ReflectiveShadowMap.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Flux
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.fluxTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ReflectiveShadowMap.width, ReflectiveShadowMap.height, 0, GL_RGBA, GL_FLOAT, NULL);
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

void LPV::DrawSceneToReflectiveShadowMap() {
	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

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

        glUniform3fv(glGetUniformLocation(ReflectiveShadowMap.renderShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(lightDirection));

		Meshes[i]->RenderDepthOnly(ReflectiveShadowMap.depthViewProjectionMatrix, ReflectiveShadowMap.renderShader.programShaderObject);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
}

bool LPV::InitShadowMap()
{
    shadowMap.shadowShader = GL_Shader("shaders/vxgi/shadow.vert", "", "shaders/vxgi/shadow.frag");

	// Create framebuffer for shadow map
	glGenFramebuffers(1, &shadowMap.depthFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.depthFramebuffer);

	// Depth texture
	shadowMap.depthTexture.width = shadowMap.depthTexture.height = shadowMap.resolution;

	glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
	glm::mat4 projectionMatrix = glm::ortho	<float>(-orthoSize, orthoSize, -orthoSize, orthoSize, -500, 500);
	shadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;

	glGenTextures(1, &shadowMap.depthTexture.glTex);
	glBindTexture(GL_TEXTURE_2D, shadowMap.depthTexture.glTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMap.depthTexture.width, shadowMap.depthTexture.height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB);
    
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


void LPV::DrawSceneToShadowMap() {
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

void LPV::InitLightInjectionTextures()
{
    glEnable(GL_TEXTURE_3D);
    glGenTextures(1, &lightInjectionR);
    glBindTexture(GL_TEXTURE_3D, lightInjectionR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
    
    glGenTextures(1, &lightInjectionG);
    glBindTexture(GL_TEXTURE_3D, lightInjectionG);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
    
    glGenTextures(1, &lightInjectionB);
    glBindTexture(GL_TEXTURE_3D, lightInjectionB);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_3D, 0);

    glGenFramebuffers(1, &lightInjectionFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, lightInjectionFramebuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, lightInjectionR, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, lightInjectionG, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, lightInjectionB, 0);
	
    unsigned int attachments[3] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, &attachments[0]);  

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error creating framebuffer" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);   
}


void LPV::InitLightInjectionBuffers()
{
	glGenVertexArrays(1, &VPLVAO);

	glBindVertexArray(VPLVAO);
    glGenBuffers(1, &VPLVBO);
	glBindBuffer(GL_ARRAY_BUFFER, VPLVBO);

    int vplCount = ReflectiveShadowMap.width * ReflectiveShadowMap.height;

	float *dummy = new float[2 * vplCount];
	//Alocate buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(dummy), dummy, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (sizeof(float)* 2), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	delete dummy;    
}

void LPV::ClearLightInjectionTextures()
{
    GLfloat data[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glClearTexImage(lightInjectionR, 0, GL_RGBA, GL_FLOAT, &data[0]);
    glClearTexImage(lightInjectionG, 0, GL_RGBA, GL_FLOAT, &data[0]);
    glClearTexImage(lightInjectionB, 0, GL_RGBA, GL_FLOAT, &data[0]);
}

void LPV::InjectLight()
{
    glViewport(0, 0, texture3DResolution, texture3DResolution); 
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glm::vec3 vMin(-40, -40, -40);
    float cellSize = 2.5f;

    glBindFramebuffer(GL_FRAMEBUFFER, lightInjectionFramebuffer);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    // //Additive
    glBlendEquation(GL_FUNC_ADD);
    
    glUseProgram(lightInjectionShader.programShaderObject);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.positionTexture);
    glUniform1i(glGetUniformLocation(lightInjectionShader.programShaderObject, "rsmWorldPosition"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.normalTexture);
    glUniform1i(glGetUniformLocation(lightInjectionShader.programShaderObject, "rsmWorldNormal"), 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, ReflectiveShadowMap.textures.fluxTexture);
    glUniform1i(glGetUniformLocation(lightInjectionShader.programShaderObject, "rsmFlux"), 2);
    
    
    glUniform1i(glGetUniformLocation(lightInjectionShader.programShaderObject, "RSMsize"), ReflectiveShadowMap.width);
    glUniform1f(glGetUniformLocation(lightInjectionShader.programShaderObject, "cellSize"), cellSize);
    glUniform3f(glGetUniformLocation(lightInjectionShader.programShaderObject, "gridDim"), (float)texture3DResolution, (float)texture3DResolution, (float)texture3DResolution);
    glUniform3fv(glGetUniformLocation(lightInjectionShader.programShaderObject, "min"), 1, glm::value_ptr(vMin));
        
    int vplCount = ReflectiveShadowMap.width * ReflectiveShadowMap.height;

    glBindVertexArray(VPLVAO);//aktivujeme VAO
    glDrawArrays(GL_POINTS, 0, vplCount);
    glBindVertexArray(0);//deaktivujeme VAO
    glUseProgram(0);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LPV::InitLightPropagationTextures()
{    
    glGenTextures(1, &lightPropagationR);
    glBindTexture(GL_TEXTURE_3D, lightPropagationR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
    
    glGenTextures(1, &lightPropagationG);
    glBindTexture(GL_TEXTURE_3D, lightPropagationG);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
    
    glGenTextures(1, &lightPropagationB);
    glBindTexture(GL_TEXTURE_3D, lightPropagationB);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
    
    glBindTexture(GL_TEXTURE_3D, 0);

    
    propagationTexturesRed.resize(numPropagationSteps);
    propagationTexturesGreen.resize(numPropagationSteps);
    propagationTexturesBlue.resize(numPropagationSteps);
    glGenTextures(numPropagationSteps, propagationTexturesRed.data());
    glGenTextures(numPropagationSteps, propagationTexturesGreen.data());
    glGenTextures(numPropagationSteps, propagationTexturesBlue.data());
    
    propagationTexturesRed[0] = lightInjectionR;
    propagationTexturesGreen[0] = lightInjectionG;
    propagationTexturesBlue[0] = lightInjectionB;

    for(int i=1; i<numPropagationSteps; i++)
    {
        glBindTexture(GL_TEXTURE_3D, propagationTexturesRed[i]);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
        
        glBindTexture(GL_TEXTURE_3D, propagationTexturesGreen[i]);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);
        
        glBindTexture(GL_TEXTURE_3D, propagationTexturesBlue[i]);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, texture3DResolution, texture3DResolution, texture3DResolution, 0, GL_RGBA, GL_FLOAT, nullptr);            
    }

    glBindTexture(GL_TEXTURE_3D, 0);

    propagationFramebuffers.resize(numPropagationSteps);
    for(int i=0; i<numPropagationSteps; i++)
    {
        glGenFramebuffers(1, &propagationFramebuffers[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, propagationFramebuffers[i]);

        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, lightPropagationR, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, lightPropagationG, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, lightPropagationB, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, propagationTexturesRed[i], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, propagationTexturesGreen[i], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, propagationTexturesBlue[i], 0);
        
        unsigned int attachments[6] = {
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5
        };
        glDrawBuffers(6, &attachments[0]);  

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Error creating framebuffer" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);   
    }       
}

void LPV::InitLightPropagationBuffers()
{
    lightPropagationShader = GL_Shader("shaders/LPV/propagation.vert", "shaders/LPV/propagation.geom", "shaders/LPV/propagation.frag");

	glGenVertexArrays(1, &propagationVAO);

	//Bind VAO
	glBindVertexArray(propagationVAO);

	//Generate VBO
	glGenBuffers(1, &propagationVBO);
	//Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, propagationVBO);

	
	std::vector<glm::vec3> coords;
	for (int d = 0; d < texture3DResolution; d++) {
		for (int c = 0; c < texture3DResolution; c++) {
			for (int r = 0; r < texture3DResolution; r++) {
				coords.push_back(glm::vec3((float)r, (float)c, (float)d));
			}
		}
	}
	glBufferData(GL_ARRAY_BUFFER, coords.size() * 3 * sizeof(float), &coords.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(float)* 3), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void LPV::ClearLightPropagationTextures()
{
    GLfloat data[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    glClearTexImage(lightPropagationR, 0, GL_RGBA, GL_FLOAT, &data[0]);
    glClearTexImage(lightPropagationG, 0, GL_RGBA, GL_FLOAT, &data[0]);
    glClearTexImage(lightPropagationB, 0, GL_RGBA, GL_FLOAT, &data[0]);
}

void LPV::PropagateLight()
{
    glViewport(0, 0, texture3DResolution, texture3DResolution); //!! Set vieport to width and height of 3D texture!!
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(lightPropagationShader.programShaderObject);
	
	// glm::vec3 vMin(-40, -40, -40);
    // float cellSize = 2.5f;

    ClearLightPropagationTextures();

    // glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(lightPropagationShader.programShaderObject, "LPVGridR"), 1);
	glUniform1i(glGetUniformLocation(lightPropagationShader.programShaderObject, "LPVGridG"), 2);
	glUniform1i(glGetUniformLocation(lightPropagationShader.programShaderObject, "LPVGridB"), 3);
	glUniform3f(glGetUniformLocation(lightPropagationShader.programShaderObject, "gridDim"), (float)texture3DResolution, (float)texture3DResolution, (float)texture3DResolution);
   for (int i = 1; i < numPropagationSteps; i++) {     
      glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_3D, propagationTexturesRed[i - 1]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_3D, propagationTexturesGreen[i - 1]);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_3D, propagationTexturesBlue[i - 1]);

        glBindFramebuffer(GL_FRAMEBUFFER, propagationFramebuffers[i]);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        //Additive
        glBlendEquation(GL_FUNC_ADD);

        glBindImageTexture(3, propagationTexturesRed[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindImageTexture(4, propagationTexturesGreen[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
        glBindImageTexture(5, propagationTexturesBlue[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);

        glBindVertexArray(propagationVAO);
        glDrawArrays(GL_POINTS, 0, texture3DResolution * texture3DResolution * texture3DResolution);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    for (int i = 1; i < numPropagationSteps; i++) {
        GLfloat data[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        glClearTexImage(propagationTexturesRed[i], 0, GL_RGBA, GL_FLOAT, &data[0]);
        glClearTexImage(propagationTexturesGreen[i], 0, GL_RGBA, GL_FLOAT, &data[0]);
        glClearTexImage(propagationTexturesBlue[i], 0, GL_RGBA, GL_FLOAT, &data[0]);
    }
    glUseProgram(0);
}

LPV::LPV() {
}

void LPV::Load() {

    MeshShader = GL_Shader();
    std::ifstream stream("shaders/LPV/MeshShader.vert");
    std::stringstream vertBuffer;
    vertBuffer << stream.rdbuf();
    MeshShader.vertSrc= vertBuffer.str();

    stream = std::ifstream ("shaders/LPV/MeshShader.frag");
    std::stringstream fragBuffer;
    fragBuffer << stream.rdbuf();
    MeshShader.fragSrc= fragBuffer.str();

    stream.close();
    std::cout << "StandardShaders:Compile: Compiling unlitMeshShader" << std::endl; 
    MeshShader.Compile();      

    // MeshesFromFile("resources/models/crytek-sponza/test/sponza_2.obj", &Meshes, &Materials);
    MeshesFromFile("resources/models/Sponza_gltf/glTF/Sponza.gltf", &Meshes, &Materials);
    // for(int i=0; i<Meshes.size(); i++)
    // {
    //     Meshes[i]->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
    // }

    bbMin = glm::vec3(std::numeric_limits<float>::max());
    bbMax = glm::vec3(-std::numeric_limits<float>::max());
    for(int i=0; i<Meshes.size(); i++)
    {
        Meshes[i]->SetScale(glm::vec3(0.05, 0.05, 0.05));
        glm::vec3 meshBbMin = Meshes[i]->GetMinBoundingBox();
        glm::vec3 meshBbMax = Meshes[i]->GetMaxBoundingBox();
        bbMin = glm::min(bbMin, meshBbMin);
        bbMax = glm::max(bbMax, meshBbMax);
    }

    size = bbMax - bbMin;
    
    
    lightDirection = glm::normalize(glm::vec3(0, 0.94, 0.320));

    cam = GL_Camera(glm::vec3(0, 10, 0));  

    InitReflectiveShadowMap();
    InitShadowMap();

    InitLightInjectionTextures();
    InitLightInjectionBuffers();
    lightInjectionShader = GL_Shader("shaders/LPV/lightInjection.vert", "shaders/LPV/lightInjection.geom", "shaders/LPV/lightInjection.frag");

    InitLightPropagationTextures();
    InitLightPropagationBuffers();
}

void LPV::RenderGUI() {
    ImGui::Begin("Parameters : ");

    glm::vec3 localLightDirection = lightDirection;
    ImGui::SliderFloat3("Light direction", glm::value_ptr(localLightDirection), -1.0f, 1.0f);

    if(localLightDirection != lightDirection)
    {
        lightDirectionChanged=true;
        lightDirection = localLightDirection;
    }    
        
    if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;

    ImGui::Checkbox("display GI", &displayGI);
    ImGui::SliderFloat("GI intensity", &GIIntensity, 0.0, 3.0);
    ImGui::Checkbox("display Shadows", &displayShadows);
    ImGui::Checkbox("display Direct Diffuse", &displayDirectDiffuse);
    ImGui::SliderFloat("ambient", &ambient, 0.0f, 1.0f);
    ImGui::SliderFloat("specular", &specular, 0.0f, 1.0f);
    ImGui::SliderInt("Propagation steps", &numPropagationSteps, 1, 7);

    ImGui::End();
}

void LPV::Render() {
    // if(lightDirectionChanged)
    {
        glm::mat4 viewMatrix = glm::lookAt(lightDirection, glm::vec3(0,0,0), glm::vec3(0,1,0));
        glm::mat4 projectionMatrix = glm::ortho	<float>(-orthoSize, orthoSize, -orthoSize, orthoSize, -500, 500);
        ReflectiveShadowMap.depthViewProjectionMatrix = projectionMatrix * viewMatrix;
        shadowMap.depthViewProjectionMatrix = ReflectiveShadowMap.depthViewProjectionMatrix;
        DrawSceneToShadowMap();
        DrawSceneToReflectiveShadowMap();
        
        ClearLightInjectionTextures();
        InjectLight();

        PropagateLight();
    }

    //TODO(Jacques) : This seems to break the rsm    
    glViewport(0, 0, windowWidth, windowHeight); 
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    
    for(int i=0; i<Meshes.size(); i++)
    {
        glm::vec3 normalizedLightDirection = glm::normalize(lightDirection);

        glUseProgram(MeshShader.programShaderObject);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "lightDirection"), 1, glm::value_ptr(normalizedLightDirection));
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "cameraPosition"), 1, glm::value_ptr(cam.worldPosition));                

        glActiveTexture(GL_TEXTURE0 + 6);
        glBindTexture(GL_TEXTURE_3D, lightPropagationR);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "RAccumulatorLPV_l0"), 6);

        glActiveTexture(GL_TEXTURE0 + 7);
        glBindTexture(GL_TEXTURE_3D, lightPropagationG);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "GAccumulatorLPV_l0"), 7);

        glActiveTexture(GL_TEXTURE0 + 8);
        glBindTexture(GL_TEXTURE_3D, lightPropagationB);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "BAccumulatorLPV_l0"), 8);

        glActiveTexture(GL_TEXTURE0 + 9);
        glBindTexture(GL_TEXTURE_2D, shadowMap.depthTexture.glTex);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "shadowMap"), 9);
        glUniformMatrix4fv(glGetUniformLocation(MeshShader.programShaderObject, "shadowMapViewProjectionMatrix"), 1, false, glm::value_ptr(shadowMap.depthViewProjectionMatrix));

        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "displayGI"), (int)displayGI);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "displayShadows"), (int)displayShadows);
        glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "displayDirectDiffuse"), displayDirectDiffuse);
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "specular"), specular);
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "ambient"), ambient);
        glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "GIIntensity"), GIIntensity);

        glm::vec3 gridDim((float)texture3DResolution);
        glm::vec3 gridMin(-40);
        glm::vec3 cellSize(2.5);
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "gridDim"), 1, glm::value_ptr(gridDim));                
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "allGridMins"), 1, glm::value_ptr(gridMin));                
        glUniform3fv(glGetUniformLocation(MeshShader.programShaderObject, "allCellSizes"), 1, glm::value_ptr(cellSize));                

        Meshes[i]->Render(cam, MeshShader.programShaderObject);
    }
}

void LPV::Unload() {
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

    //ReflectiveShadowMap
    glDeleteFramebuffers(1, &ReflectiveShadowMap.framebuffer);
    glDeleteTextures(3, (GLuint*)&ReflectiveShadowMap.textures);
    glDeleteTextures(1, &ReflectiveShadowMap.depthTexture);
    ReflectiveShadowMap.renderShader.Unload(); 

    glDeleteFramebuffers(1, &shadowMap.depthFramebuffer);
    glDeleteTextures(1, &shadowMap.depthTexture.glTex);
    shadowMap.shadowShader.Unload();

    glDeleteTextures(1, &lightInjectionR); 
    glDeleteTextures(1, &lightInjectionG);
    glDeleteTextures(1, &lightInjectionB);
    glDeleteFramebuffers(1, &lightInjectionFramebuffer);
    lightInjectionShader.Unload();
    glDeleteBuffers(1, &VPLVBO);
    glDeleteVertexArrays(1, &VPLVAO);

    
    glDeleteTextures(1, &lightPropagationR);
    glDeleteTextures(1, &lightPropagationG);
    glDeleteTextures(1, &lightPropagationB);
    lightPropagationShader.Unload();
    glDeleteBuffers(1, &propagationVBO);
    glDeleteVertexArrays(1, &propagationVAO);
    
    glDeleteTextures(7, propagationTexturesRed.data());
    glDeleteTextures(7, propagationTexturesGreen.data());
    glDeleteTextures(7, propagationTexturesBlue.data());
    glDeleteTextures(7, propagationFramebuffers.data());
}


void LPV::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void LPV::LeftClickDown() {
    cam.mousePressEvent(0);
}

void LPV::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void LPV::RightClickDown() {
    cam.mousePressEvent(1);
}

void LPV::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void LPV::Scroll(float offset) {
    cam.Scroll(offset);
}

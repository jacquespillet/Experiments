#include "PicFlipFluid.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include "GL_Helpers/Util.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/glm.hpp>

PicFlipFluid::PicFlipFluid() {
}

float lerp(float a, float b, float t)
{
	return (1.0f - t) * a + t * b;
}

void PicFlipFluid::InitShaders()
{
    pointsShader = GL_Shader("shaders/PicFlipFluid/points.vert", "shaders/PicFlipFluid/points.geom", "shaders/PicFlipFluid/points.frag");

	//Compute shaders
	std::vector<std::string> sources;
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/ResetGrid.compute"}; 
	CreateComputeShader(sources, &resetGridShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/TransferAccumulation.compute"}; 
	CreateComputeShader(sources, &transferAccumulationShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/ApplyTransferToGrid.compute"}; 
	CreateComputeShader(sources, &applyTransferToGridShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/ApplyForces.compute"}; 
	CreateComputeShader(sources, &applyForcesShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/GridToParticle.compute"}; 
	CreateComputeShader(sources, &gridToParticleShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/AdvectParticles.compute"}; 
	CreateComputeShader(sources, &advectParticlesShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/GridProjection.compute"}; 
	CreateComputeShader(sources, &gridProjectionShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/JacobiIteration.compute"}; 
	CreateComputeShader(sources, &jacobiIterationShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/PressureGuess.compute"}; 
	CreateComputeShader(sources, &pressureGuessShader);
	sources = {"shaders/PicFlipFluid/Common.compute", "shaders/PicFlipFluid/PressureUpdate.compute"}; 
	CreateComputeShader(sources, &pressureUpdateShader);
}

void PicFlipFluid::InitBuffers()
{
	glGenBuffers(1, (GLuint*)&particlesBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, NULL, GL_DYNAMIC_COPY); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
	std::string name = "particles";
	glObjectLabel(  GL_BUFFER, particlesBuffer, (GLsizei)name.size(), name.c_str());

	glGenBuffers(1, (GLuint*)&gridBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, NULL, GL_DYNAMIC_COPY); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
	name = "grid";
	glObjectLabel(  GL_BUFFER, gridBuffer, (GLsizei)name.size(), name.c_str());

	glGenBuffers(1, (GLuint*)&transferBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, transferBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 0, NULL, GL_DYNAMIC_COPY); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
	name = "transfer";
	glObjectLabel(  GL_BUFFER, transferBuffer, (GLsizei)name.size(), name.c_str());
	
	//Render buffers
	glGenVertexArrays(1, &pointsVAO);
	glBindVertexArray(pointsVAO);
    glGenBuffers(1, &pointsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);

	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, (sizeof(float)), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void PicFlipFluid::InitFramebuffer()
{
	CreateComputeShader("shaders/PicFlipFluid/BlurDepth.compute", &blurDepthShader);
	
    //Flux
	glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Depth
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, windowWidth, windowHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB);
    
    //Depth
	glGenTextures(1, &depthTexture1);
	glBindTexture(GL_TEXTURE_2D, depthTexture1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Depth
	glGenTextures(1, &horizontalBlurTexture);
	glBindTexture(GL_TEXTURE_2D, horizontalBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    //Depth
	glGenTextures(1, &verticalBlurTexture);
	glBindTexture(GL_TEXTURE_2D, verticalBlurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, windowWidth, windowHeight, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depthTexture1, 0);
    unsigned int attachments[2] = {
        GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1
    };
    glDrawBuffers(2, &attachments[0]);  
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);	
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
    screenspaceQuad = GL_Mesh(vertices, triangles);	
	quadShader = GL_Shader("shaders/PicFlipFluid/quad.vert", "", "shaders/PicFlipFluid/quad.frag");
}


void PicFlipFluid::Load() {
	InitFramebuffer();
	InitShaders();
    InitBuffers();

	Reset();
	cam = GL_Camera(glm::vec3(0, 2, 0));  
	
}

void PicFlipFluid::RenderGUI() {
    ImGui::Begin("Parameters : ");

	ImGui::SliderFloat("timestep", &timestep, 0.01f, 0.1f);
	ImGui::SliderFloat("mouseForce", &mouseForce, 0.01f, 3.0f);
	ImGui::SliderFloat3("Gravity", &gravity[0], -10, 10);
	ImGui::SliderFloat("picFlip blend", &picFlipBlend, 0, 1);
	ImGui::SliderInt("Jacobi iterations", &iters, 1, 60);
	ImGui::SliderFloat("mass", &massFactor, 0.01f, 100.0f);
	if(ImGui::Button("Attract on/off")) {
		attract = !attract;		
	}

	ImGui::Text("Reseting params");
	ImGui::SliderInt("Density", &density, 1, 256);
	ImGui::SliderInt("gridSize", &gridSize, 12, 48);
	if(ImGui::Button("Reset"))
	{
		Reset();
	}
	
	if(ImGui::IsAnyItemActive()) cam.locked=true;
    else cam.locked=false;
    ImGui::End();
}


void PicFlipFluid::Reset()
{
	gridDimensions = {gridSize + 1, gridSize + 1, gridSize + 1};
	gridCellDimensions = {gridSize, gridSize, gridSize};
    boundsMin = {-1, -1, -1};
    boundsMax = {1, 1, 1};
    bounds_size =  boundsMax - boundsMin;
    cell_size  = bounds_size / glm::vec3(gridCellDimensions);
    numCells = gridDimensions.x * gridDimensions.y * gridDimensions.z;

	//Simulation buffers
	std::vector<gridCell> gridData;
	std::vector<particle> particlesData;
	std::vector<transfer> transferData;
	
	//25x25x25
	for (int gz = 0; gz < gridDimensions.z; ++gz) {
		for (int gy = 0; gy < gridDimensions.y; ++gy) {
			for (int gx = 0; gx < gridDimensions.x; ++gx) {

				//integer position
				const glm::ivec3 gpos{gx, gy, gz};
				
				//position in the world (Based on cell size)
				const glm::vec3 cell_pos = GetWorldCoordinate(gpos);

				
				transferData.emplace_back(transfer());
				
				float length = glm::length(glm::vec3(cell_pos.x, cell_pos.y, 0));
				if (length < 2) {
					//Fills the grid
					gridData.emplace_back(gridCell{
						cell_pos,
						glm::vec3(0),
						GRID_FLUID
					});
					
					//Add 8 particles around this position
					if (gx < gridCellDimensions.x && gy < gridCellDimensions.y && gz < gridCellDimensions.z) {
						for (int i = 0; i < density; ++i) {
							const glm::vec3 particle_pos = glm::linearRand(cell_pos, cell_pos + cell_size);
							particlesData.emplace_back(particle{
								particle_pos,
								glm::vec3(0),
								glm::vec4(0.32,0.57,0.79,1.0)
							});
						}
					}
				} else {
					//Sets the grid to 0
					gridData.emplace_back(gridCell{
						cell_pos,
						glm::vec3(0),
						GRID_AIR
					});
				}
			}
		}
	}

	numParticles = (int)particlesData.size();
	std::cout << numParticles << std::endl;
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particlesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)particlesData.size() * sizeof(particle), particlesData.data(), GL_DYNAMIC_COPY); 
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)gridData.size() * sizeof(gridCell), gridData.data(), GL_DYNAMIC_COPY); 
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, transferBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)transferData.size() * sizeof(transfer), transferData.data(), GL_DYNAMIC_COPY); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 
	

}


void PicFlipFluid::SetGridUniforms(GLint shader)
{
	glUniform3fv(glGetUniformLocation(shader, "boundsMin"), 1, glm::value_ptr(boundsMin));
	glUniform3fv(glGetUniformLocation(shader, "boundsMax"), 1, glm::value_ptr(boundsMax));
	glUniform3iv(glGetUniformLocation(shader, "gridDim"), 1, glm::value_ptr(gridDimensions));
}

//Set initial values in all cells of the grid
void PicFlipFluid::ResetGrid()
{
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(resetGridShader);
	SetGridUniforms(resetGridShader);
	glDispatchCompute(gridDimensions.x, gridDimensions.y, gridDimensions.z);
	glUseProgram(0);
}

void PicFlipFluid::TransferVelocities()
{
	int groupSize  = 1024;
	
	//Fill the transfer buffer
 	glUseProgram(transferAccumulationShader);
	SetGridUniforms(transferAccumulationShader);
	int numGroups = numParticles / groupSize + 1;
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(numGroups, 1, 1);
	
	//Apply the transfer buffer to the grid buffer
	numGroups = numCells / 1024 + 1;
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(applyTransferToGridShader);
	SetGridUniforms(applyTransferToGridShader);
	glDispatchCompute(numGroups, 1, 1);
	glUseProgram(0);
}

void PicFlipFluid::ApplyForces()
{
	
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(applyForcesShader);
	SetGridUniforms(applyForcesShader);
	glUniform1f(glGetUniformLocation(applyForcesShader, "dt"), timestep);
	glUniform3fv(glGetUniformLocation(applyForcesShader, "externalForce"), 1, 	glm::value_ptr(gravity));
	glUniform1i(glGetUniformLocation(applyForcesShader, "attract"), (int)attract);
	glDispatchCompute(gridDimensions.x / 10 + 1, gridDimensions.y / 10 + 1, gridDimensions.z / 10 + 1);
	glUseProgram(0);
}

void PicFlipFluid::GridProjection()
{
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(gridProjectionShader);
	SetGridUniforms(gridProjectionShader);
	glUniform1f(glGetUniformLocation(gridProjectionShader, "density"), massFactor);
	
	glUniform1f(glGetUniformLocation(gridProjectionShader, "dt"), timestep);
	glDispatchCompute(gridDimensions.x, gridDimensions.y, gridDimensions.z);
	glUseProgram(0);
}

void PicFlipFluid::PressureSolve()
{
	
	for (int i = 0; i < iters; ++i) {
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		glUseProgram(jacobiIterationShader);
		SetGridUniforms(jacobiIterationShader);
		glDispatchCompute(gridDimensions.x, gridDimensions.y, gridDimensions.z);

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		glUseProgram(pressureGuessShader);
		SetGridUniforms(pressureGuessShader);
		glDispatchCompute(gridDimensions.x, gridDimensions.y, gridDimensions.z);
	}
}
void PicFlipFluid::PressureUpdate()
{
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(pressureUpdateShader);
	glUniform1f(glGetUniformLocation(pressureUpdateShader, "dt"), timestep);
	SetGridUniforms(pressureUpdateShader);
	glUniform1f(glGetUniformLocation(pressureUpdateShader, "density"), massFactor);
	glDispatchCompute(gridDimensions.x, gridDimensions.y, gridDimensions.z);
}

void PicFlipFluid::GridToParticles()
{
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(gridToParticleShader);
	SetGridUniforms(gridToParticleShader);
	glUniform1f(glGetUniformLocation(gridToParticleShader, "picFlipBlend"), picFlipBlend);
	glDispatchCompute(numParticles / 1024 + 1, 1, 1);
	glUseProgram(0);
}

void PicFlipFluid::AdvectParticles()
{
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glUseProgram(advectParticlesShader);
	SetGridUniforms(advectParticlesShader);
	
	glUniform3fv(glGetUniformLocation(advectParticlesShader, "eye"), 1, glm::value_ptr(cam.worldPosition));
	glUniform3fv(glGetUniformLocation(advectParticlesShader, "mousePos"), 1, glm::value_ptr(mousePos));
	glUniform3fv(glGetUniformLocation(advectParticlesShader, "mouseVel"), 1, glm::value_ptr(mouseVel));
        

	glUniform1f(glGetUniformLocation(advectParticlesShader, "dt"), timestep);
	glDispatchCompute(numParticles / 1024 + 1, 1, 1);
	glUseProgram(0);
}

void PicFlipFluid::StepSimulation()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particlesBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gridBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, transferBuffer);

	//Particles to grid
	ResetGrid();
	TransferVelocities();
	
	ApplyForces();
	
	GridProjection();
	
	PressureSolve();
	PressureUpdate();
	
	GridToParticles();
	
	AdvectParticles();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void PicFlipFluid::Render() {
	glEnable(GL_DEPTH_TEST);
        
	StepSimulation();
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
            
	glUseProgram(pointsShader.programShaderObject);
    glUniformMatrix4fv(glGetUniformLocation(pointsShader.programShaderObject, "v"), 1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(pointsShader.programShaderObject, "p"), 1, GL_FALSE, glm::value_ptr(cam.GetProjectionMatrix()));
    glUniform1f(glGetUniformLocation(pointsShader.programShaderObject, "pointSize"), 2);
	BindSSBO(pointsShader.programShaderObject, particlesBuffer, "ParticlesBuffer", 0);
	//bind VAO	
	glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glDrawArrays(GL_POINTS, 0, numParticles);
	glBindVertexArray(0);    
	glUseProgram(0);    

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Blur depth
	int kernelSize = 7;
	float distance = glm::length(cam.worldPosition);
	float normalizedDistance = Clamp01(distance / 20);
	float kernelSizeFloat = lerp(15.0f, 3.0f, normalizedDistance);
	kernelSize = (int)kernelSizeFloat;
	
	glUseProgram(blurDepthShader);
	glBindImageTexture(0, depthTexture1, 0, GL_FALSE, 0, GL_READ_WRITE , GL_R32F);
	glBindImageTexture(1, horizontalBlurTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_R32F);
	glUniform2iv(glGetUniformLocation(blurDepthShader, "direction"),1, glm::value_ptr(glm::ivec2(1, 0)));
	glUniform1i(glGetUniformLocation(blurDepthShader, "kernelSize"),kernelSize);
	

	glDispatchCompute(windowWidth / 32 + 1, windowHeight/32 + 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	// //Blur depth
	glBindImageTexture(0, horizontalBlurTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_R32F);
	glBindImageTexture(1, verticalBlurTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_R32F);
	glUniform2iv(glGetUniformLocation(blurDepthShader, "direction"),1, glm::value_ptr(glm::ivec2(0, 1)));
	glUniform1i(glGetUniformLocation(blurDepthShader, "kernelSize"),kernelSize);
	glDispatchCompute(windowWidth / 32 + 1, windowHeight/32 + 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glUseProgram(quadShader.programShaderObject);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, colorTexture);
	glUniform1i(glGetUniformLocation(quadShader.programShaderObject, "colorTexture"), 0);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, verticalBlurTexture);
	glUniform1i(glGetUniformLocation(quadShader.programShaderObject, "depthTexture"), 1);
	glm::mat4 iv = glm::inverse(cam.GetViewMatrix());
	glm::mat4 ip = glm::inverse(cam.GetProjectionMatrix());
	glUniformMatrix4fv(glGetUniformLocation(quadShader.programShaderObject, "ip"), 1, GL_FALSE, glm::value_ptr(ip));
	glUniformMatrix4fv(glGetUniformLocation(quadShader.programShaderObject, "iv"), 1, GL_FALSE, glm::value_ptr(iv));
	glUniform3fv(glGetUniformLocation(quadShader.programShaderObject, "eyePos"), 1, glm::value_ptr(cam.worldPosition));
	screenspaceQuad.RenderShader(quadShader.programShaderObject);
}

void PicFlipFluid::Unload() {
	glDeleteFramebuffers(1, &framebuffer);
    glDeleteTextures(1, &depthTexture);
    glDeleteTextures(1, &depthTexture1);
    glDeleteTextures(1, &colorTexture);
    screenspaceQuad.Unload();
    quadShader.Unload();

    glDeleteShader(blurDepthShader);
    glDeleteTextures(1, &horizontalBlurTexture);
    glDeleteTextures(1, &verticalBlurTexture);

	glDeleteShader(resetGridShader);
    glDeleteShader(transferAccumulationShader);
    glDeleteShader(applyTransferToGridShader);
    glDeleteShader(applyForcesShader);
    glDeleteShader(gridToParticleShader);
    glDeleteShader(advectParticlesShader);
    glDeleteShader(gridProjectionShader);
    glDeleteShader(jacobiIterationShader);
    glDeleteShader(pressureGuessShader);
    glDeleteShader(pressureUpdateShader);

    glDeleteBuffers(1, &particlesBuffer);
    glDeleteBuffers(1, &gridBuffer);
    glDeleteBuffers(1, &transferBuffer);	

    pointsShader.Unload();
    glDeleteVertexArrays(1, &pointsVAO);
	glDeleteBuffers(1, &pointsVBO);	
}


void PicFlipFluid::MouseMove(float x, float y) {
	
	glm::vec3 mouse_world = glm::unProject(glm::vec3(x, windowHeight - y, 0), cam.GetViewMatrix(), cam.GetProjectionMatrix(), glm::vec4(0, 0, windowWidth, windowHeight));
	mousePos = mouse_world;
	mouseVel = ((mousePos + glm::normalize(mousePos - cam.worldPosition) * 6.f) - (mouseOldPos + glm::normalize(mouseOldPos - cam.worldPosition) * 6.f)) * 10.f;
	mouseVel *= mouseForce;
	mouseOldPos = mousePos;
	
    cam.mouseMoveEvent(x, y);
}

void PicFlipFluid::LeftClickDown() {

    cam.mousePressEvent(0);
}

void PicFlipFluid::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void PicFlipFluid::RightClickDown() {
    cam.mousePressEvent(1);
}

void PicFlipFluid::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void PicFlipFluid::Scroll(float offset) {
	
    cam.Scroll(offset);
}

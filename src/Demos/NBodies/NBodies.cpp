#include "NBodies.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include "GL_Helpers/Util.hpp"
#include <glm/gtc/type_ptr.hpp>
NBodies::NBodies() {
}


void NBodies::InitBuffers()
{
    CreateComputeShader("shaders/NBodies/nbodies.compute", &nbodiesShader);	
	pointsShader = GL_Shader("shaders/NBodies/points.vert", "shaders/NBodies/points.geom", "shaders/NBodies/points.frag");

	glGenBuffers(2, (GLuint*)&particleBuffer);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleBuffer[0] * sizeof(particle), nullptr, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particleBuffer[1] * sizeof(particle), nullptr, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	Reset();


	glGenVertexArrays(1, &pointsVAO);
	glBindVertexArray(pointsVAO);
    glGenBuffers(1, &pointsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);

	float *dummy = new float[bufferSize];
	glBufferData(GL_ARRAY_BUFFER, sizeof(dummy), dummy, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, (sizeof(float)), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	delete[] dummy;

	texture = GL_Texture("resources/textures/Particle.png", {
		GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT, GL_LINEAR, GL_LINEAR,true
	});
}



void NBodies::Load() {
    InitBuffers();
	cam = GL_Camera(glm::vec3(0, 10, 0));  
}

void NBodies::RenderGUI() {
    ImGui::Begin("Parameters : ");

	static const char* mode = modes[0];

	if (ImGui::BeginCombo("##combo", mode)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < IM_ARRAYSIZE(modes); n++)
		{
			bool is_selected = (mode == modes[n]); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(modes[n], is_selected))
			{
				mode = modes[n];
				if(mode != nullptr) {
					currentMode = mode;
					Reset();
				}
			}
			if (is_selected){
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
		}
		ImGui::EndCombo();
	}

	ImGui::DragInt("Particle Count", &bufferSize, 10);
	ImGui::SliderFloat("softening Factor", &softeningFactor, 0.01f, 0.1f);
	ImGui::SliderFloat("damping", &damping, 0.8f, 1.0f);
	ImGui::SliderFloat("timestep", &timestep, 0.001f, 0.1f);
	ImGui::SliderFloat("maxMass", &maxMass, 1.0f, 100.0f);
	ImGui::SliderFloat("pointSize", &pointSize, 1.0, 10.0f);
	ImGui::DragFloat("maxVelocity", &MaxVelocity, 1.0f, 1.0, 10000.0f);

	if(ImGui::Button("Reset"))
	{
		Reset();
	}
    ImGui::End();
}

void NBodies::Reset()
{
	particles = std::vector<particle>(bufferSize);
	
	std::vector<int32_t> randomBuffer(bufferSize);
	std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int32_t>    distr(-30, 30);

	if(currentMode == modes[0]) //Random in cube
	{
		for(int i=0; i<bufferSize; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			particles[i] = {
				glm::vec3((float)distr(generator),
						(float)distr(generator),
						(float)distr(generator)), 
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
		}
	} else if(currentMode == modes[1]) //Random on cube
	{
		int numPerSide = bufferSize / 6;
		int numAdded=0;
		for(int i=0; i<numPerSide; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			particles[i] = {
				glm::vec3(-30,
						(float)distr(generator),
						(float)distr(generator)), 
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
			numAdded++;
		}

		int startIndex = numAdded;
		int endIndex = startIndex + numPerSide;
		for(int i=startIndex; i<endIndex; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			particles[i] = {
				glm::vec3(30,
						(float)distr(generator),
						(float)distr(generator)), 
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
			numAdded++;
		}

		startIndex = numAdded;
		endIndex = startIndex + numPerSide;
		for(int i=startIndex; i<endIndex; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			particles[i] = {
				glm::vec3((float)distr(generator),
						30,
						(float)distr(generator)), 
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
			numAdded++;
		}

		startIndex = numAdded;
		endIndex = startIndex + numPerSide;
		for(int i=startIndex; i<endIndex; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			particles[i] = {
				glm::vec3((float)distr(generator),
						-30,
						(float)distr(generator)), 
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
			numAdded++;
		}

		startIndex = numAdded;
		endIndex = startIndex + numPerSide;
		for(int i=startIndex; i<endIndex; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			particles[i] = {
				glm::vec3((float)distr(generator),
						(float)distr(generator), 
						-30),
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
			numAdded++;
		}
		startIndex = numAdded;
		endIndex = startIndex + numPerSide;
		for(int i=startIndex; i<endIndex; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			particles[i] = {
				glm::vec3((float)distr(generator),
						(float)distr(generator), 
						30),
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
			numAdded++;
		}
	}
	else if(currentMode == modes[2]) //Random on Sphere
	{
		float r = 30;
		for(int i=0; i<bufferSize; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			float theta = RandomFloat(0, PI);
			float phi = RandomFloat(0, 2 * PI);
			float x = r * cos(phi) * sin(theta);
			float y = r * sin(phi) * sin(theta);
			float z = r * cos(theta);
			particles[i] = {
				glm::vec3(x, y, z),
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
		}
	}	
	else if(currentMode == modes[3]) //Random in Sphere
	{
		float r = RandomFloat(1, 10);
		for(int i=0; i<bufferSize; i++)
		{
			// float mass = RandomFloat(0.5, maxMass);
			float mass = maxMass;
			float theta = RandomFloat(0, PI);
			float phi = RandomFloat(0, 2 * PI);
			float x = r * cos(phi) * sin(theta);
			float y = r * sin(phi) * sin(theta);
			float z = r * cos(theta);
			particles[i] = {
				glm::vec3(x, y, z),
				0,
				glm::vec3(0),
				mass,
				glm::vec3(0), 
				1.0f / mass
			};
		}
	}	
	else if(currentMode == modes[4]) //Galaxy
	{
		int numGalaxies = RandomInt(4, 30);
		int numPergalaxies = bufferSize / numGalaxies;
		int numAdded=false;
		for(int i=0; i<numGalaxies; i++)
		{
			float gx = RandomFloat(-30, 30);
			float gy = RandomFloat(-30, 30);
			float gz = RandomFloat(-30, 30);
			int randomAddend = RandomInt(-1000, 1000);
			int actualNumber = std::max(0, numPergalaxies + randomAddend);
			for(int j=0; j<actualNumber; j++)
			{
				if(numAdded >= bufferSize) break;

				float mass = maxMass;
				float r = RandomFloat(1, 5);
				float theta = RandomFloat(0, PI);
				float phi = RandomFloat(0, 2 * PI);
				float x = gx +  r * cos(phi) * sin(theta);
				float y = gy +  r * sin(phi) * sin(theta);
				float z = gz +  r * cos(theta);				
				particles[numAdded] = {
					glm::vec3(x, y, z),
					0,
					glm::vec3(0),
					mass,
					glm::vec3(0), 
					1.0f / mass
				};

				numAdded++;
			}
		}
	}	

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(particle), particles.data(), GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(particle), particles.data(), GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}

void NBodies::UpdateParticles()
{
	oldPingPongInx = pingPongInx;
	
	pingPongInx = pingPongInx++;
	pingPongInx %=2;
	
	glUseProgram(nbodiesShader);
	BindSSBO(nbodiesShader, particleBuffer[pingPongInx], "ParticlesBuffer", 10);
	BindSSBO(nbodiesShader, particleBuffer[oldPingPongInx], "PreviousParticlesBuffer", 11);
	
	glUniform1i(glGetUniformLocation(nbodiesShader, "numParticles"), bufferSize);
	
	glUniform1f(glGetUniformLocation(nbodiesShader, "softeningFactor"), softeningFactor);
	glUniform1f(glGetUniformLocation(nbodiesShader, "damping"), damping);
	glUniform1f(glGetUniformLocation(nbodiesShader, "timestep"), timestep);
	glUniform1f(glGetUniformLocation(nbodiesShader, "maxVelocity"), MaxVelocity);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.glTex);
	glUniform1i(glGetUniformLocation(nbodiesShader, "particleTexture"), 0);
	
	glDispatchCompute(bufferSize / 1024 + 1, 1, 1);
	glMemoryBarrier( GL_ALL_BARRIER_BITS );	
}


void NBodies::Render() {
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	
	UpdateParticles();
	
	glUseProgram(pointsShader.programShaderObject);
    glUniformMatrix4fv(glGetUniformLocation(pointsShader.programShaderObject, "v"), 1, GL_FALSE, glm::value_ptr(cam.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(pointsShader.programShaderObject, "p"), 1, GL_FALSE, glm::value_ptr(cam.GetProjectionMatrix()));
    glUniform1f(glGetUniformLocation(pointsShader.programShaderObject, "pointSize"), pointSize);
	glPointSize(4);
	BindSSBO(pointsShader.programShaderObject, particleBuffer[pingPongInx], "ParticlesBuffer", 10);
	//bind VAO	
	glBindVertexArray(pointsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
    glDrawArrays(GL_POINTS, 0, bufferSize);
	glBindVertexArray(0);    
	glUseProgram(0);    

}

void NBodies::Unload() {
	glDeleteProgram(nbodiesShader);
	glDeleteBuffers(1, &particleBuffer[0]);
	glDeleteBuffers(1, &particleBuffer[1]);
	pointsShader.Unload();
	glDeleteVertexArrays(1, &pointsVAO);
	glDeleteBuffers(1, &pointsVBO);
	texture.Unload();

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}


void NBodies::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void NBodies::LeftClickDown() {
    cam.mousePressEvent(0);
	UpdateParticles();
}

void NBodies::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void NBodies::RightClickDown() {
    cam.mousePressEvent(1);
	UpdateParticles();
}

void NBodies::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void NBodies::Scroll(float offset) {
    cam.Scroll(offset);
}

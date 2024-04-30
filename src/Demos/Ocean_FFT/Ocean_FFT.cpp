#include "Ocean_FFT.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
// #include "imgui_plot.h"

Ocean_FFT::Ocean_FFT() {
}



glm::vec4 gaussRND ()
{
	float noise00 = Clamp01((float)std::rand() / (float)RAND_MAX);
	float noise01 = Clamp01((float)std::rand() / (float)RAND_MAX);
	float noise02 = Clamp01((float)std::rand() / (float)RAND_MAX);
	float noise03 = Clamp01((float)std::rand() / (float)RAND_MAX);

	float u0 = 2.0f * PI * noise00 ;
	float v0 = sqrt ( -2.0f * std::log(noise01)) ;
	float u1 = 2.0f * PI * noise02 ;
	float v1 = sqrt ( -2.0f * std::log(noise03)) ;
	glm::vec4 rnd = glm::vec4 ( v0 * std::cos ( u0 ) , v0 * std::sin ( u0 ) , v1 * std::cos ( u1 ) , v1 * std::sin ( u1 ) ) ;
	return rnd ;
}

void Ocean_FFT::InitInitialHkTextures()
{
	windDirection = glm::vec2(1.0f, 1.0f);
	int N = resolution;
	h0kBuffer.resize(resolution * resolution);
	h0MinuskBuffer.resize(resolution * resolution);

	for(int32_t y=0; y<(int)resolution; y++)
	{
		for(int32_t x=0; x< (int)resolution; x++)
		{
			int flatIndex = y * resolution + x;
			//-256, 256
			glm::vec2 position = glm::vec2 (x,y) - (float)N / 2.0f;
			
			glm::vec2 k = glm::vec2 (2.0 *  PI * position.x/L, 2.0 *  PI * position.y/L ) ;

			float L_ = ( windSpeed * windSpeed )/ g ;
			float mag = length (k ) ;
			if (mag < 0.00001f) mag = 0.00001f;
			float magSq = mag * mag ;

			// sqrt (Ph(k ))/ sqrt (2)
			float h0k = Clamp ( sqrt ((A/(magSq* magSq ))
			* std::pow( glm::dot (glm::normalize (k) , glm::normalize(windDirection)) , 6.0f)
			* std::exp ( - (1.0f/(magSq * L_ * L_) ) )
			* std::exp(- magSq * pow(L/2000.0f ,2.0f)))
			/ std::sqrt(2.0f), 
			-4000.0f, 4000.0f);
			
			float power = pow(glm::dot(glm::normalize(-k), glm::normalize(windDirection)), 6.0f);
			float exp1 = exp(-(1.0f / (magSq * L_ * L_)));
			float exp2 = exp(-magSq * pow(L / 2000.0f, 2.0f));
			// sqrt (Ph(- k ))/ sqrt (2)
			float h0minusk = Clamp ( sqrt ((A/(magSq* magSq ))
			* power
			* exp1
			* exp2)
			/ sqrt ( 2.0f ) , - 4000.0f, 4000.0f);
			glm::vec4 gauss_random = gaussRND ( ) ;
			
			glm::vec2 randomizedh = glm::vec2(gauss_random.x, gauss_random.y) * h0k;
			h0kBuffer[flatIndex] = glm::vec4 ( randomizedh.x,randomizedh.y,  0 , 1 );
			// h0kBuffer[flatIndex] = glm::vec4 ( 1, 0,  0 , 1 );
			
			randomizedh = glm::vec2(gauss_random.z, gauss_random.w) * h0minusk;
			// h0MinuskBuffer[flatIndex] = glm::vec4 (randomizedh.x, randomizedh.y  , 0 , 1 ); 
			h0MinuskBuffer[flatIndex] = glm::vec4 (randomizedh.x, randomizedh.y  , 0 , 1 ); 
		}		
	}

	glGenTextures(1, &h0kTex);
	glBindTexture(GL_TEXTURE_2D, h0kTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution, resolution, 0, GL_RGBA, GL_FLOAT, h0kBuffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	std::string name = "h0ktex";
	glObjectLabel(  GL_TEXTURE, h0kTex, (GLsizei)name.size(), name.c_str());

	glGenTextures(1, &h0MinuskTex);
	glBindTexture(GL_TEXTURE_2D, h0MinuskTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution, resolution, 0, GL_RGBA, GL_FLOAT, h0MinuskBuffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	name = "h0minusktex";
	glObjectLabel(  GL_TEXTURE, h0MinuskTex, (GLsizei)name.size(), name.c_str());

	glGenTextures(1, &hktTexture);
	glBindTexture(GL_TEXTURE_2D, hktTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution, resolution, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	name = "hktTexture";
	glObjectLabel(  GL_TEXTURE, hktTexture, (GLsizei)name.size(), name.c_str());

}

int reverseBytes(int i) {
	return (i << 24) |
		((i & 0xff00) << 8) |
		((i >>  8) & 0xff00) |
		(i >>  24);
}

int reverseBits(int i) {
	// HD, Figure 7-1
	i = (i & 0x55555555) << 1 | (i >> 1) & 0x55555555;
	i = (i & 0x33333333) << 2 | (i >> 2) & 0x33333333;
	i = (i & 0x0f0f0f0f) << 4 | (i >> 4) & 0x0f0f0f0f;

	return reverseBytes(i);
}

uint32_t rotateLeft(uint32_t i, uint32_t distance) {
	return (i << distance) | (i >> -distance);
}


void Ocean_FFT::InitButterflyTexture()
{	
	//Create compute shaders
	CreateComputeShader("shaders/Ocean_FFT/ButterflyTexture.compute", &butterflyTextureComputeShader);
	CreateComputeShader("shaders/Ocean_FFT/ifft.compute", &ifftComputeShader);
	
	//Calculate butterfly texture size
	int butterflyTexHeight = resolution;
	int butterflyTexWidth = (int)std::log2(resolution);
	
	//Create the texture
	glGenTextures(1, &butterflyTexture);
	glBindTexture(GL_TEXTURE_2D, butterflyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, butterflyTexWidth, butterflyTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	std::string name = "butterfly";
	glObjectLabel(  GL_TEXTURE, butterflyTexture, (GLsizei)name.size(), name.c_str());

	//Compute reversed indices
	std::vector<int> reversedIndices(resolution);
	int bits = (int) (std::log(resolution)/std::log(2));
	for (int i = 0; i<(int)resolution; i++)
	{
		int x = reverseBits(i);
		x = (int)rotateLeft((uint32_t)x, (uint32_t)bits);
		reversedIndices[i] = x;
	}
	glGenBuffers(1, &indicesBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, indicesBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, resolution * sizeof(int), reversedIndices.data(), GL_STATIC_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	//Execute the compute shader
	int groupX = butterflyTexWidth;
	int groupY = butterflyTexHeight / 16;
	glUseProgram(butterflyTextureComputeShader);
	BindSSBO(butterflyTextureComputeShader, indicesBuffer, "indices", 1);
	glUniform1i(glGetUniformLocation(butterflyTextureComputeShader, "N"), (int)resolution);
	glUniform1i(glGetUniformLocation(butterflyTextureComputeShader, "twiddleIndices"), 0); //program must be active
	glBindImageTexture(0, butterflyTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	glDispatchCompute(groupX, groupY, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void Ocean_FFT::InitFFTTextures()
{
	glGenTextures(1, &pingPongTexture);
	glBindTexture(GL_TEXTURE_2D, pingPongTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution, resolution, 0, GL_RGBA, GL_FLOAT, h0kBuffer.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	std::string name = "pingpong0";
	glObjectLabel(  GL_TEXTURE, pingPongTexture, (GLsizei)name.size(), name.c_str());
}

void Ocean_FFT::InitDisplacementTextures()
{
	CreateComputeShader("shaders/Ocean_FFT/wave.compute", &waveComputeShader);
	CreateComputeShader("shaders/Ocean_FFT/normals.compute", &normalsComputeShader);
	CreateComputeShader("shaders/Ocean_FFT/displacement.compute", &displacementComputeShader);
	
	glGenTextures(1, &displacementTexture);
	glBindTexture(GL_TEXTURE_2D, displacementTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution, resolution, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);
	std::string name = "displacement";
	glObjectLabel(  GL_TEXTURE, displacementTexture, (GLsizei)name.size(), name.c_str());

	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution, resolution, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	name = "normals";
	glObjectLabel(  GL_TEXTURE, normalTexture, (GLsizei)name.size(), name.c_str());
}

void Ocean_FFT::IFFTGPU()
{
	int group = resolution/16;

	//Write hkt
	glUseProgram(waveComputeShader);
	glUniform1i(glGetUniformLocation(waveComputeShader, "N"), (int)resolution); //program must be active
	glUniform1i(glGetUniformLocation(waveComputeShader, "L"), (int)L); //program must be active
	glUniform1f(glGetUniformLocation(waveComputeShader, "t"), elapsedTime); //program must be active
	
	glUniform1i(glGetUniformLocation(waveComputeShader, "tilde_hkt_dy"), 0); //program must be active
	glBindImageTexture(0, hktTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	glUniform1i(glGetUniformLocation(waveComputeShader, "tilde_h0k"), 3); //program must be active
	glBindImageTexture(3, h0kTex, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	glUniform1i(glGetUniformLocation(waveComputeShader, "tilde_h0minusk"), 4); //program must be active
	glBindImageTexture(4, h0MinuskTex, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	glDispatchCompute(group, group, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	
	//2D IFFT
	int numStages = (int)std::log2(resolution);
	int pingPong = 0;
	glUseProgram(ifftComputeShader);
	
	glUniform1i(glGetUniformLocation(ifftComputeShader, "twiddlesIndices"), 0); //program must be active
	glUniform1i(glGetUniformLocation(ifftComputeShader, "pingpong0"), 1); //program must be active
	glUniform1i(glGetUniformLocation(ifftComputeShader, "pingpong1"), 2); //program must be active
	glBindImageTexture(0, butterflyTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	glBindImageTexture(1, hktTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	glBindImageTexture(2, pingPongTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	for(int stage=0; stage<numStages; stage++)
	{	
		glUniform1i(glGetUniformLocation(ifftComputeShader, "stage"), stage); //program must be active
		glUniform1i(glGetUniformLocation(ifftComputeShader, "direction"), 0); //program must be active
		glUniform1i(glGetUniformLocation(ifftComputeShader, "pingpong"), pingPong); //program must be active
	
		glDispatchCompute(group, group, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		
		pingPong++;
		pingPong %=2;
	}

	
	for(int stage=0; stage<numStages; stage++)
	{
		glUniform1i(glGetUniformLocation(ifftComputeShader, "stage"), stage); //program must be active
		glUniform1i(glGetUniformLocation(ifftComputeShader, "direction"), 1); //program must be active
		glUniform1i(glGetUniformLocation(ifftComputeShader, "pingpong"), pingPong); //program must be active
		
		glDispatchCompute(group, group, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		pingPong++;
		pingPong %=2;
	}

	// Write displacement
	glUseProgram(displacementComputeShader);
	glUniform1i(glGetUniformLocation(displacementComputeShader, "N"), (int)resolution); //program must be active
	glUniform1i(glGetUniformLocation(displacementComputeShader, "pingpong"), pingPong); //program must be active
	
	glUniform1i(glGetUniformLocation(displacementComputeShader, "displacement"), 0); //program must be active
	glBindImageTexture(0, displacementTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	glUniform1i(glGetUniformLocation(displacementComputeShader, "pingpong0"), 1); //program must be active
	glBindImageTexture(1, hktTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	glUniform1i(glGetUniformLocation(displacementComputeShader, "pingpong1"), 2); //program must be active
	glBindImageTexture(2, pingPongTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	glDispatchCompute(group, group, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// // Write displacement
	glUseProgram(normalsComputeShader);
	glUniform1i(glGetUniformLocation(normalsComputeShader, "displacement"), 0); //program must be active
	glBindImageTexture(0, displacementTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	glUniform1i(glGetUniformLocation(normalsComputeShader, "normals"), 1); //program must be active
	glBindImageTexture(1, normalTexture, 0, GL_FALSE, 0, GL_READ_WRITE , GL_RGBA32F);
	
	glDispatchCompute(group, group, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

}

void Ocean_FFT::Load() {
	InitInitialHkTextures();
    InitButterflyTexture();
	InitFFTTextures();
	InitDisplacementTextures();
	
	cam = GL_Camera(glm::vec3(0, 10, 0));  

	oceanShader = GL_Shader("shaders/Ocean_FFT/ocean.vert", "", "shaders/Ocean_FFT/ocean.frag");
	oceanMesh = PlaneMesh(planeSize, planeSize, resolution * 3, resolution * 3);

	lightDirection = glm::normalize(glm::vec3(1,1 ,1));
}

void Ocean_FFT::RenderGUI() {
	bool open = false;
    ImGui::Begin("Parameters : ", &open);
	
	ImGui::Text("Visuals");
	ImGui::SliderFloat("metallic", &metallic, 0.0f, 1.0f); 
	ImGui::SliderFloat("roughness", &roughness, 0.0f, 1.0f); 
	ImGui::SliderFloat("lightIntensity", &lightIntensity, 0.1f, 5.0f); 
	ImGui::SliderFloat("ambient", &ambient, 0.0f, 1.0f); 
	
	ImGui::Text("Height Field");
	ImGui::SliderFloat("height", &height, 0.1f, 3.0f); 

    ImGui::End();
}

void Ocean_FFT::Render() {
	clock_t newTime = clock();
    clock_t delta = newTime - t;
    deltaTime = ((float)delta)/CLOCKS_PER_SEC;
    elapsedTime += deltaTime;

	IFFTGPU();
	

	glUseProgram(oceanShader.programShaderObject);
	glm::mat4 modelViewProjectionMatrix = cam.GetProjectionMatrix() * cam.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(oceanShader.programShaderObject, "modelViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
	glUniform3fv(glGetUniformLocation(oceanShader.programShaderObject, "lightDirection"), 1,glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(oceanShader.programShaderObject, "cameraPosition"), 1,glm::value_ptr(cam.worldPosition));
	glUniform1f(glGetUniformLocation(oceanShader.programShaderObject, "size"), planeSize);
	
	glUniform1f(glGetUniformLocation(oceanShader.programShaderObject, "metallic"), metallic);
	glUniform1f(glGetUniformLocation(oceanShader.programShaderObject, "roughness"), roughness);
	glUniform1f(glGetUniformLocation(oceanShader.programShaderObject, "lightIntensity"), lightIntensity);
	glUniform1f(glGetUniformLocation(oceanShader.programShaderObject, "ambient"), ambient);
	
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, displacementTexture);
	glUniform1i(glGetUniformLocation(oceanShader.programShaderObject, "displacementTexture"), 0);
	
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glUniform1i(glGetUniformLocation(oceanShader.programShaderObject, "normalTexture"), 1);

	oceanMesh->RenderShader(oceanShader.programShaderObject);

	
    t = clock();

}

void Ocean_FFT::Unload() {
	glDeleteTextures(1, &h0kTex);
	glDeleteTextures(1, &h0MinuskTex);
    glDeleteTextures(2, &pingPongTexture);
    glDeleteTextures(1, &butterflyTexture);
    glDeleteTextures(1, &displacementTexture);
    glDeleteTextures(1, &hktTexture);
    glDeleteTextures(1, &normalTexture);
    glDeleteShader(butterflyTextureComputeShader);
    glDeleteShader(ifftComputeShader);
    glDeleteShader(displacementComputeShader);
    glDeleteShader(waveComputeShader);
    glDeleteShader(normalsComputeShader);
    oceanShader.Unload();
    glDeleteBuffers(1, &indicesBuffer);
    oceanMesh->Unload();
	delete oceanMesh;	
}

void Ocean_FFT::MouseMove(float x, float y) {
    cam.mouseMoveEvent(x, y);
}

void Ocean_FFT::LeftClickDown() {
    cam.mousePressEvent(0);
}

void Ocean_FFT::LeftClickUp() {
    cam.mouseReleaseEvent(0);
}

void Ocean_FFT::RightClickDown() {
    cam.mousePressEvent(1);
}

void Ocean_FFT::RightClickUp() {
    cam.mouseReleaseEvent(1);
}

void Ocean_FFT::Scroll(float offset) {
    cam.Scroll(offset);
}

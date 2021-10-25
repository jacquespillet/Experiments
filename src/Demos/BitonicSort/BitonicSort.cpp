#include "BitonicSort.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

BitonicSort::BitonicSort() {
}

void BitonicSort::InitBuffers()
{
    CreateComputeShader("shaders/bitonicSort/bitonicSort.compute", &sortShader);
    CreateComputeShader("shaders/bitonicSort/matrixTranspose.compute", &matrixTransposeShader);

	glGenBuffers(1, &inBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, inBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(uint32_t), nullptr, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	glGenBuffers(1, &tmpBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tmpBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(uint32_t), nullptr, GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	Reset();
}



void BitonicSort::Load() {
    InitBuffers();
}

void BitonicSort::RenderGUI() {
    ImGui::Begin("Parameters : ");



    ImGui::End();
}

void BitonicSort::showValuesOnConsole()
{	
	std::cout << "//////////////////////////////////////////" << std::endl;
	std::vector<uint32_t> storage(bufferSize); // n is the size  
	glGetNamedBufferSubData(inBuffer, 0, bufferSize * sizeof(uint32_t), storage.data());

	std::stringstream ss;
	for(int i=0; i<bufferSize; i++)
	{
		ss << storage[i] << " ";
	}

	std::cout << ss.str() << std::endl;
}

void BitonicSort::Reset()
{
	std::vector<uint32_t> randomBuffer(bufferSize);
	std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<uint32_t>    distr(1, bufferSize);
	for(int i=0; i<bufferSize; i++)
	{
		randomBuffer[i] = distr(generator);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, inBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(uint32_t), randomBuffer.data(), GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}


void BitonicSort::Sort()
{
	Reset();

	uint32_t NUM_ELEMENTS = (uint32_t)bufferSize; //number of elements to sort
	uint32_t MATRIX_WIDTH = BITONIC_BLOCK_SIZE; //Block size
	uint32_t MATRIX_HEIGHT = (uint32_t)NUM_ELEMENTS / BITONIC_BLOCK_SIZE; //Height of the blocks

	//The matrix is an array of size 512x32
	//On each line is one ascending or descending sequence


	for(uint32_t l=0; l<512; l++)
	{
		uint32_t j = 16;
		uint32_t firstIndex =  l & ~j;
		uint32_t secondIndex = l | j;
		uint32_t swapIndex = l ^ j;
		// std::cout << "nj " << nj << std::endl;
		std::cout << l << "  " << firstIndex << " " << secondIndex << "  " << swapIndex <<  std::endl;
	}

	//Create bitonic sequences of size 1024 (512 ascending, 512 descending)
	for (uint32_t level = 2; level <= BITONIC_BLOCK_SIZE; level <<= 1) {
        glUseProgram(sortShader);
		BindSSBO(sortShader, inBuffer, "DataBuf", 10);
		glUniform1ui(glGetUniformLocation(sortShader, "_Level"), level);
		glUniform1ui(glGetUniformLocation(sortShader, "_LevelMask"), level);
        glDispatchCompute(NUM_ELEMENTS / BITONIC_BLOCK_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

	//Merge the bitonic sequences
	//For level = 1024, level < 8096, level*=2
	//1024, 2048, 4096, 8192
	for (uint32_t level = BITONIC_BLOCK_SIZE << 1; level <= NUM_ELEMENTS; level <<= 1) {
        
		// Transpose the data from buffer 1 into buffer 2
		glUseProgram(matrixTransposeShader);
        BindSSBO(matrixTransposeShader, inBuffer, "InputBuf", 10);
        BindSSBO(matrixTransposeShader, tmpBuffer, "DataBuf", 11);
		glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Width"), MATRIX_WIDTH);
		glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Height"), MATRIX_HEIGHT);
        glDispatchCompute(MATRIX_WIDTH / TRANSPOSE_BLOCK_SIZE, MATRIX_HEIGHT / TRANSPOSE_BLOCK_SIZE, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		//
		//Levels : 2, 4, 8, 16
		//LeveMasks : 2, 4, 8, 0
        // Sort the transposed column data
		glUseProgram(sortShader);
        BindSSBO(sortShader, tmpBuffer, "DataBuf", 10);
		glUniform1ui(glGetUniformLocation(sortShader, "_Level"), level / BITONIC_BLOCK_SIZE);
		glUniform1ui(glGetUniformLocation(sortShader, "_LevelMask"), (level & ~NUM_ELEMENTS)/BITONIC_BLOCK_SIZE);
        glDispatchCompute(NUM_ELEMENTS / BITONIC_BLOCK_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Transpose the data from buffer 2 back into buffer 1
        glUseProgram(matrixTransposeShader);
        BindSSBO(matrixTransposeShader, tmpBuffer, "InputBuf", 10);
        BindSSBO(matrixTransposeShader, inBuffer, "DataBuf", 11);
		glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Width"), MATRIX_HEIGHT);
		glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Height"), MATRIX_WIDTH);
        glDispatchCompute(MATRIX_HEIGHT / TRANSPOSE_BLOCK_SIZE, MATRIX_WIDTH / TRANSPOSE_BLOCK_SIZE, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Sort the row data
		//Levels : 512, 512, 512, 512
		//LeveMasks : 1024, 2048, 4096, 8192
		glUseProgram(sortShader);
        BindSSBO(sortShader, inBuffer, "DataBuf", 10);
		glUniform1ui(glGetUniformLocation(sortShader, "_Level"), BITONIC_BLOCK_SIZE);
		glUniform1ui(glGetUniformLocation(sortShader, "_LevelMask"), level);
        glDispatchCompute(NUM_ELEMENTS / BITONIC_BLOCK_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
}

void BitonicSort::Render() {
	// Sort();
}

void BitonicSort::Unload() {
	glDeleteProgram(sortShader);
	glDeleteProgram(matrixTransposeShader);
	
	glDeleteBuffers(1, &inBuffer);
	glDeleteBuffers(1, &tmpBuffer);
}


void BitonicSort::MouseMove(float x, float y) {}

void BitonicSort::LeftClickDown() {
	// Reset();
	// showValuesOnConsole();
}

void BitonicSort::LeftClickUp() {
	
}

void BitonicSort::RightClickDown() {
	Sort();
	showValuesOnConsole();
}

void BitonicSort::RightClickUp() {

}

void BitonicSort::Scroll(float offset) {}

#include "BitonicSort.hpp"

#include <glad/gl.h>
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include <implot.h>

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

	if(multiFrame && !done && started)
	{
		int nthFrame =(int)(1.0f / speed);
		if(frame % nthFrame == 0)
		{
			Step();
		}
	}
    ImGui::Begin("Parameters : ");
	ImGui::SliderFloat("Speed", &speed, 0.0f, 1.0f);
	ImGui::Checkbox("Animated", &multiFrame);
	
	if(ImGui::Button("Reset"))
	{
		Reset();
	}

	if(ImGui::Button("Sort"))
	{
		Sort();
	}


    ImGui::SetNextItemWidth(200);

    if (ImPlot::BeginPlot("##Histograms")) {
        ImPlot::SetupAxes(nullptr,nullptr,ImPlotAxisFlags_AutoFit,ImPlotAxisFlags_AutoFit);
        ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL,0.5f);
        ImPlot::PlotHistogram("Empirical", bufferCPU.data(), bufferCPU.size(), 0, 1.0, ImPlotRange(0,RAND_MAX), ImPlotHistogramFlags_None);
		ImPlot::PlotBars("Bars", Xs.data(), bufferCPU.data(), bufferCPU.size(), 1, ImPlotBarsFlags_None, 0);
        ImPlot::EndPlot();
    }

    ImGui::End();
	frame++;
}

void BitonicSort::DownloadValues()
{	
	std::cout << "//////////////////////////////////////////" << std::endl;
	glGetNamedBufferSubData(inBuffer, 0, bufferSize * sizeof(uint32_t), bufferCPU.data());
}

void BitonicSort::Reset()
{
	std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<uint32_t>    distr(1, bufferSize);
	bufferCPU.resize(bufferSize);
	Xs.resize(bufferSize);
	for(int i=0; i<bufferSize; i++)
	{
		bufferCPU[i] = distr(generator);
		Xs[i] = i;
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, inBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize * sizeof(uint32_t), bufferCPU.data(), GL_STREAM_DRAW); 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}

void BitonicSort::Step()
{
	
	uint32_t NUM_ELEMENTS = (uint32_t)bufferSize; //number of elements to sort
	uint32_t MATRIX_WIDTH = BITONIC_BLOCK_SIZE; //Block size
	uint32_t MATRIX_HEIGHT = (uint32_t)NUM_ELEMENTS / BITONIC_BLOCK_SIZE; //Height of the blocks

	if(iteration0 <= BITONIC_BLOCK_SIZE)
	{
		glUseProgram(sortShader);
		BindSSBO(sortShader, inBuffer, "DataBuf", 10);
		glUniform1ui(glGetUniformLocation(sortShader, "_Level"), iteration0);
		glUniform1ui(glGetUniformLocation(sortShader, "_LevelMask"), iteration0);
		glDispatchCompute(NUM_ELEMENTS / BITONIC_BLOCK_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		
		DownloadValues();

		iteration0 <<= 1;
		return;
	}
	else if(iteration1 <= NUM_ELEMENTS)
	{	
		// Transpose the data from buffer 1 into buffer 2
		glUseProgram(matrixTransposeShader);
		BindSSBO(matrixTransposeShader, inBuffer, "InputBuf", 10);
		BindSSBO(matrixTransposeShader, tmpBuffer, "DataBuf", 11);
		glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Width"), MATRIX_WIDTH);
		glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Height"), MATRIX_HEIGHT);
		glDispatchCompute(MATRIX_WIDTH / TRANSPOSE_BLOCK_SIZE, MATRIX_HEIGHT / TRANSPOSE_BLOCK_SIZE, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		DownloadValues();
		//
		//Levels : 2, 4, 8, 16
		//LeveMasks : 2, 4, 8, 0
		// Sort the transposed column data
		glUseProgram(sortShader);
		BindSSBO(sortShader, tmpBuffer, "DataBuf", 10);
		glUniform1ui(glGetUniformLocation(sortShader, "_Level"), iteration1 / BITONIC_BLOCK_SIZE);
		glUniform1ui(glGetUniformLocation(sortShader, "_LevelMask"), (iteration1 & ~NUM_ELEMENTS)/BITONIC_BLOCK_SIZE);
		glDispatchCompute(NUM_ELEMENTS / BITONIC_BLOCK_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		DownloadValues();
		// Transpose the data from buffer 2 back into buffer 1
		glUseProgram(matrixTransposeShader);
		BindSSBO(matrixTransposeShader, tmpBuffer, "InputBuf", 10);
		BindSSBO(matrixTransposeShader, inBuffer, "DataBuf", 11);
		glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Width"), MATRIX_HEIGHT);
		glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Height"), MATRIX_WIDTH);
		glDispatchCompute(MATRIX_HEIGHT / TRANSPOSE_BLOCK_SIZE, MATRIX_WIDTH / TRANSPOSE_BLOCK_SIZE, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		DownloadValues();
		// Sort the row data
		//Levels : 512, 512, 512, 512
		//LeveMasks : 1024, 2048, 4096, 8192
		glUseProgram(sortShader);
		BindSSBO(sortShader, inBuffer, "DataBuf", 10);
		glUniform1ui(glGetUniformLocation(sortShader, "_Level"), BITONIC_BLOCK_SIZE);
		glUniform1ui(glGetUniformLocation(sortShader, "_LevelMask"), iteration1);
		glDispatchCompute(NUM_ELEMENTS / BITONIC_BLOCK_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		DownloadValues();		

		iteration1 <<= 1;
	}
	else
	{
		done = true;
		started=false;
	}
}


void BitonicSort::Sort()
{
	if(multiFrame)
	{
		started=true;
		iteration0=2;
		iteration1=BITONIC_BLOCK_SIZE << 1;
		done=false;
		frame=0;
	}
	else
	{
		uint32_t NUM_ELEMENTS = (uint32_t)bufferSize; //number of elements to sort
		uint32_t MATRIX_WIDTH = BITONIC_BLOCK_SIZE; //Block size
		uint32_t MATRIX_HEIGHT = (uint32_t)NUM_ELEMENTS / BITONIC_BLOCK_SIZE; //Height of the blocks

		//The matrix is an array of size 512x32
		//On each line is one ascending or descending sequence

		//Create bitonic sequences of size 1024 (512 ascending, 512 descending)
		for (uint32_t level = 2; level <= BITONIC_BLOCK_SIZE; level <<= 1) {
			glUseProgram(sortShader);
			BindSSBO(sortShader, inBuffer, "DataBuf", 10);
			glUniform1ui(glGetUniformLocation(sortShader, "_Level"), level);
			glUniform1ui(glGetUniformLocation(sortShader, "_LevelMask"), level);
			glDispatchCompute(NUM_ELEMENTS / BITONIC_BLOCK_SIZE, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			
			DownloadValues();
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

			DownloadValues();
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

			DownloadValues();
			// Transpose the data from buffer 2 back into buffer 1
			glUseProgram(matrixTransposeShader);
			BindSSBO(matrixTransposeShader, tmpBuffer, "InputBuf", 10);
			BindSSBO(matrixTransposeShader, inBuffer, "DataBuf", 11);
			glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Width"), MATRIX_HEIGHT);
			glUniform1ui(glGetUniformLocation(matrixTransposeShader, "_Height"), MATRIX_WIDTH);
			glDispatchCompute(MATRIX_HEIGHT / TRANSPOSE_BLOCK_SIZE, MATRIX_WIDTH / TRANSPOSE_BLOCK_SIZE, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			DownloadValues();
			// Sort the row data
			//Levels : 512, 512, 512, 512
			//LeveMasks : 1024, 2048, 4096, 8192
			glUseProgram(sortShader);
			BindSSBO(sortShader, inBuffer, "DataBuf", 10);
			glUniform1ui(glGetUniformLocation(sortShader, "_Level"), BITONIC_BLOCK_SIZE);
			glUniform1ui(glGetUniformLocation(sortShader, "_LevelMask"), level);
			glDispatchCompute(NUM_ELEMENTS / BITONIC_BLOCK_SIZE, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			DownloadValues();
		}
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
}

void BitonicSort::RightClickUp() {

}

void BitonicSort::Scroll(float offset) {}

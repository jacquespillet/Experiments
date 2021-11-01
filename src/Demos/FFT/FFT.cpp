#include "FFT.hpp"

#include "GL/glew.h"
#include <glm/gtx/quaternion.hpp>

#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"
#include "imgui_plot.h"

void FFT::DFT1D(const std::vector<complex>& in, std::vector<complex>& out) 
{
	int N = (int) in.size(); 
	for (int k = 0; k < N; ++k) { 
		out[k] = 0; 
		for (int n = 0; n < N; ++n) { 
			double w = 2 * PI * n * k / N; 
			out[k] += in[n] * complex(cos(w), -sin(w)); 
		} 
	} 
} 

void FFT::iDFT1D(std::vector<complex>& in, std::vector<complex>& out) 
{
	int N = (int) in.size(); 
	for (int n = 0; n < N; ++n) { 
		out[n] = 0; 
		for (int k = 0; k < N; ++k) { 
			double w = 2 * PI * n * k / N; 
			out[n] += in[k] * complex(cos(w), sin(w)); 
		} 
		out[n] /= N; 
	}
}

//F(k) = F_even(k) + F_odd(k) * W^k
//Because of the rotational properties of the complex exponential
//F(k+u) = F_even(k) - F_odd(k) * W^k
//So we only need to compute the frequencies until N/2 and retrieve the values for k > N/2
void FFT::FFT1D(const std::vector<complex>& in, std::vector<complex>& out)
{
    const size_t N = in.size();
    if (N <= 1) {
		out = in;
		return;
	}
 
	std::vector<complex> odd, even;
	
	for(int i=0; i<N; i++)
	{
		if(i%2==0) even.push_back(in[i]);
		else odd.push_back(in[i]);
	}
	std::vector<complex> oddResult(odd.size());
	std::vector<complex> evenResult(even.size());
	
	//This simply computes the DFT of the 2 arrays. Could be replaced with DFT() instead for example
	FFT1D(even, evenResult);
    FFT1D(odd, oddResult);

	
	for (size_t k = 0; k < N/2; ++k)
    {
        complex w = std::polar(1.0, -2.0 * PI * (double)k / N) * oddResult[k];
        out[k    ] = evenResult[k] + w;
        out[k+N/2] = evenResult[k] - w;
    }
}

void FFT::iFFT1D(std::vector<complex>& in, std::vector<complex>& out)
{
	for(int i=0; i<in.size(); i++)
	{
		in[i] = std::conj(in[i]);
	}
 
    // forward fft
    FFT1D(in, out);
 
    // conjugate the complex numbers again
	for(int i=0; i<out.size(); i++)
	{
		out[i] = std::conj(out[i]);
		out[i]/=(float)out.size();
	}
}


void FFT::FFT2D(const std::vector<complex>& in, std::vector<complex>& out)
{
	std::vector<complex> temp(in.size());
	std::vector<complex> outLine(width);
	for(int i=0; i<height; i++)
	{
		int start = i * width;
		int end = start + width;
		std::vector<complex> line(in.begin() + start, in.begin() + end);
		FFT1D(line, outLine);
		for(int j=start, k=0; j<end; j++, k++)
		{
			temp[j] = outLine[k];
		}
	}

	std::vector<complex> outColumn(height);
	std::vector<complex> column(height);
	for(int i=0; i<width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int flatInx = j * width + i;
			column[j] = temp[flatInx];
		}
		
		FFT1D(column, outColumn);
		for(int j=0; j<height; j++)
		{
			int flatInx = j * width + i;
			out[flatInx] = outColumn[j];
		}
	}
}

void FFT::iFFT2D(std::vector<complex>& in, std::vector<complex>& out)
{
	std::vector<complex> temp(in.size());
	std::vector<complex> outLine(width);
	for(int i=0; i<height; i++)
	{
		int start = i * width;
		int end = start + width;
		std::vector<complex> line(in.begin() + start, in.begin() + end);
		iFFT1D(line, outLine);
		for(int j=start, k=0; j<end; j++, k++)
		{
			temp[j] = outLine[k];
		}
	}

	std::vector<complex> outColumn(height);
	std::vector<complex> column(height);
	for(int i=0; i<width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			int flatInx = j * width + i;
			column[j] = temp[flatInx];
		}
		
		iFFT1D(column, outColumn);
		for(int j=0; j<height; j++)
		{
			int flatInx = j * width + i;
			out[flatInx] = outColumn[j];
		}
	}
}



FFT::FFT() {
}

void FFT::InitBuffers1D()
{
	inputFloat1D = std::vector<float> (size1D);
	inputXAxis = std::vector<float> (size1D);
	input1D = std::vector<complex> (size1D);

	std::vector<float> frequencies(numFrequencies);
	for (int i = 0; i < numFrequencies; i++)
	{
		frequencies[i] = (float)std::rand() / (float)RAND_MAX;
		frequencies[i] *= 20;
		frequencies[i] = std::floor(frequencies[i]);
		std::cout << frequencies[i] << std::endl;
	}

	for(int i=0; i<input1D.size(); i++)
	{
		for (int j = 0; j < numFrequencies; j++)
		{
			float time = ((float)i / (float)input1D.size()) * (float)TWO_PI * frequencies[j];
			float cosValue = cos(time) / (float)numFrequencies;
			input1D[i] += complex(cosValue, 0);
			inputFloat1D[i] += cosValue;
		}
		float time = ((float)i / (float)input1D.size()) * (float)TWO_PI;
		inputXAxis[i] = time;
	}
}

void FFT::InitBuffers2D()
{
	std::string filename = "C:/Users/jacqu/OneDrive/Pictures/geoguessr/Capture.PNG";
	stbi_set_flip_vertically_on_load(true);  
	std::cout << "Reading image..." << std::endl;

	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nChannels, 0);
	int size = width * height;

	std::cout << "Allocating buffers..." << std::endl;
	input2D = std::vector<complex> (size);

	dftOutputFloat2D = std::vector<float> (size);
	dftOutput2D = std::vector<complex> (size);
	
	idftOutputFloat2D = std::vector<float> (size);
	idftOutput2D = std::vector<complex> (size);

	for(int i=0; i<input2D.size(); i++)
	{	
		uint32_t index = i * nChannels;
		int r = (int) data[index + 0];
		int g = (int) data[index + 1];
		int b = (int) data[index + 2];
		int gray = (r + g + b) / 3;
		float grayFloat = (float)gray / 255.0f;
		input2D[i] = complex(grayFloat, 0);
	}

	glGenTextures(1, &inputTexture);
	glBindTexture(GL_TEXTURE_2D, inputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glGenTextures(1, &frequencyTexture);
	glBindTexture(GL_TEXTURE_2D, frequencyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	glGenTextures(1, &outputTexture);
	glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    
	glBindTexture(GL_TEXTURE_2D, 0);
}



void FFT::Load() {
    InitBuffers1D();
    InitBuffers2D();
}

void FFT::RenderGUI() {
	bool open = false;
    ImGui::Begin("Parameters : ", &open);
	ImGui::SetWindowSize(ImVec2((float)800, (float)600));
	
    ImGui::Checkbox("2D", &mode);
	if(!mode) //1D
	{
		if(ImGui::Button("Regenerate"))
		{
			InitBuffers1D();
		}
		if(ImGui::Button("FFT"))
		{
			DoFFT1D();
		}
		if(ImGui::Button("IFFT"))
		{
			DoIFFT1D();
		}
		if(ImGui::Button("DFT"))
		{
			DoDFT1D();
		}
		if(ImGui::Button("IDFT"))
		{
			DoIDFT1D();
		}
		ImGui::PlotConfig conf;
		conf.values.ys = inputFloat1D.data();
		conf.values.count = size1D;
		conf.scale.min = -1;
		conf.scale.max = 1;
		conf.tooltip.show = true;
		conf.tooltip.format = "x=%.2f, y=%.2f";
		conf.grid_y.show = true;
		conf.frame_size = ImVec2((float)windowWidth, 300);
		conf.line_thickness = 2.f;
		ImGui::Plot("plot", conf);

		if(dftOutputFloat1D.size() > 0)
		{
			ImGui::PlotConfig confDftOut;
			// confDftOut.values.xs = inputXAxis.data();
			confDftOut.values.ys = dftOutputFloat1D.data();
			confDftOut.values.count = (int)dftOutputFloat1D.size()/2;
			confDftOut.scale.min = -1;
			confDftOut.scale.max = 1;
			confDftOut.tooltip.show = true;
			confDftOut.tooltip.format = "x=%.2f, y=%.2f";
			// confDftOut.grid_x.show = true;
			confDftOut.grid_y.show = true;
			confDftOut.frame_size = ImVec2((float)windowWidth, 300);
			confDftOut.line_thickness = 2.f;
			ImGui::Plot("plot", confDftOut);
		}
		if(idftOutputFloat1D.size() > 0)
		{
			ImGui::PlotConfig confIdftOut;
			// confIdftOut.values.xs = inputXAxis.data();
			confIdftOut.values.ys = idftOutputFloat1D.data();
			confIdftOut.values.count = (int)idftOutputFloat1D.size();
			confIdftOut.scale.min = -1;
			confIdftOut.scale.max = 1;
			confIdftOut.tooltip.show = true;
			confIdftOut.tooltip.format = "x=%.2f, y=%.2f";
			// confIdftOut.grid_x.show = true;
			confIdftOut.grid_y.show = true;
			confIdftOut.frame_size = ImVec2((float)windowWidth, 300);
			confIdftOut.line_thickness = 2.f;
			ImGui::Plot("plot", confIdftOut);
		}
	}
	else
	{
		if(ImGui::Button("FFT")){
			DoFFT2D();
		} 
		if(ImGui::Button("IFFT")){
			DoIFFT2D();
		} 
		ImGui::Image((void*)(intptr_t)inputTexture,  ImVec2(256,256));
		ImGui::Image((void*)(intptr_t)frequencyTexture,  ImVec2(256,256));
		ImGui::Image((void*)(intptr_t)outputTexture,  ImVec2(256,256));
	}

    ImGui::End();
}

void FFT::DoFFT1D()
{

	dftOutputFloat1D = std::vector<float> (size1D);
	dftOutput1D = std::vector<complex> (size1D);

	FFT1D(input1D, dftOutput1D);
	for(int i=0; i<input1D.size()/2; i++)
	{
		dftOutputFloat1D[i] = ((float)std::abs(dftOutput1D[i]) * 2) / (float)size1D;
	}
}


void FFT::DoIFFT1D()
{		
	idftOutputFloat1D = std::vector<float> (size1D);
	idftOutput1D = std::vector<complex> (size1D);

	iFFT1D(dftOutput1D, idftOutput1D);

	for(int i=0; i<input1D.size(); i++)
	{
		idftOutputFloat1D[i] = (float)idftOutput1D[i].real();
	}
}

void FFT::DoDFT1D()
{
	dftOutputFloat1D = std::vector<float> (size1D);
	dftOutput1D = std::vector<complex> (size1D);
	
	DFT1D(input1D, dftOutput1D);

	for(int i=0; i<input1D.size()/2; i++)
	{
		dftOutputFloat1D[i] = ((float)std::abs(dftOutput1D[i]) * 2) / (float)size1D;
	}
}


void FFT::DoIDFT1D()
{
	idftOutputFloat1D = std::vector<float> (size1D);
	idftOutput1D = std::vector<complex> (size1D);
	
	iDFT1D(dftOutput1D, idftOutput1D);

	for(int i=0; i<input1D.size(); i++)
	{
		idftOutputFloat1D[i] = (float)idftOutput1D[i].real();
	}
}

void FFT::DoFFT2D()
{
	std::cout << "Doing FFT..." << std::endl;
	FFT2D(input2D, dftOutput2D);

	int halfWidth = width/2;
	int halfHeight = height/2;
	for(int i=0; i<dftOutput2D.size(); i++)
	{

		int x = i % width;
		int y = i / width;
		int targetX = x, targetY=y, targetPixel;
		if(x < halfWidth) targetX = halfWidth - x - 1;
		else targetX = std::max(0, std::min(width- 1, (width - x) + halfWidth));
		
		if(y < halfHeight) targetY = halfHeight - y - 1;
		else targetY = std::max(0, std::min(height-1, (height - y) + halfHeight));

		targetPixel = targetY * width + targetX;
		dftOutputFloat2D[targetPixel] = ((float)std::abs(dftOutput2D[i])) / (float)width;
	}


	
	glBindTexture(GL_TEXTURE_2D, frequencyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, dftOutputFloat2D.data());
    
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FFT::DoIFFT2D()
{
	iFFT2D(dftOutput2D, idftOutput2D);
	
	for(int i=0; i<input2D.size(); i++)
	{
		idftOutputFloat2D[i] = (float)idftOutput2D[i].real();
	}

	glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, idftOutputFloat2D.data());
    
	glBindTexture(GL_TEXTURE_2D, 0);
}



void FFT::Render() {
	// Sort();
}

void FFT::Unload() {
    glDeleteTextures(1, &frequencyTexture);
    glDeleteTextures(1, &inputTexture);
    glDeleteTextures(1, &outputTexture);	
}


void FFT::MouseMove(float x, float y) {}

void FFT::LeftClickDown() {
	// Reset();
	// showValuesOnConsole();
}

void FFT::LeftClickUp() {
	
}

void FFT::RightClickDown() {
}

void FFT::RightClickUp() {

}

void FFT::Scroll(float offset) {}

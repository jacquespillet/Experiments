
#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"
#include <complex>

typedef std::complex<double> complex;
class FFT : public Demo {
public : 
    FFT();
    void Load();
    void Render();
    void RenderGUI();
    void Unload();

    void MouseMove(float x, float y);
    void LeftClickDown();
    void LeftClickUp();
    void RightClickDown();
    void RightClickUp();
    void Scroll(float offset);

    void FFT1D(const std::vector<complex>& in, std::vector<complex>& out);
    void iFFT1D(std::vector<complex>& in, std::vector<complex>& out);
    
    void FFT2D(const std::vector<complex>& in, std::vector<complex>& out);
    void iFFT2D(std::vector<complex>& in, std::vector<complex>& out);
    
    void DFT1D(const std::vector<complex>& in, std::vector<complex>& out);
    void iDFT1D(std::vector<complex>& in, std::vector<complex>& out);
    
    void FFTGPU1D(std::vector<complex>& in, std::vector<complex>& out);
    void IFFTGPU1D(std::vector<complex>& in, std::vector<complex>& out);
    
    void FFTGPU2D(std::vector<complex>& in, std::vector<complex>& out);
    void IFFTGPU2D(std::vector<complex>& in, std::vector<complex>& out);
    
    void DoDFT1D();
    void DoIDFT1D();
    
    void DoFFT1D();
    void DoIFFT1D();

    void DoFFT2D();
    void DoIFFT2D();

    void FilterFFT2D();

    //false = 1d, true =  2d
    bool mode = false;

private:
    clock_t t;
    float deltaTime;
    float elapsedTime;

    

    //1D
    std::vector<complex> input1D;
	std::vector<complex> idftOutput1D;
	std::vector<complex> dftOutput1D;
	std::vector<float> inputXAxis;
	std::vector<float> outputXAxis;
	std::vector<float> inputFloat1D;
	std::vector<float> dftOutputFloat1D;
	std::vector<float> idftOutputFloat1D;
    int numFrequencies = 10;
    int size1D = 1024;
    
    //2D
    std::vector<complex> input2D;
	std::vector<complex> idftOutput2D;
	std::vector<complex> dftOutput2D;
	std::vector<float> inputFloat2D;
	std::vector<float> dftOutputFloat2D;
	std::vector<float> idftOutputFloat2D;
    int width, height, nChannels;
    GLuint frequencyTexture;
    GLuint inputTexture;
    GLuint outputTexture;
    float LowFilter = 1;
    float HighFilter = 0;

    // 
    std::vector<complex> odd, even;


	
        
    void InitBuffers2D();
    void InitBuffers1D();
};
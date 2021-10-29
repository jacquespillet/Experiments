
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
    
    int bufferSize = 1024;
    int resolution2D = 256;
    
    void DoDFT1D();
    
    void DoFFT();
    void DoFFT2D();

    //1d / 2d
    bool mode = false;

private:
    clock_t t;
    float deltaTime;
    float elapsedTime;

    
    std::vector<complex> input;
	std::vector<complex> idftOutput;

    //1D
	std::vector<complex> dftOutput;
	std::vector<float> inputFloat;
	std::vector<float> inputXAxis;
	std::vector<float> dftOutputFloat;
	std::vector<float> idftOutputFloat;
    
    //2D
    int width, height, nChannels;
    GLuint frequencyTexture;
    GLuint inputTexture;
    GLuint outputTexture;

	
        
    void InitBuffers();
};
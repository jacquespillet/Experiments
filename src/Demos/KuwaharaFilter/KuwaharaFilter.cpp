#include "KuwaharaFilter.hpp"

#include <glad/gl.h>
#define GLM_ENABLE_EXPERIMENTAL
 
#include "GL_Helpers/Util.hpp"
#include <fstream>
#include <sstream>
#include <random>

#include "imgui.h"

KuwaharaFilter::KuwaharaFilter() {
}

void KuwaharaFilter::ReloadK0()
{
    std::vector<float> K0Values(32*32);
    std::vector<uint8_t> K0ValuesView(32*32 * 4);
    int size = 32;
    int N = 8;
    float sigmaR = 0.25f * (size-1);
    float sigmaS = smoothing * sigmaR;

    //Original function
    for(int y=0; y<size; y++)
    {
        for(int x=0; x<size; x++)
        {
            float xf = ((float)x / (float)size)-0.5f;
            float yf = ((float)y / (float)size)-0.5f;
            float arg = atan2(yf, xf);
            if(arg > -PI/N && arg <= PI/N)
            {
                K0Values[y * size + x] = 1;
            }
        }
    }

    
    //Convolution kernel
    float ss = 2.0f * sigmaS * sigmaS;
    float sum = 0.0;
    int halfSize = (int)ceil( 2.0 * sigmaS );
    int kernelSize= 2 * halfSize + 1;
    std::vector<std::vector<float>> convolutionKernel(kernelSize, std::vector<float>(kernelSize));
    for (int x = -halfSize; x <= halfSize-1; x++) {
        for (int y = -halfSize; y <= halfSize-1; y++) {
            float r = sqrt((float)(x * x + y * y));
            convolutionKernel[x + halfSize][y + halfSize] = (exp(-r*r / ss)) / (PI * ss);
            sum += convolutionKernel[x + halfSize][y + halfSize];
        }
    }

    // // normalising the convolutionKernel
    for (int i = 0; i < kernelSize; ++i)
        for (int j = 0; j < kernelSize; ++j)
            convolutionKernel[i][j] /= sum;


    //Convolve
    for(int y=0; y<size; y++)
    {
        for(int x=0; x<size; x++)
        {
            //Convolve with the gaussian kernel
            float value=0;
            for(int ky=-halfSize; ky< halfSize; ky++)
            {
                for(int kx=-halfSize; kx< halfSize; kx++)
                {
                    int sampleX = std::max(0, std::min(size-1, x + kx));
                    int sampleY = std::max(0, std::min(size-1, y + ky));
                    int sampleInx = sampleY * size + sampleX;
                    float w = convolutionKernel[kx + halfSize][ky + halfSize];
                    value += w * K0Values[sampleInx];
                }                
            }
            int inx = y * size + x;
            K0Values[inx] = value;
        }
    }

     if(doDecay) 
     {
         //Multiply with gaussian 
         float maxValue = 0.0;
         float sr = 2.0f * sigmaR * sigmaR;
         int halfSizeTexture = size/2;
         for (int j = 0; j < size; ++j) {
             for (int i = 0; i < size; ++i) {
                 float x = (float)(i - halfSizeTexture);
                 float y = (float)(j - halfSizeTexture);
                
                 float r = sqrt((float)(x * x + y * y));
                 K0Values[j * size + i] *= (exp(-r*r / sr)) / (PI * sr);
                //  K0Values[j * size + i] *= exp(-0.5f * r * r / sigmaR / sigmaR);

                 if (K0Values[j * size + i] > maxValue) maxValue = K0Values[j * size + i];
             }
         }

         for (int j = 0; j < size; ++j) {
             for (int i = 0; i < size; ++i) {
                 K0Values[j * size + i] /= maxValue;
             }
         }
     }

    //Set viewer texture values
    uint8_t *colorizeData = K0Colorizer.Data();
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i < size; ++i) {
            int value = (int)(K0Values[j * size + i] * 255);
            uint8_t r = colorizeData[value * K0Colorizer.nChannels + 0];
            uint8_t g = colorizeData[value * K0Colorizer.nChannels + 1];
            uint8_t b = colorizeData[value * K0Colorizer.nChannels + 2];
            K0ValuesView[j * size * 4 + i * 4 + 0] = r;
            K0ValuesView[j * size * 4 + i * 4 + 1] = g;
            K0ValuesView[j * size * 4 + i * 4 + 2] = b;
            K0ValuesView[j * size * 4 + i * 4 + 3] = 255;
        }
    }

	glBindTexture(GL_TEXTURE_2D, K0Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, k0Size, k0Size, 0, GL_RED, GL_FLOAT, K0Values.data());
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, K0TextureView);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, k0Size, k0Size, 0, GL_RGBA, GL_UNSIGNED_BYTE, K0ValuesView.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void KuwaharaFilter::Load() {

    MeshShader = GL_Shader("shaders/KuwaharaFilter/Filter.vert", "", "shaders/KuwaharaFilter/Filter.frag");
    
    TextureCreateInfo tci = {};
    tci.generateMipmaps =false;
    tci.srgb=true;
    tci.minFilter = GL_LINEAR;
    tci.magFilter = GL_LINEAR;
    texture = GL_Texture("resources/textures/KuwaharaFilter/image.jpg", tci);
    Quad = GetQuad();

    
    K0Colorizer = GL_Texture("resources/textures/KuwaharaFilter/color.png", tci);
	
    glGenTextures(1, &K0Texture);
	glBindTexture(GL_TEXTURE_2D, K0Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, k0Size, k0Size, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
    glGenTextures(1, &K0TextureView);
	glBindTexture(GL_TEXTURE_2D, K0TextureView);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, k0Size, k0Size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    ReloadK0();

}

void KuwaharaFilter::RenderGUI() {
    ImGui::Begin("Parameters : ");

    
    static const char* mode = modes[currentMode];
    if (ImGui::BeginCombo("##combo", mode)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < IM_ARRAYSIZE(modes); n++)
		{
			bool is_selected = (mode == modes[n]); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(modes[n], is_selected))
			{
				mode = modes[n];
				if(mode != nullptr) {
					currentMode = n;
				}
			}
			if (is_selected){
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
		}
		ImGui::EndCombo();
	}
    ImGui::SliderInt("Radius", &radius, 0, 20);
    
    if(currentMode == 2)
    {
        float lSmoothing = smoothing;
        ImGui::SliderFloat("Smoothing", &lSmoothing, 0.01f, 1.0f);
        if(lSmoothing != smoothing)
        {
            smoothing = lSmoothing;
            ReloadK0();
        }

        ImGui::SliderFloat("q", &q, 0.01f, 20);

        bool lDoDecay = doDecay;
        ImGui::Checkbox("Apply Decay K0", &lDoDecay);
        if(lDoDecay != doDecay)
        {
            doDecay = lDoDecay;
            ReloadK0();
        }
        ImGui::Image((void*)(intptr_t)K0TextureView, ImVec2(100, 100));
    }
    
    ImGui::End();
}

void KuwaharaFilter::Render() {
    
    glUseProgram(MeshShader.programShaderObject);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.glTex);
    glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "textureImage"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, K0Texture);
    glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "K0"), 1);
    
    glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "radius"), radius);
    glUniform1f(glGetUniformLocation(MeshShader.programShaderObject, "q"), q);
    glUniform1i(glGetUniformLocation(MeshShader.programShaderObject, "currentMode"), currentMode);
    Quad->RenderShader(MeshShader.programShaderObject);
}

void KuwaharaFilter::Unload() {
    Quad->Unload();
    delete Quad;
    MeshShader.Unload();

    texture.Unload();

    glDeleteTextures(1, &K0Texture);
    glDeleteTextures(1, &K0TextureView);
    K0Colorizer.Unload();
}


void KuwaharaFilter::MouseMove(float x, float y) {
}

void KuwaharaFilter::LeftClickDown() {
}

void KuwaharaFilter::LeftClickUp() {
}

void KuwaharaFilter::RightClickDown() {
}

void KuwaharaFilter::RightClickUp() {
}

void KuwaharaFilter::Scroll(float offset) {
}

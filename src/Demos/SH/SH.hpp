#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class SH : public Demo {
public : 
    SH();
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

private:
    clock_t t;
    float deltaTime;
    float elapsedTime;
        
    GL_Camera cam;

    double P(int l,int m,double x) 
    { 
        // evaluate an Associated Legendre Polynomial P(l,m,x) at x 
        double pmm = 1.0; 
        if(m>0) {  
            double somx2 = sqrt((1.0-x)*(1.0+x));  
            double fact = 1.0;  
            for(int i=1; i<=m; i++) {  
                pmm *= (-fact) * somx2;  
                fact += 2.0;  
            }  
        }
        if(l==m) return pmm; 
        double pmmp1 = x * (2.0*m+1.0) * pmm; 
        if(l==m+1) return pmmp1; 
        double pll = 0.0; 
        for(int ll=m+2; ll<=l; ++ll) { 
            pll = ( (2.0*ll-1.0)*x*pmmp1-(ll+m-1.0)*pmm ) / (ll-m); 
            pmm = pmmp1; 
            pmmp1 = pll; 
        }  
        return pll; 
    }
    int factorial(int input)
    {
        int Result = 1;
        for(int i=1; i<=input; i++)
        {
            Result *= i;
        }
        return Result;
    }
    
    // double PMM(int m,double x) 
    // {
    //     double Result = 0;
    //     Result = std::pow(-1.0, m) * factorial(factorial((2 * m - 1)) * std::pow((1 - x * x), m/2);
    //     return Result;
    // }


    double K(int l, int m) 
    { 
        // renormalisation constant for SH function 
		double num = ((2.0*l + 1.0)*factorial(l - m));
		double denum = (4.0*PI*factorial(l + m));
		double temp = num / denum; 

        return sqrt(temp);  
    } 
    
    double SphericalHarmonics(int l, int m, double theta, double phi) 
    { 
        // return a point sample of a Spherical Harmonic basis function 
        // l is the band, range [0..N] 
        // m in the range [-l..l] 
        // theta in the range [0..Pi] 
        // phi in the range [0..2*Pi] 
        const double sqrt2 = sqrt(2.0); 
        if(m==0) return K(l,0)*P(l,m,cos(theta));  
		else if (m > 0) {
			double Kv = K(l, m);
			double cosv = cos(m*phi);
			double polynom = P(l, m, cos(theta));
			return sqrt2 * Kv *cosv * polynom;
		}
        else return sqrt2*K(l,-m)*sin(-m*phi)*P(l,-m,cos(theta));  
    }

    GLuint pointsVAO, pointsVBO;
    GL_Shader pointsShader;

    struct point 
    {
        glm::vec3 position;
        uint8_t r, g, b, a;
    };
    std::vector<point> points;

    int resolutionX = 1024;
    int resolutionY = 1024;
    int lValue=1;
    int mValue=1;

    void InitBuffers();
    void Recompute();
};
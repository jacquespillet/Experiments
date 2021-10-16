#pragma once

#include "Common.h"

class GL_Shader
{		
	private:
	public:
		bool compiled; 
		//shader source	
		std::string vertSrc;
		std::string fragSrc;
		std::string geometrySrc;
	
		//shader programs
		int programShaderObject;
		int vertexShaderObject;
		int geometryShaderObject;
		int fragmentShaderObject;

		GL_Shader();
		GL_Shader(std::string vertFilename, std::string geoFilename, std::string fragFilename);
		void Unload();
		void Compile();
		
		void SetMat4(std::string varName, glm::mat4 mat, bool doBind=false);
		void SetInt(std::string varName, int val, bool doBind=false);		
		void SetTexture2D(std::string varName, unsigned int tex, unsigned int texSlot, bool doBind=false);
		void SetTexture3D(std::string varName, unsigned int tex, unsigned int texSlot, bool doBind=false);
		void SetVector3(std::string varName, glm::vec3 v, bool doBind=false);
		void SetFloat(std::string varName, float v, bool doBind=false);

};
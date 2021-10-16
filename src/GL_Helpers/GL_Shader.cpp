#include "GL_Shader.hpp"
	
#include <fstream>
#include <sstream>
#include <GL/glew.h>
GL_Shader::GL_Shader()
{
	compiled = false; 
}
GL_Shader::GL_Shader(std::string vertFilename, std::string geoFilename, std::string fragFilename) {
    
	std::ifstream t(vertFilename);
    std::stringstream vertBuffer;
    vertBuffer << t.rdbuf();
    vertSrc= vertBuffer.str();

    t = std::ifstream (geoFilename);
	if(t.is_open())
	{
		std::stringstream geomBuffer;
		geomBuffer << t.rdbuf();
		geometrySrc= geomBuffer.str();
	}

    t = std::ifstream (fragFilename);
    std::stringstream fragBuffer;
    fragBuffer << t.rdbuf();
    fragSrc= fragBuffer.str();

    t.close();

	Compile();	
}
	
void GL_Shader::Compile()
{
	bool hasGeometry=geometrySrc.size()>0;

	//make array to pointer for source code (needed for opengl )
	const char* vsrc[1];
	const char* fsrc[1];
	const char* gsrc[1];
	vsrc[0] = vertSrc.c_str();
	fsrc[0] = fragSrc.c_str();
	if(hasGeometry)gsrc[0] = geometrySrc.c_str();
	
	//compile vertex and fragment shaders from source
	vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderObject, 1, vsrc, NULL);
	glCompileShader(vertexShaderObject);
	fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderObject, 1, fsrc, NULL);
	glCompileShader(fragmentShaderObject);
	if(hasGeometry) {
		geometryShaderObject = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShaderObject, 1, gsrc, NULL);
		glCompileShader(geometryShaderObject); 
	}
	std::cout << "Shader:Compile: Linking shader" << std::endl;
	
	//link vertex and fragment shader to create shader program object
	programShaderObject = glCreateProgram();
	glAttachShader(programShaderObject, vertexShaderObject);
	glAttachShader(programShaderObject, fragmentShaderObject);
	if(hasGeometry) glAttachShader(programShaderObject, geometryShaderObject);
	glLinkProgram(programShaderObject);
	std::cout << "Shader:Compile: checking shader status" << std::endl;
	
	//Check status of shader and log any compile time errors
	int linkStatus;
	glGetProgramiv(programShaderObject, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) 
	{
		char log[5000];
		int lglen; 
		glGetProgramInfoLog(programShaderObject, 5000, &lglen, log);
		std::cerr << "Shader:Compile: Could not link program: " << std::endl;
		std::cerr << log << std::endl;
		glGetShaderInfoLog(vertexShaderObject, 5000, &lglen, log);
		std::cerr << "vertex shader log:\n" << log << std::endl;
		glGetShaderInfoLog(fragmentShaderObject, 5000, &lglen, log);
		std::cerr << "fragment shader log:\n" << log << std::endl;
		glDeleteProgram(programShaderObject);
		programShaderObject = 0;
	}
	else
	{
		std::cout << "Shader:Compile: compile success " << std::endl;
		compiled = true; 
	}
}


void GL_Shader::SetMat4(std::string varName, glm::mat4 mat, bool doBind) {
    if(doBind) 	glUseProgram(programShaderObject);
	int location = glGetUniformLocation(programShaderObject, varName.c_str()); 
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(mat));
}

void GL_Shader::SetInt(std::string varName, int val, bool doBind){
    if(doBind) 	glUseProgram(programShaderObject);
    int location = glGetUniformLocation(programShaderObject, varName.c_str()); 
    glUniform1i(location,val);
}

void GL_Shader::SetTexture3D(std::string varName, unsigned int tex, unsigned int texSlot,bool doBind) {
    if(doBind) 	glUseProgram(programShaderObject);
    int location = glGetUniformLocation(programShaderObject, varName.c_str()); 
	glActiveTexture(GL_TEXTURE0 + texSlot);
	glBindTexture(GL_TEXTURE_3D, tex);
	glUniform1i(location,texSlot);
}

void GL_Shader::SetTexture2D(std::string varName, unsigned int tex, unsigned int texSlot,bool doBind) {
    if(doBind) 	glUseProgram(programShaderObject);
    int location = glGetUniformLocation(programShaderObject, varName.c_str()); 
	glActiveTexture(GL_TEXTURE0 + texSlot);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(location,texSlot);
}


void GL_Shader::Unload() {
	glDeleteShader(vertexShaderObject);
	glDeleteShader(fragmentShaderObject);
	glDeleteShader(geometryShaderObject);
	glDeleteProgram(programShaderObject);
}

void GL_Shader::SetVector3(std::string varName, glm::vec3 v, bool doBind) {
    if(doBind) 	glUseProgram(programShaderObject);
    int location = glGetUniformLocation(programShaderObject, varName.c_str()); 
    glUniform3fv(location,1, glm::value_ptr(v));
}

void GL_Shader::SetFloat(std::string varName, float v, bool doBind) {
    if(doBind) 	glUseProgram(programShaderObject);
    int location = glGetUniformLocation(programShaderObject, varName.c_str()); 
    glUniform1f(location, v);
}
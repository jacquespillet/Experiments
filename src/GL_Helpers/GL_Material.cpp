#include "GL_Material.hpp"
#include <glm/ext.hpp>
#include <fstream>
#include <sstream>

void GL_Material::SetMat4(std::string varName, glm::mat4 mat) {
    int location = glGetUniformLocation(shader, varName.c_str()); 
    glUniformMatrix4fv(location, 1, false, glm::value_ptr(mat));
}

void GL_Material::SetInt(std::string varName, int val){
    int location = glGetUniformLocation(shader, varName.c_str()); 
    glUniform1i(location,val);
}

void GL_Material::SetFloat(std::string varName, float val){
    int location = glGetUniformLocation(shader, varName.c_str()); 
    glUniform1f(location,val);
}

void GL_Material::SetVec4(std::string varName, glm::vec4 val){
    int location = glGetUniformLocation(shader, varName.c_str()); 
    glUniform4fv(location, 1, glm::value_ptr(val));
}

void GL_Material::SetVec3(std::string varName, glm::vec3 val){
    int location = glGetUniformLocation(shader, varName.c_str()); 
    glUniform3fv(location, 1, glm::value_ptr(val));
}

void GL_Material::SetTexture(std::string varName, const GL_Texture& texture, int numBind)
{
	glActiveTexture(GL_TEXTURE0 + numBind);
	glBindTexture(GL_TEXTURE_2D, texture.glTex);
	glUniform1i(glGetUniformLocation(shader, varName.c_str()), numBind);
}
#include "GL_Mesh.hpp"

#include <fstream>
#include <sstream>
#include <GL/glew.h>

GL_Mesh::GL_Mesh(): inited(false) {
}

GL_Mesh::GL_Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> triangles) : position(glm::vec3(0)), scale(glm::vec3(1)),rotation(glm::vec3(0)),   vertices(vertices), triangles(triangles), inited(false) {
    glGenVertexArrays(1, &vertexArrayObject);
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &elementBuffer);
    
    //Bind VAO
    glBindVertexArray(vertexArrayObject);
    
    //bind buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    //set vertex attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(GL_Mesh::Vertex), (void*)((uintptr_t)0));
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(GL_Mesh::Vertex), (void*)((uintptr_t)12));
    glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(GL_Mesh::Vertex), (void*)((uintptr_t)24));
    glVertexAttribPointer(3, 3, GL_FLOAT, true,  sizeof(GL_Mesh::Vertex), (void*)((uintptr_t)36));
    glVertexAttribPointer(4, 2, GL_FLOAT, true,  sizeof(GL_Mesh::Vertex), (void*)((uintptr_t)48));

    // //copy data to buffers
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_Mesh::Vertex), (uint8_t*)&vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(unsigned int), (uint8_t*)&triangles[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);        //Unbind VAO
    glBindVertexArray(0);
    //Unbind array and element buffers
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 

    RecalculateBoundingBox();
}

void GL_Mesh::Render(const GL_Camera& camera, GLuint shader) {
    if(!visible) return; 
    
	material->BindShader(shader);
    material->SetShaderUniforms();
    
    glm::mat4 modelViewProjectionMatrix = camera.GetProjectionMatrix() * camera.GetViewMatrix() * modelMatrix;
    material->SetMat4("modelViewProjectionMatrix", modelViewProjectionMatrix);
    material->SetMat4("modelMatrix", modelMatrix);
    //bind VAO	
	glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size(), GL_UNSIGNED_INT, (void*)0);
	//unbind VAO
	glBindVertexArray(0);    
	glUseProgram(0);    
    material->UnbindShader();
}

void GL_Mesh::RenderDepthOnly(const glm::mat4& viewProjection, GLuint shader)
{
    if(!visible) return;
	
	glUseProgram(shader);

	glm::mat4 modelViewProjectionMatrix = viewProjection * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(shader, "modelViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(modelViewProjectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size(), GL_UNSIGNED_INT, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);    
	glUseProgram(0);    
}

void GL_Mesh::RenderShader(GLuint shader)
{
    if(!visible) return;
	
	glUseProgram(shader);
    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size(), GL_UNSIGNED_INT, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);    
	glUseProgram(0);    
}

void GL_Mesh::RenderTo3DTexture(const glm::mat4& viewProjection, GLuint shader)
{
    if(!visible) return;
	
    material->BindShader(shader);
    material->SetShaderUniforms();
    

    // Matrix to transform to light position
    glm::mat4 depthModelViewProjectionMatrix = viewProjection * modelMatrix;
    
    glUniformMatrix4fv(glGetUniformLocation(shader, "DepthModelViewProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(depthModelViewProjectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    
    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glDrawElements(GL_TRIANGLES, (GLsizei)triangles.size(), GL_UNSIGNED_INT, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);    
	glUseProgram(0);    

    material->UnbindShader();
}


void GL_Mesh::Translate(glm::vec3 dPos) {
    position += dPos;
    RecalculateMatrices();
    RecalculateBoundingBox();
}

void GL_Mesh::Rotate(glm::vec3 dRot) {
    rotation += dRot;
    RecalculateMatrices();
    RecalculateBoundingBox();
}

void GL_Mesh::Scale(glm::vec3 dScale) {
    scale += dScale;
    RecalculateMatrices();
    RecalculateBoundingBox();
}


void GL_Mesh::SetPos(glm::vec3 pos) {
    position = pos;
    RecalculateMatrices();
    RecalculateBoundingBox();
}

void GL_Mesh::SetRot(glm::vec3 rot) {
    rotation = rot;
    RecalculateMatrices();
    RecalculateBoundingBox();
}

void GL_Mesh::SetScale(glm::vec3 _scale) {
    scale = _scale;
    RecalculateMatrices();
    RecalculateBoundingBox();
}

void GL_Mesh::RecalculateBoundingBox()
{
    bbMin = glm::vec3 (std::numeric_limits<float>::max());
    bbMax = glm::vec3 (-std::numeric_limits<float>::max());
    for(int i=0; i<vertices.size(); i++)
    {
        glm::vec3 worldVertex = glm::vec3(modelMatrix * glm::vec4(vertices[i].position, 1.0f));
        bbMin = glm::min(worldVertex, bbMin);
        bbMax = glm::max(worldVertex, bbMax);
    }
}

glm::vec3 GL_Mesh::GetMinBoundingBox()
{
    return bbMin;
}

glm::vec3 GL_Mesh::GetMaxBoundingBox()
{
    return bbMax;
}

void GL_Mesh::RecalculateMatrices() {
    glm::mat4 t=  glm::translate(glm::mat4(1.0), position);

    glm::mat4 rx=  glm::rotate(glm::mat4(1.0), glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
    glm::mat4 ry=  glm::rotate(glm::mat4(1.0), glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 rz=  glm::rotate(glm::mat4(1.0), glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
    glm::mat4 r = rz * ry * rx;

    glm::mat4 s = glm::scale(glm::mat4(1.0), scale);

    modelMatrix = t * s * r;
    invModelMatrix = glm::inverse(modelMatrix);
}

void GL_Mesh::UpdateGPUBuffers()
{
    glBindVertexArray(vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_Mesh::Vertex), (uint8_t*)&vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);        //Unbind VAO
    glBindVertexArray(0);
}

void GL_Mesh::Unload(){ 
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &elementBuffer);
    glDeleteVertexArrays(1, &vertexArrayObject);
    inited=false;
}
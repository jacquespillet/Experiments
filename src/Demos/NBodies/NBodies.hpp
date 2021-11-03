#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class NBodies : public Demo {
public : 
    NBodies();
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

    void Reset();
    void UpdateParticles();
    void ClearAccelerations();
private:
    clock_t t;
    float deltaTime;
    float elapsedTime;
        
    void InitBuffers();

    GL_Camera cam;

    GLint nbodiesShader;
    GLint clearAccelerationsShader;
    GLuint particleBuffer[2];
    
    int bufferSize = 32768;

    struct particle
    {
        glm::vec3 position;
        uint32_t color;
        glm::vec3 velocity;
        float mass;
        glm::vec3 acceleration;
        float invMass;
    };
    std::vector<particle> particles;

    GL_Shader pointsShader;
    GLuint pointsVAO, pointsVBO;

    GL_Texture texture;

    int oldPingPongInx=0;
    int pingPongInx=0;

    const char* modes[5] = { "Random In Cube","Random On Cube", "Random On Sphere", "Random In Sphere", "Galaxies"};
    const char* currentMode = modes[0];
    float softeningFactor = 0.3125f;
    float damping = 0.999f;
    float timestep = 0.005f;
    float maxMass = 1;
    float pointSize = 1;

};
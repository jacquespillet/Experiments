#pragma once
#include "../Demo.hpp"
#include "GL_Helpers/GL_Shader.hpp"
#include "GL_Helpers/GL_Camera.hpp"

#include "GL_Helpers/GL_Mesh.hpp"
#include "GL_Helpers/GL_Camera.hpp"

class PicFlipFluid : public Demo {
public : 
    PicFlipFluid();
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
    void StepSimulation();
private:
    clock_t t;
    float deltaTime;
    float elapsedTime;
        
    void InitShaders();
    void InitBuffers();
    
    GL_Camera cam;

    
    GL_Shader pointsShader;
    GLuint pointsVAO, pointsVBO;


    glm::vec3 mousePos, mouseVel, mouseOldPos;

    //Grid data
    glm::ivec3 gridDimensions{gridSize + 1, gridSize + 1, gridSize + 1};
    glm::ivec3 gridCellDimensions{gridSize, gridSize, gridSize};
    glm::vec3 boundsMin{-1, -1, -1};
    glm::vec3 boundsMax{1, 1, 1};
    glm::vec3 bounds_size = boundsMax - boundsMin;
    glm::vec3 cell_size = bounds_size / glm::vec3(gridCellDimensions);
    int numCells = gridDimensions.x * gridDimensions.y * gridDimensions.z;


    //SSBOs
    int numParticles;
    GLuint particlesBuffer;
    GLuint gridBuffer;
    GLuint transferBuffer;

    //Simulation compute shaders
    GLint resetGridShader;
    GLint transferAccumulationShader;
    GLint applyTransferToGridShader;
    GLint applyForcesShader;
    GLint gridToParticleShader;
    GLint advectParticlesShader;
    GLint gridProjectionShader;
    GLint jacobiIterationShader;
    GLint pressureGuessShader;
    GLint pressureUpdateShader;

    //
    GLuint framebuffer;
    GLuint depthTexture;
    GLuint depthTexture1;
    GLuint colorTexture;
    GL_Mesh screenspaceQuad;
    GL_Shader quadShader;

    GLint blurDepthShader;
    GLuint horizontalBlurTexture;
    GLuint verticalBlurTexture;
    void InitFramebuffer();

    //Grid helpers
    glm::vec3 GetWorldCoordinate(const glm::ivec3& gridCoord, const glm::ivec3& halfOffset = glm::ivec3(0, 0, 0)) {
        return boundsMin + glm::vec3(gridCoord) * cell_size + glm::vec3(halfOffset) * cell_size * 0.5f;
    }

    void SetGridUniforms(GLint shader);

    //Simulation functions
    void ResetGrid();
    void TransferVelocities();
    void ApplyForces();
    void GridToParticles();
    void AdvectParticles();
    void GridProjection();
    void PressureSolve();
    void PressureUpdate();

    //Structs :
    
    //TODO(Jacques) : Make that an enum
    const int GRID_AIR = 0;
    const int GRID_SOLID = 1;
    const int GRID_FLUID = 2;

    //TODO(Jacques): Get rid of alignas
    struct gridCell {
        glm::vec3 pos;
        int type;
        glm::vec3 vel; // velocity component at each of three faces on cell cube (not a real vector)
        float rhs = 0; // negative divergence for pressure solve

        glm::vec3 old_vel; // old velocity for FLIP update
        float padd0;
        
        // elements of A matrix in pressure solve
        float a_diag = 0;
        float a_x = 0;
        float a_y = 0;
        float a_z = 0;

        float pressure_guess = 0;
        float pressure = 0;
        int vel_unknown = 1;
        float padd1;

        gridCell(const glm::vec3& pos, const glm::vec3& vel, int type) : pos(pos), type(type), vel(vel), old_vel(vel) {}
    };

    struct particle {
        glm::vec4 color;
        glm::vec3 pos;
        float pad0;
        glm::vec3 vel;
        float pad1;

        particle(glm::vec3 pos, glm::vec3 vel, glm::vec4 color) : color(color), pos(pos), vel(vel) {}
    };


    class transfer {
        float u;
        float v;
        float w;
        float weight_u;
        float weight_v;
        float weight_w;
        bool is_fluid = false;
        float pad0;
    };

    float massFactor = 1.0f;

    int density = 8;
    int gridSize = 24;
    
    float mouseForce = 1.0f;
    float timestep = 0.02f;
    glm::vec3 gravity = glm::vec3(0, -9.81, 0);
    float picFlipBlend = 0.95f;
    int iters = 40;
    bool attract=false;
    enum RENDER_MODE
    {
        SPHERES=0,
        COLORED_SPHERES=1,
        LIT=2,
        NORMALS=3
    };
};
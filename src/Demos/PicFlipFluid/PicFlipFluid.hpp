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
        alignas(16) glm::vec3 pos;
        alignas(4)  int type;
        alignas(16) glm::vec3 vel; // velocity component at each of three faces on cell cube (not a real vector)
        alignas(4)  float rhs = 0; // negative divergence for pressure solve

        alignas(16) glm::vec3 old_vel; // old velocity for FLIP update

        // elements of A matrix in pressure solve
        alignas(4)  float a_diag = 0;
        alignas(4)  float a_x = 0;
        alignas(4)  float a_y = 0;
        alignas(4)  float a_z = 0;

        alignas(4)  float pressure_guess = 0;
        alignas(4)  float pressure = 0;
        alignas(4)  int vel_unknown = 1;

        gridCell(const glm::vec3& pos, const glm::vec3& vel, int type) : pos(pos), type(type), vel(vel), old_vel(vel) {}
    };

    struct particle {
        alignas(16) glm::vec4 color;
        alignas(16) glm::vec3 pos;
        alignas(16) glm::vec3 vel;

        particle(glm::vec3 pos, glm::vec3 vel, glm::vec4 color) : color(color), pos(pos), vel(vel) {}
    };


    class transfer {
        using byte4 = char[4]; // true type may differ in GLSL based on capabilities
        alignas(4) byte4 u;
        alignas(4) byte4 v;
        alignas(4) byte4 w;
        alignas(4) byte4 weight_u;
        alignas(4) byte4 weight_v;
        alignas(4) byte4 weight_w;
        alignas(4) bool is_fluid = false;
    };

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
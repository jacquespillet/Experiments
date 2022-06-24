#define STB_IMAGE_IMPLEMENTATION

#include "Common.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include "Demos/Demo.hpp"
#include "Demos/VXGI/VXGI.hpp"
#include "Demos/RSM/RSM.hpp"
#include "Demos/ISM/ISM.hpp"
#include "Demos/LPV/LPV.hpp"
#include "Demos/SH/SH.hpp"
#include "Demos/NBodies/NBodies.hpp"
#include "Demos/BitonicSort/BitonicSort.hpp"
#include "Demos/FFT/FFT.hpp"
#include "Demos/Ocean_FFT/Ocean_FFT.hpp"
#include "Demos/PicFlipFluid/PicFlipFluid.hpp"
#include "Demos/DeferredDecals/DeferredDecals.hpp"
#include "Demos/PathTracer/PathTracer.hpp"
#include "Demos/Template/Template.hpp"
#include "Demos/Boids/Boids.hpp"
#include "Demos/TiledRendering/TiledRendering.hpp"
#include "Demos/SSAO/SSAO.hpp"
#include "Demos/SSR/SSR.hpp"
#include "Demos/OIT/OIT.hpp"
#include "Demos/CSM/CSM.hpp"
#include "Demos/ShellFur/ShellFur.hpp"
#include "Demos/DispMapping/DispMapping.hpp"
#include "Demos/SVT/SVT.hpp"
#include "Demos/SSDO/SSDO.hpp"
#include "Demos/KuwaharaFilter/KuwaharaFilter.hpp"
#include "Demos/SSSS/SSSS.hpp"
#include "Demos/PostProcessing/PostProcessing.hpp"



struct DemoManager {
    Demo *demo;
    DemoManager(){
        demo=nullptr;
    }
    void ClearDemo() {
        if(demo!=nullptr) {
            demo->Unload();
            delete demo;
        }
    }

    void LoadDemo(int demoInx) {
        switch (demoInx)
        {
        case 1:
            demo = new VXGI;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 2:
            demo = new RSM;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 3:
            demo = new ISM;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 4:
            demo = new LPV;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 5:
            demo = new SH;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 6:
            demo = new BitonicSort;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 7:
            demo = new NBodies;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 8:
            demo = new FFT;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 9:
            demo = new Ocean_FFT;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 10:
            demo = new PicFlipFluid;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 11:
            demo = new DeferredDecals;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 12:
            demo = new PathTracer;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 13:
            demo = new Boids;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 14:
            demo = new TiledRendering;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 15:
            demo = new SSAO;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 16:
            demo = new SSR;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 17:
            demo = new OIT;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 18:
            demo = new CSM;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 19:
            demo = new ShellFur;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 20:
            demo = new DispMapping;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 21:
            demo = new SVT;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 22:
            demo = new SSDO;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 23:
            demo = new KuwaharaFilter;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 24:
            demo = new SSSS;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        case 25:
            demo = new PostProcessing;
            demo->windowWidth = windowWidth;
            demo->windowHeight = windowHeight;
            demo->Load();
            break;
        default:
            break;
        }
    }

    void MouseMove(float xpos, float ypos ) {
        if(demo!=nullptr) {
            demo->MouseMove(xpos, ypos);
        }
    }

    void MouseAction(int button, int action, int mods) {
        if(demo!=nullptr) {
            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
                demo->RightClickDown();
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
                demo->RightClickUp();
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
                demo->LeftClickDown();
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
                demo->LeftClickUp();
            }
        }
    }    

    void Scroll(float offset) {
        if(demo!=nullptr) {
            demo->Scroll(offset);
        }
    }

    void Render() {
        if(demo!=nullptr) {
            demo->Render();
        }
    }

    void RenderGUI()
    {
        if(demo!=nullptr) {
            demo->RenderGUI();
        }
    }

    int windowWidth, windowHeight;

};

static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}    


void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

struct App {
public:
    App(){}
    int InitWindow() {
        // Setup window
        glfwSetErrorCallback(ErrorCallback);
        if (!glfwInit())
            return 1;

        // GL 3.0 + GLSL 130
        glsl_version = "#version 430";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT , GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT , GLFW_FALSE);
        window = glfwCreateWindow(1280, 720, "Testing", NULL, NULL);
        if (window == NULL)
            return 1;
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        bool err = glewInit() != GLEW_OK;
        if (err)
        {
            fprintf(stderr, "Failed to initialize OpenGL loader!\n");
            return 1;
        }

        glfwSetCursorPosCallback(window, CursorPositionCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetScrollCallback(window, ScrollCallback);
        
        // glDisable(GL_CULL_FACE);
        
        // glEnable(GL_STENCIL_TEST);    
        // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
        // glStencilFunc(GL_ALWAYS, 1, 0xFF); // all fragments should update the stencil buffer
        // glStencilMask(0xFF); // enable writing to the stencil buffer
        
        
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        glEnable(GL_DEPTH_TEST);
        // glEnable(GL_MULTISAMPLE);
        
        
        return 0;
    }    
    void InitImGUI() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void Start() {
        demoManager={};
        ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);

        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            GLenum err;
            while((err = glGetError()) != GL_NO_ERROR)
            {
                if(err==GL_INVALID_ENUM) std::cout << "GL_INVALID_ENUM"<<std::endl;
                if(err==GL_INVALID_VALUE) std::cout << "GL_INVALID_VALUE"<<std::endl;
                if(err==GL_INVALID_OPERATION) std::cout << "GL_INVALID_OPERATION"<<std::endl;
                if(err==GL_STACK_OVERFLOW) std::cout << "GL_STACK_OVERFLOW"<<std::endl;
                if(err==GL_STACK_UNDERFLOW) std::cout << "GL_STACK_UNDERFLOW"<<std::endl;
                if(err==GL_OUT_OF_MEMORY) std::cout << "GL_OUT_OF_MEMORY"<<std::endl;
                if(err==GL_INVALID_FRAMEBUFFER_OPERATION) std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION"<<std::endl;
                if(err==GL_CONTEXT_LOST) std::cout << "GL_CONTEXT_LOST"<<std::endl;
                if(err==GL_TABLE_TOO_LARGE) std::cout << "GL_TABLE_TOO_LARGE1"<<std::endl;
            }
            
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            RenderGUI();
            

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
            

            demoManager.windowWidth = display_w; 
            demoManager.windowHeight = display_h; 
            glDepthMask(GL_TRUE);
	
			demoManager.Render();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            

            glfwSwapBuffers(window);
        }

        demoManager.ClearDemo();

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void MouseMove(float x, float y) {
        demoManager.MouseMove(x, y);
    }

    void MouseAction(int button, int action, int mods) {
        demoManager.MouseAction(button, action, mods);
    }

    void Scroll(float yOffset) {
        demoManager.Scroll(yOffset);
    }

 

    GLFWwindow* window;
private:
    const char* glsl_version;
    ImGuiIO io;
    DemoManager demoManager;


    void RenderGUI() {
        {
            ImGui::Begin("Demos : "); 
            
            if(ImGui::Button("Voxel cone tracing", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(1);
            }
            if(ImGui::Button("Reflective Shadow Maps", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(2);
            }
            if(ImGui::Button("Light Propagation Volumes", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(4);
            }
            if(ImGui::Button("Spherical Harmonics viewer", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(5);
            }
            if(ImGui::Button("Bitonic Sort", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(6);
            }
            if(ImGui::Button("N-Bodies", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(7);
            }
            if(ImGui::Button("Fast Fourier Transform", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(8);
            }
            if(ImGui::Button("Ocean FFT", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(9);
            }
            if(ImGui::Button("Pic-Flip fluid", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(10);
            }
            if(ImGui::Button("Deferred Decals", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(11);
            }
            if(ImGui::Button("Path Tracer", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(12);
            }
            if(ImGui::Button("Tiled Rendering", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(14);
            }
            if(ImGui::Button("Screen Space Ambient Occlusion", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(15);
            }
            if(ImGui::Button("Screen Space Reflections", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(16);
            }
            if(ImGui::Button("Order Independant Transparency", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(17);
            }
            if(ImGui::Button("Cascaded Shadow Mapping", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(18);
            }
            if(ImGui::Button("Shell Fur", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(19);
            }
            if(ImGui::Button("Sparse Virtual Texturing", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(21);
            }
            if(ImGui::Button("Screen Space Directional Occlusion", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(22);
            }
            if(ImGui::Button("Screen Space Subsurface Scattering", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(24);
            }
            if(ImGui::Button("Post Processing", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(25);
            }

            ImGui::Text("WIP"); 
            if(ImGui::Button("Boids", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(13);
            }
            if(ImGui::Button("Imperfect Shadow Maps", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(3);
            }
            if(ImGui::Button("Displacement mapping", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(20);
            }
            if(ImGui::Button("Kuwahara Filter", ImVec2(100, 20))) {
                demoManager.ClearDemo();
                demoManager.LoadDemo(23);
            }
            demoManager.RenderGUI();

            ImGui::End();
        }    
    }
};

App app;
void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    app.MouseMove((float)xpos, (float)ypos);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    app.MouseAction(button, action, mods);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    app.Scroll((float)yoffset);
}



int main(int, char**)
{
    app.InitWindow();
    app.InitImGUI();
    app.Start();    
    return 0;
}
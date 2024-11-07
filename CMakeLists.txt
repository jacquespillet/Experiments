project(Experiments)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

set(IMGUI_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends ${CMAKE_CURRENT_SOURCE_DIR}/vendor/implot)
set(IMGUI_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_opengl3.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/implot/implot.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/implot/implot_demo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/vendor/implot/implot_items.cpp
)
include_directories(${IMGUI_INC_DIR})


# Glfw
set(GLFW_INSTALL FALSE)
add_subdirectory(vendor/glfw)
include_directories(vendor/glfw/include)

# # Glad
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/cmake" glad_cmake)
glad_add_library(glad_gl_core_45 STATIC API gl:core=4.5)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/gladsources/glad_gl_core_45/include")
set(GLAD_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/gladsources/glad_gl_core_45/src/gl.c)

# GLM
set(GLM_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
include_directories(${GLM_INC_DIR})

# TinyGLTF
set(TINYGLTF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinygltf)
include_directories(${TINYGLTF_INC_DIR})

# STB
set(STB_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb)
include_directories(${STB_INC_DIR})

# FileDialog
set(FILEDIALOG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nfd/src/include)
set(FILEDIALOG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nfd/src/nfd_common.c)
set(FILEDIALOG_SOURCE ${FILEDIALOG_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nfd/src/nfd_win.cpp)
include_directories(${FILEDIALOG_INC_DIR})


# ASSIMP
set(ASSIMP_INSTALL_PDB OFF)
add_subdirectory(vendor/assimp)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/vendor/assimp/include")
include_directories(vendor/assimp/include)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/src)


include_directories("gpuRT/include")
link_directories("C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v11.8\\lib\\x64")

set (sourceFiles
        Main.cpp
        src/GL_Helpers/GL_Camera.cpp
        src/GL_Helpers/GL_Material.cpp
        src/GL_Helpers/GL_Mesh.cpp
        src/GL_Helpers/GL_Shader.cpp
        src/GL_Helpers/Util.cpp
        src/Demos/VXGI/VXGI.cpp src/Demos/RSM/RSM.cpp src/Demos/ISM/ISM.cpp src/Demos/LPV/LPV.cpp src/Demos/SH/SH.cpp src/Demos/BitonicSort/BitonicSort.cpp src/Demos/Template/Template.cpp
        src/Demos/NBodies/NBodies.cpp src/Demos/FFT/FFT.cpp  src/Demos/Ocean_FFT/Ocean_FFT.cpp  src/Demos/PicFlipFluid/PicFlipFluid.cpp 
        src/Demos/DeferredDecals/DeferredDecals.cpp src/Demos/PathTracer/PathTracer.cpp  src/Demos/SVGF/SVGF.cpp  src/Demos/Boids/Boids.cpp  src/Demos/TiledRendering/TiledRendering.cpp 
        src/Demos/SSAO/SSAO.cpp src/Demos/SSR/SSR.cpp  src/Demos/OIT/OIT.cpp  src/Demos/CSM/CSM.cpp  src/Demos/ShellFur/ShellFur.cpp   src/Demos/DispMapping/DispMapping.cpp 
        src/Demos/SVT/SVT.cpp src/Demos/SSDO/SSDO.cpp src/Demos/KuwaharaFilter/KuwaharaFilter.cpp  src/Demos/SSSS/SSSS.cpp src/Demos/PostProcessing/PostProcessing.cpp src/Demos/SVO/SVO.cpp src/Demos/MeshManipulation/MeshManipulation.cpp
        ${GLAD_SOURCE}
        ${IMGUI_SOURCE}
        ${FILEDIALOG_SOURCE}
)

add_executable(Experiments ${sourceFiles})
target_link_libraries(Experiments glfw opengl32 assimp)

install(TARGETS Experiments RUNTIME)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION bin/resources)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ DESTINATION bin/shaders)
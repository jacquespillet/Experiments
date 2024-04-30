project(Experiments)
set(CMAKE_CXX_STANDARD 17)

set(IMGUI_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui)
set(IMGUI_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_demo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_glfw.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_draw.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_widgets.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/ImGuizmo.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ext/dearimgui/imgui_impl_opengl3.cpp
                )
include_directories(${IMGUI_INC_DIR})


# Glfw
set(GLFW_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/build/install/include)
set(GLFW_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/build/install/lib)
include_directories(${GLFW_INC_DIR})
link_directories(${GLFW_LIB_DIR})

# # Glew
set(GLEW_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/include)
set(GLEW_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/lib/Release/x64)
set(GLEW_DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glew/bin/Release/x64)
include_directories(${GLEW_INC_DIR})
link_directories(${GLEW_LIB_DIR})

# GLM
set(GLM_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm)
include_directories(${GLM_INC_DIR})

# TinyGLTF
set(TINYGLTF_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/tinygltf)
include_directories(${TINYGLTF_INC_DIR})

# STB
set(STB_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/stb)
include_directories(${STB_INC_DIR})

# FileDialog
set(FILEDIALOG_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/fileDialog/include)
set(FILEDIALOG_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ext/fileDialog/src/nfd_common.c)
set(FILEDIALOG_SOURCE ${FILEDIALOG_SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/ext/fileDialog/src/nfd_win.cpp)
include_directories(${FILEDIALOG_INC_DIR})
link_directories(${FILEDIALOG_LIB_DIR})

# ASSIMP
set(ASSIMP_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/include)
set(ASSIMP_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/lib)
set(ASSIMP_DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/assimp/bin)
include_directories(${ASSIMP_INC_DIR})
link_directories(${ASSIMP_LIB_DIR})

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
        src/Demos/DeferredDecals/DeferredDecals.cpp src/Demos/PathTracer/PathTracer.cpp  src/Demos/Boids/Boids.cpp  src/Demos/TiledRendering/TiledRendering.cpp 
        src/Demos/SSAO/SSAO.cpp src/Demos/SSR/SSR.cpp  src/Demos/OIT/OIT.cpp  src/Demos/CSM/CSM.cpp  src/Demos/ShellFur/ShellFur.cpp   src/Demos/DispMapping/DispMapping.cpp 
        src/Demos/SVT/SVT.cpp src/Demos/SSDO/SSDO.cpp src/Demos/KuwaharaFilter/KuwaharaFilter.cpp  src/Demos/SSSS/SSSS.cpp src/Demos/PostProcessing/PostProcessing.cpp src/Demos/SVO/SVO.cpp src/Demos/MeshManipulation/MeshManipulation.cpp
        ${IMGUI_SOURCE}
        ${FILEDIALOG_SOURCE}
)

add_executable(Experiments ${sourceFiles})
target_link_libraries(Experiments glfw3 glew32 opengl32 assimp-vc143-mt)

install(TARGETS Experiments RUNTIME)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/ DESTINATION bin/resources)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ DESTINATION bin/shaders)
install(FILES ${GLEW_DLL_DIR}/glew32.dll DESTINATION bin)
install(FILES ${ASSIMP_DLL_DIR}/assimp-vc143-mt.dll DESTINATION bin)
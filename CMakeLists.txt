cmake_minimum_required(VERSION 3.0)
project(Phoenix)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# set the include dir structure
include_directories(${PROJECT_SOURCE_DIR})

# BVH
set(BVH_DIR "${PROJECT_SOURCE_DIR}/BVH")
file(GLOB BVH_SRC "${BVH_DIR}/*.cpp" "${BVH_DIR}/*.h")
source_group("BVH" FILES ${BVH_SRC})

# BVHLighting
set(BVHLIGHTING_DIR "${PROJECT_SOURCE_DIR}/BVHLighting")
file(GLOB BVHLIGHTING_SRC "${BVHLIGHTING_DIR}/*.cpp" "${BVHLIGHTING_DIR}/*.h")
source_group("BVHLighting" FILES ${BVHLIGHTING_SRC})

# Camera
set(CAMERA_DIR "${PROJECT_SOURCE_DIR}/Camera")
file(GLOB CAMERA_SRC "${CAMERA_DIR}/*.cpp" "${CAMERA_DIR}/*.h")
source_group("Camera" FILES ${CAMERA_SRC})

# D3D11
set(D3D11_DIR "${PROJECT_SOURCE_DIR}/D3D11")
file(GLOB D3D11_SRC "${D3D11_DIR}/*.cpp" "${D3D11_DIR}/*.h")
source_group("D3D11" FILES ${D3D11_SRC})

# D3D11LineRenderer
set(D3D11LINERENDERER_DIR "${PROJECT_SOURCE_DIR}/D3D11LineRenderer")
file(GLOB D3D11LINERENDERER_SRC "${D3D11LINERENDERER_DIR}/*.cpp" "${D3D11LINERENDERER_DIR}/*.h")
source_group("D3D11LineRenderer" FILES ${D3D11LINERENDERER_SRC})

# D3D11MeshRenderer
set(D3D11MESHRENDERER_DIR "${PROJECT_SOURCE_DIR}/D3D11MeshRenderer")
file(GLOB D3D11MESHRENDERER_SRC "${D3D11MESHRENDERER_DIR}/*.cpp" "${D3D11MESHRENDERER_DIR}/*.h")
source_group("D3D11MeshRenderer" FILES ${D3D11MESHRENDERER_SRC})

# Image
set(IMAGE_DIR "${PROJECT_SOURCE_DIR}/Image")
file(GLOB IMAGE_SRC "${IMAGE_DIR}/*.cpp" "${IMAGE_DIR}/*.h")
source_group("Image" FILES ${IMAGE_SRC})

# Material
set(MATERIAL_DIR "${PROJECT_SOURCE_DIR}/Material")
file(GLOB MATERIAL_SRC "${MATERIAL_DIR}/*.cpp" "${MATERIAL_DIR}/*.h")
source_group("Material" FILES ${MATERIAL_SRC})

# Scenario
set(SCENARIO_DIR "${PROJECT_SOURCE_DIR}/Scenario")
file(GLOB SCENARIO_SRC "${SCENARIO_DIR}/*.cpp" "${SCENARIO_DIR}/*.h")
source_group("Scenario" FILES ${SCENARIO_SRC})

# SharedGPU_CPU
set(SHARED_GPU_CPU_DIR "${PROJECT_SOURCE_DIR}/SharedGPU_CPU")
file(GLOB SHARED_GPU_CPU_SRC "${SHARED_GPU_CPU_DIR}/*.cpp" "${SHARED_GPU_CPU_DIR}/*.h")
source_group("SharedGPU_CPU" FILES ${SHARED_GPU_CPU_SRC})

# SkyIllumination
set(SKYILLUM_DIR "${PROJECT_SOURCE_DIR}/SkyIllumination")
file(GLOB SKYILLUM_SRC "${SKYILLUM_DIR}/*.cpp" "${SKYILLUM_DIR}/*.h")
source_group("SkyIllumination" FILES ${SKYILLUM_SRC})

# Utils
set(UTILS_DIR "${PROJECT_SOURCE_DIR}/Utils")
file(GLOB UTILS_SRC "${UTILS_DIR}/*.cpp" "${UTILS_DIR}/*.h")
source_group("Utils" FILES ${UTILS_SRC})

# Window
set(WINDOW_DIR "${PROJECT_SOURCE_DIR}/Window")
file(GLOB WINDOW_SRC "${WINDOW_DIR}/*.cpp" "${WINDOW_DIR}/*.h")
source_group("Window" FILES ${WINDOW_SRC})

# ThirdParty
set(THIRDPARTY_DIR "${PROJECT_SOURCE_DIR}/ThirdParty")
file(GLOB IMGUI_SRC "${THIRDPARTY_DIR}/imgui/*.cpp" "${THIRDPARTY_DIR}/imgui/*.h")
source_group("ThirdParty/imgui" FILES ${IMGUI_SRC})
file(GLOB JSON_SRC "${THIRDPARTY_DIR}/Json/*.hpp")
source_group("ThirdParty/Json" FILES ${JSON_SRC})
file(GLOB MINIZ_SRC "${THIRDPARTY_DIR}/miniz/*.cpp" "${THIRDPARTY_DIR}/miniz/*.h")
source_group("ThirdParty/miniz" FILES ${MINIZ_SRC})
file(GLOB SKY_SRC "${THIRDPARTY_DIR}/Sky/HosekWilkie/*.c" "${THIRDPARTY_DIR}/Sky/HosekWilkie/*.h")
source_group("ThirdParty/Sky/HosekWilkie" FILES ${SKY_SRC})
file(GLOB STBIMAGE_SRC "${THIRDPARTY_DIR}/stb_image/*.h")
source_group("ThirdParty/stb_image" FILES ${STBIMAGE_SRC})
file(GLOB TINYEXR_SRC "${THIRDPARTY_DIR}/tinyexr/*.h")
source_group("ThirdParty/tinyexr" FILES ${TINYEXR_SRC})
file(GLOB TINYOBJLOADER_SRC "${THIRDPARTY_DIR}/tinyobjloader/*.cc" "${THIRDPARTY_DIR}/tinyobjloader/*.h")
source_group("ThirdParty/tinyobjloader" FILES ${TINYOBJLOADER_SRC})
file(GLOB TINYXML2_SRC "${THIRDPARTY_DIR}/tinyxml2/*.cpp" "${THIRDPARTY_DIR}/tinyxml2/*.h")
source_group("ThirdParty/tinyxml2" FILES ${TINYXML2_SRC})

add_executable(${PROJECT_NAME} WIN32 ${BVH_SRC} ${BVHLIGHTING_SRC} ${CAMERA_SRC} 
${D3D11_SRC} ${D3D11LINERENDERER_SRC} ${D3D11MESHRENDERER_SRC} 
${IMAGE_SRC} ${MATERIAL_SRC} ${SCENARIO_SRC} ${SHARED_GPU_CPU_SRC} ${SKYILLUM_SRC} 
${UTILS_SRC} ${WINDOW_SRC} 
${IMGUI_SRC} ${JSON_SRC} ${MINIZ_SRC} ${SKY_SRC} ${STBIMAGE_SRC} ${TINYEXR_SRC} ${TINYOBJLOADER_SRC} ${TINYXML2_SRC}
main.cpp)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
# set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PUBLIC CPU_ENVIRONMENT)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    # for the binary directory (if exec directly)
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Assets ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/Assets
    # for the visual studio solution if debug
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Assets ${CMAKE_BINARY_DIR}/Assets
)


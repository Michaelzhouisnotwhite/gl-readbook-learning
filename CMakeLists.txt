cmake_minimum_required(VERSION 2.6)

project(vermilion9)
include(FetchContent)
FetchContent_Declare(
  toy
  GIT_REPOSITORY https://github.com/Michaelzhouisnotwhite/My-CXX-Small-Toys
  GIT_TAG main
)
FetchContent_MakeAvailable(toy)
execute_process(
  COMMAND ${CMAKE_COMMAND} -S${CMAKE_SOURCE_DIR}/lib/glfw-3.3.8 -B${CMAKE_BINARY_DIR}/lib/glfw_build
  -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/lib/glfw_install -D_GLFW_WIN32=1
)
execute_process(
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/lib/glfw_build --config Release
)
execute_process(
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/lib/glfw_build --target install --config Release
)

add_subdirectory(lib/glm)

file(GLOB imgui_srcs lib/imgui/*.cpp)
file(GLOB imgui_hdrs lib/imgui/*.h)
file(GLOB imgui_backend_srcs lib/imgui/backends/*_opengl3.cpp)
file(GLOB imgui_backend_hdrs lib/imgui/backends/*_opengl3.h)

file(GLOB imgui_backend_deps_srcs lib/imgui/backends/*_glfw.cpp)
file(GLOB imgui_backend_deps_hdrs lib/imgui/backends/*_glfw.h)
message("imgui_srcs: " ${imgui_srcs})
message("imgui_backend_srcs: " ${imgui_backend_srcs})
message("imgui_backend_deps_hdrs: " ${imgui_backend_deps_hdrs})
if (WIN32)
  file(GLOB imgui_win32 lib/imgui/backends/*_win32.cpp)
  list(APPEND ${imgui_backend_srcs} ${imgui_win32})
endif()
add_library(imgui STATIC ${imgui_srcs} ${imgui_hdrs} ${imgui_backend_srcs} ${imgui_backend_deps_srcs} ${imgui_backend_hdrs} ${imgui_backend_deps_hdrs})
target_include_directories(imgui PUBLIC lib/imgui lib/imgui/backends)

if(WIN32)
  # find_package(glfw3 REQUIRED PATHS ${CMAKE_BINARY_DIR}/lib/glfw_install/cmake NO_DEFAULT_PATH)
  # include(CheckCXXCompilerFlag)
  # message("glfw_DIR: " ${glfw3_DIR})
  # file(GLOB glfw3_cmake ${glfw3_DIR}/*.cmake)

  # foreach(cmake_file ${glfw3_cmake})
  # include(${cmake_file})
  # endforeach()
endif()

LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# Second, for multi-config builds (e.g. msvc)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/lib)
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

# set(CMAKE_DEBUG_POSTFIX "_d")
find_package(OpenGL REQUIRED)

if(WIN32)
  set(COMMON_LIBS vermilion ${OPENGL_LIBRARIES})
elseif(UNIX)
  set(COMMON_LIBS vermilion ${OPENGL_LIBRARIES} glfw ${GLFW_LIBRARIES} GL rt dl -L/usr/lib64 -lXrandr -lXrender -lXi -lXfixes -lXxf86vm -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp -lGLEW -lGL -lX11 -lGLU -lOpenGL -lglfw -lrt -lm -ldl)
else()
  set(COMMON_LIBS vermilion)
endif()

set(COMMON_HEADERS include/vgl.h include/vapp.h include/Shader.hpp)

set(COMMON_LIBS ${COMMON_LIBS} ${EXTRA_LIBS})

add_library(vermilion
  lib/gl3w.c
  lib/LoadShaders.cpp
  lib/targa.cpp
  lib/vdds.cpp
  lib/loadtexture.cpp
  lib/vermilion.cpp
  lib/vbm.cpp
)

set(RUN_DIR ${PROJECT_SOURCE_DIR}/bin)

set(EXAMPLES
  01-keypress
  01-triangles
  03-drawcommands
  03-instancing
  03-instancing2
  03-instancing3
  03-pointsprites
  03-primitiverestart
  03-xfb
  04-gouraud
  04-gouraud-float
  04-shadowmap
  06-cubemap
  06-load-texture
  06-mipfilters
  06-multitexture
  06-statictexture
  06-texturewrapping
  06-volumetexturing
  08-lightmodels
  09-simpletess
  09-teapot
  10-draw-xfb
  10-fur
  10-viewport-array
  11-doublewrite
  11-oit
  11-overdrawcount
  12-imageprocessing
  12-particlesimulator
  12-raytracer
  12-simplecompute
)

find_package(OpenMP)

if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

file(GLOB_RECURSE my_test_files CONFIGURE_DEPENDS "src/*/test*.cpp")
message("my test source file: " ${my_test_files})

foreach(test_file ${my_test_files})
  get_filename_component(test_file_name ${test_file} NAME_WLE)
  add_executable(${test_file_name} WIN32 ${test_file} ${COMMON_HEADERS})
  target_link_directories(${test_file_name} BEFORE PRIVATE ${CMAKE_BINARY_DIR}/lib/glfw_install/lib)
  target_link_libraries(${test_file_name} ${COMMON_LIBS} toy::generic MyShader glm::glm)
  target_link_libraries(${test_file_name} imgui)
  target_include_directories(${test_file_name} PRIVATE ${CMAKE_BINARY_DIR}/include)

  if(MSVC)
    configure_file(${PROJECT_SOURCE_DIR}/build/templates/vs2013.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}.vcxproj.user @ONLY)
  endif(MSVC)
endforeach()

foreach(EXAMPLE ${EXAMPLES})
  add_executable(${EXAMPLE} WIN32 src/${EXAMPLE}/${EXAMPLE}.cpp ${COMMON_HEADERS})

  # set_property(TARGET ${EXAMPLE} PROPERTY DEBUG_POSTFIX _d)
  if(WIN32)
    target_include_directories(${EXAMPLE} PRIVATE ${CMAKE_BINARY_DIR}/lib)
  endif()

  target_link_libraries(${EXAMPLE} ${COMMON_LIBS})
  target_link_directories(${EXAMPLE} BEFORE PRIVATE ${CMAKE_BINARY_DIR}/lib/glfw_install/lib)
  target_link_libraries(${EXAMPLE} glfw3 MyShader)

  if(MSVC)
    configure_file(${PROJECT_SOURCE_DIR}/build/templates/vs2013.vcxproj.user.in ${CMAKE_CURRENT_BINARY_DIR}/${EXAMPLE}.vcxproj.user @ONLY)
  endif(MSVC)
endforeach(EXAMPLE)

add_library(MyShader src/Shader.cpp)
target_link_libraries(MyShader PRIVATE ${COMMON_LIBS} glm::glm)
target_link_directories(MyShader PUBLIC ${CMAKE_BINARY_DIR}/lib/glfw_install/lib)
target_link_libraries(MyShader PRIVATE glfw3 toy::generic)
target_include_directories(MyShader PUBLIC include)

add_library(MyCamera src/Camera.cpp include/Camera.hpp)
target_link_libraries(MyCamera PRIVATE ${COMMON_LIBS} glm::glm)
target_link_directories(MyCamera PUBLIC ${CMAKE_BINARY_DIR}/lib/glfw_install/lib)
target_link_libraries(MyCamera PRIVATE glfw3 toy::generic)
target_include_directories(MyCamera PUBLIC include)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LINUX")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(include)

# include_directories(lib/glfw/include)
include_directories(${CMAKE_BINARY_DIR}/lib/glfw_install/include)

ADD_CUSTOM_TARGET(debug ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Debug ${PROJECT_SOURCE_DIR})
ADD_CUSTOM_TARGET(release ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE:STRING=Release ${PROJECT_SOURCE_DIR})

if(NOT EXISTS ${CMAKE_BINARY_DIR}/include)
  make_directory(${CMAKE_BINARY_DIR}/include)
endif()

file(GLOB asserts ${CMAKE_SOURCE_DIR}/bin/media/*)

set(path_macro_path ${CMAKE_BINARY_DIR}/include/path_macro.h)
file(WRITE ${path_macro_path} "")
file(APPEND ${path_macro_path} "#ifndef PATH_MACRO_PATH\n")
file(APPEND ${path_macro_path} "#define PATH_MACRO_PATH\n")

foreach(assert ${asserts})
  message("assert: ${assert}")
  get_filename_component(name ${assert} NAME)
  string(REPLACE "." "_" name_with_on_dot ${name})
  string(TOUPPER ${name_with_on_dot} name_macros)
  file(APPEND ${path_macro_path} "#define ${name_macros} \"${assert}\"\n")
  message("to macro: " "#define ${name_macros} \"${assert}\"\n")
endforeach()

file(APPEND ${path_macro_path} "#endif // PATH_MACRO_PATH\n")

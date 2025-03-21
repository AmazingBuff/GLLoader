cmake_minimum_required(VERSION 3.25)

project(
    GLLoader
    LANGUAGES CXX
)

option(BUILD_SHARED_LIBS "build shared library" OFF)
option(ENABLE_TEST "enable a simple test" ON)
option(OPENGL_ES "use opengl es as backend" ON)


include(GenerateExportHeader)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(
    ${PROJECT_NAME}
)

generate_export_header(
        ${PROJECT_NAME}
        EXPORT_MACRO_NAME ${PROJECT_NAME}_EXPORT
        EXPORT_FILE_NAME ${PROJECT_NAME}Export.h
)

file(GLOB SOURCE_FILES src/*.cpp)

if (APPLE OR OPENGL_ES)
    set(PLATFORM_FILES
        src/platform/EGLContext.cpp
        src/platform/Utils.cpp)
elseif (WIN32)
    set(PLATFORM_FILES
        src/platform/WGLContext.cpp
        src/platform/Utils.cpp)
endif ()
list(APPEND SOURCE_FILES ${PLATFORM_FILES})


target_sources(
    ${PROJECT_NAME}
    PRIVATE
    ${SOURCE_FILES}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ext
    include/
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_features(
    ${PROJECT_NAME}
    PRIVATE
    cxx_std_23
)

#install(TARGETS ${PROJECT_NAME} DESTINATION ${LIBRARY_INSTALL_DIR})
#install(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Export.h DESTINATION ${INCLUDE_INSTALL_DIR})
#install(DIRECTORY include/ DESTINATION ${INCLUDE_INSTALL_DIR})

if (OPENGL_ES)
    target_compile_definitions(
        ${PROJECT_NAME}
        PUBLIC
        GL_ES
    )
endif (OPENGL_ES)

if (APPLE)
    if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
        file(GLOB DLLS ${CMAKE_CURRENT_SOURCE_DIR}/lib/mac/arm64/*.dylib)
    else ()
        file(GLOB DLLS ${CMAKE_CURRENT_SOURCE_DIR}/lib/mac/x64/*.dylib)
    endif ()
elseif (WIN32)
    file(GLOB DLLS ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/*.dll)
endif ()


if (ENABLE_TEST)
add_subdirectory(test)
endif (ENABLE_TEST)
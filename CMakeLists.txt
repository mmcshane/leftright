cmake_minimum_required (VERSION 3.2.2)

set(LEFTRIGHT_VERSION_MAJOR 1)
set(LEFTRIGHT_VERSION_MINOR 1)
set(LEFTRIGHT_VERSION_PATCH 3)
set(LEFTRIGHT_VERSION ${LEFTRIGHT_VERSION_MAJOR}.${LEFTRIGHT_VERSION_MINOR}.${LEFTRIGHT_VERSION_PATCH})

project(leftright VERSION ${LEFTRIGHT_VERSION} LANGUAGES CXX)

add_library(leftright INTERFACE)
target_include_directories(leftright INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

include(CTest)
add_subdirectory(test)

include(FindDoxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(
        docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating doxygen project documentation in ${CMAKE_CURRENT_BINARY_DIR}." VERBATIM
    )
else()
    ADD_CUSTOM_TARGET(docs COMMENT "Doxygen not found. Please install doxygen to use this target." VERBATIM)
endif()

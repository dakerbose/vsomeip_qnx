cmake_minimum_required(VERSION 3.21)
project(vsomeip_ecu VERSION 0.1.0 LANGUAGES CXX)

set(SOURCES vsomeip_ecu.cpp)
# Library
set(CMAKE_SKIP_RPATH TRUE)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")
option(BUILD_SHARED_LIBS "Build shared libraries (.so) instead of static ones (.a)" OFF)
set(LIBNAME lib${PROJECT_NAME})

include_directories(/home/dakerbose/qnx800/target/qnx/usr/local/include)


add_library(${LIBNAME} STATIC
    ${PROJECT_SOURCE_DIR}/lib/libvsomeip_ecu.cpp)
target_include_directories(${LIBNAME} PUBLIC ${PROJECT_SOURCE_DIR}/lib)

# Test executable
set(EXE_SOURCES 
    ${PROJECT_SOURCE_DIR}/app/vsomeip_ecu.cpp)
add_executable(${PROJECT_NAME} ${EXE_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE lib/public) 

# Add vsomeip3 library path
link_directories(/home/dakerbose/qnx800/target/qnx/aarch64le/usr/local/lib)

# Link vsomeip3 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBNAME} ${Boost_LIBRARIES} vsomeip3 vsomeip3-cfg vsomeip3-e2e vsomeip3-sd)

# QNX_PROCESSOR is inherited from the QNX toolchain file
install(TARGETS ${LIBNAME} DESTINATION nto/${QNX_PROCESSOR}/lib)
install(TARGETS ${PROJECT_NAME} DESTINATION nto/${QNX_PROCESSOR}/bin)
cmake_minimum_required(VERSION 3.22)

project(database CXX)

set(CMAKE_BUILD_PARALLEL_LEVEL 0)

set(CMAKE_BINARY_DIR build)
set(CMAKE_SOURCE_DIR src)

#add_subdirectory(libs/matplotplusplus)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/release)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/debug)
endif()

set(FUEL "${CMAKE_SOURCE_DIR}/fuel.h" "${CMAKE_SOURCE_DIR}/fuel.cpp")
set(MAT "${CMAKE_SOURCE_DIR}/material.h" "${CMAKE_SOURCE_DIR}/material.cpp")
set(FLOW "${CMAKE_SOURCE_DIR}/flow.h" "${CMAKE_SOURCE_DIR}/flow.cpp")
set(BOUNDARY "${CMAKE_SOURCE_DIR}/boundary_cond.h" "${CMAKE_SOURCE_DIR}/boundary_cond.cpp")
set(DATABASE "${CMAKE_SOURCE_DIR}/data_base.h" "${CMAKE_SOURCE_DIR}/data_base.cpp")
set(COMMON "${CMAKE_SOURCE_DIR}/common.h" "${CMAKE_SOURCE_DIR}/common.cpp")
set(MAIN_BASE "${CMAKE_SOURCE_DIR}/main_base.cpp")
set(MAIN_BOUNDARY "${CMAKE_SOURCE_DIR}/main_boundary.cpp")
set(TESTS "${CMAKE_SOURCE_DIR}/tests.cpp" "${CMAKE_SOURCE_DIR}/test_mat.h" "${CMAKE_SOURCE_DIR}/test_inidata.h")
set(SOLVER "${CMAKE_SOURCE_DIR}/solver.h" "${CMAKE_SOURCE_DIR}/solver.cpp")
set(MESH "${CMAKE_SOURCE_DIR}/mesh.cpp" "${CMAKE_SOURCE_DIR}/mesh.h")
set(INI_DATA "${CMAKE_SOURCE_DIR}/ini_data.h")
set(LOG "${CMAKE_SOURCE_DIR}/logger.h")

# add_executable(database ${MAIN_BASE} ${JSON} ${FUEL} ${MAT} ${COMMON} ${DATABASE})
# add_executable(boundary ${MAIN_BOUNDARY} ${JSON} ${FUEL} ${MAT} ${COMMON} ${DATA_BASE} ${FLOW} ${BOUNDARY})
add_executable(tests ${COMMON} ${FUEL} ${MAT} ${TESTS} ${BOUNDARY} ${FLOW} ${INI_DATA} ${MESH} ${DATABASE} ${SOLVER} ${LOG})
target_link_libraries(tests PUBLIC matplot)
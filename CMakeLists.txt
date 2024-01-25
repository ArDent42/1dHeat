cmake_minimum_required(VERSION 3.22)

project(database CXX)

set(CMAKE_BUILD_TYPE Debug)

set(SOURCE_DIR src)
set(FUEL "${SOURCE_DIR}/fuel.h" "${SOURCE_DIR}/fuel.cpp")
set(MAT "${SOURCE_DIR}/material.h" "${SOURCE_DIR}/material.cpp")
set(FLOW "${SOURCE_DIR}/flow.h" "${SOURCE_DIR}/flow.cpp")
set(BOUNDARY "${SOURCE_DIR}/boundary_cond.h" "${SOURCE_DIR}/boundary_cond.cpp")
set(DATABASE "${SOURCE_DIR}/data_base.h" "${SOURCE_DIR}/data_base.cpp")
set(COMMON "${SOURCE_DIR}/common.h" "${SOURCE_DIR}/common.cpp")
set(MAIN_BASE "${SOURCE_DIR}/main_base.cpp")
set(MAIN_BOUNDARY "${SOURCE_DIR}/main_boundary.cpp")
set(TESTS "${SOURCE_DIR}/tests.cpp" "${SOURCE_DIR}/test_mat.h" "${SOURCE_DIR}/test_inidata.h")
set(SOLVER "${SOURCE_DIR}/solver.h" "${SOURCE_DIR}/solver.cpp")
set(MESH "${SOURCE_DIR}/mesh.cpp" "${SOURCE_DIR}/mesh.h")
set(INI_DATA "${SOURCE_DIR}/ini_data.h")
set(LOG "${SOURCE_DIR}/logger.h")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/debug)

add_executable(database ${MAIN_BASE} ${JSON} ${FUEL} ${MAT} ${COMMON} ${DATA_BASE})

# add_executable(boundary ${MAIN_BOUNDARY} ${JSON} ${FUEL} ${MAT} ${COMMON} ${DATA_BASE} ${FLOW} ${BOUNDARY})
add_executable(tests ${COMMON} ${FUEL} ${MAT} ${TESTS} ${BOUNDARY} ${FLOW} ${INI_DATA} ${MESH} ${DATABASE} ${SOLVER} ${LOG})
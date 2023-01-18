cmake_minimum_required(VERSION 3.0)

project(method1 LANGUAGES C)

FILE(GLOB c_sources *.c)


add_executable(method1 ${c_sources})

#add_compile_definitions(DEBUG) 
add_compile_options(-O2)

target_link_libraries(method1 z m)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "fadso")

install(TARGETS method1 RUNTIME DESTINATION bin)




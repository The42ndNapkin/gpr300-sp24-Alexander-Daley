file(
 GLOB_RECURSE ASSIGNMENT0_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT0_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this worksession0's asset folder to bin when it is built
add_custom_target(copyAssetsW1.5 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT0_INC} DESTINATION include/worksession1.5)
add_executable(worksession1.5 ${ASSIGNMENT0_SRC} ${ASSIGNMENT0_INC})
target_link_libraries(worksession1.5 PUBLIC core IMGUI assimp)
target_include_directories(worksession1.5 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when worksession1 is built
add_dependencies(worksession1.5 copyAssetsW1.5)
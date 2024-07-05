cmake_minimum_required(VERSION 3.26.4)

project(WebApp)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

set(TEMPLATE_DIR ${CMAKE_SOURCE_DIR}/src/templates)

add_executable(WebApp ${SOURCE_FILES})

target_include_directories(WebApp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_link_libraries(WebApp PRIVATE Ws2_32 Crypt32 "${CMAKE_CURRENT_SOURCE_DIR}/include/lib/sqlite3.lib")

add_custom_command(
	TARGET WebApp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${TEMPLATE_DIR} $<TARGET_FILE_DIR:WebApp>/src/templates
)

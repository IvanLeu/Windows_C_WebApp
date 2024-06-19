cmake_minimum_required(VERSION 3.26.4)

project(WebApp)

set(HEADER_FILES
	src/io.h
	src/user_database.h
	src/global.h
	src/responses.h
	src/server.h
	src/io.h
)

set(SOURCE_FILES
	src/io.c
	src/user_database.c
	src/global.c
	src/server.c
	src/main.c
)

set(TEMPLATE_DIR ${CMAKE_SOURCE_DIR}/src/templates)

add_executable(WebApp ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(WebApp PRIVATE Ws2_32 Crypt32)

add_custom_command(
	TARGET WebApp POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${TEMPLATE_DIR} $<TARGET_FILE_DIR:WebApp>/src/templates
)

cmake_minimum_required(VERSION 3.13.0)
project(nds-rom-editor)

add_definitions(-D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Setup base

file(GLOB_RECURSE nreHpp "include/nre/base/*.hpp")
file(GLOB_RECURSE nreCpp "src/nre/base/*.cpp")

option(NRE_IS_CLI "nds-rom-editor Is Command Line Interface" OFF)

add_subdirectory(igx/ignis/core2)

add_library(nds-rom-editor STATIC ${nreHpp} ${nreCpp} CMakeLists.txt)

target_include_directories(nds-rom-editor PUBLIC include/nre/base)
target_include_directories(nds-rom-editor PUBLIC igx/ignis/core2/include)
target_link_libraries(nds-rom-editor PUBLIC ocore)

source_group("Headers" FILES ${nreHpp})
source_group("Source" FILES ${nreCpp})

if(MSVC)
	target_compile_options(nds-rom-editor PRIVATE /W4 /WX /MD /MP /wd26812 /wd4201 /EHsc /GR)
else()
	target_compile_options(nds-rom-editor PRIVATE -Wall -Wpedantic -Wextra -Werror)
endif()

# Setup versions

if(${NRE_IS_CLI})

	# CLI

	file(GLOB_RECURSE nreCliHpp "include/nre/cli/*.hpp")
	file(GLOB_RECURSE nreCliCpp "src/nre/cli/*.cpp")

	add_executable(
		nds-rom-editor-cli
		${nreCliHpp}
		${nreCliCpp}
		CMakeLists.txt
	)

	set_property(TARGET nds-rom-editor-cli PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/res")

	target_include_directories(nds-rom-editor-cli PUBLIC include/nre/cli)
	target_include_directories(nds-rom-editor-cli PUBLIC third_party)
	target_link_libraries(nds-rom-editor-cli PUBLIC nds-rom-editor)

	source_group("Headers" FILES ${nreCliHpp})
	source_group("Source" FILES ${nreCliCpp})

	if(MSVC)
	    target_compile_options(nds-rom-editor-cli PRIVATE /W4 /WX /MD /MP /wd26812 /wd4201 /EHsc /GR)
	else()
	    target_compile_options(nds-rom-editor-cli PRIVATE -Wall -Wpedantic -Wextra -Werror)
	endif()
	
else()

	# UI

	add_subdirectory(igx)

	file(GLOB_RECURSE nreUiHpp "include/nre/ui/*.hpp")
	file(GLOB_RECURSE nreUiCpp "src/nre/ui/*.cpp")

	add_executable(
		nds-rom-editor-ui
		${nreUiHpp}
		${nreUiCpp}
		CMakeLists.txt
	)

	target_include_directories(nds-rom-editor-ui PUBLIC include/nre/ui)
	target_include_directories(nds-rom-editor-ui PUBLIC include/nre/base)
	target_include_directories(nds-rom-editor-ui PUBLIC igx/include)
	target_link_libraries(nds-rom-editor-ui PUBLIC nds-rom-editor igx)

	configure_icon(nds-rom-editor-ui "${CMAKE_CURRENT_SOURCE_DIR}/res/logo.ico")
	configure_virtual_files(nds-rom-editor-ui)

	set_property(TARGET nds-rom-editor-ui PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/res")

	source_group("Headers" FILES ${nreUiHpp})
	source_group("Source" FILES ${nreUiCpp})

	if(MSVC)
	    target_compile_options(nds-rom-editor-ui PRIVATE /W4 /WX /MD /MP /wd26812 /wd4201 /EHsc /GR)
	else()
	    target_compile_options(nds-rom-editor-ui PRIVATE -Wall -Wpedantic -Wextra -Werror)
	endif()

endif()
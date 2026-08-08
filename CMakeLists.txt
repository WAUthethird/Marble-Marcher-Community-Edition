cmake_minimum_required(VERSION 3.10)
project(MarbleMarcher VERSION "1.4.6")
set(CMAKE_CXX_STANDARD 17)

configure_file(src/config.h.in config.h)

if (WIN32)
	include(FetchContent)

	if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	    message(SEND_ERROR "This project does not support non-64-bit Windows targets")
	endif()

	## DEPENDENCIES

	function(extract_if_needed)
		cmake_parse_arguments(PARSE_ARGV 0 arg
			"" "SRC;DEST;INTERMEDIATE_DIR" ""
		)

		file(TIMESTAMP "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cache/${arg_SRC}" ARCHIVE_TIMESTAMP UTC)
		file(TIMESTAMP "${CMAKE_CURRENT_SOURCE_DIR}/vendor/${arg_DEST}.extracted" EXTRACTED_TIMESTAMP UTC)
		if (EXTRACTED_TIMESTAMP STRLESS ARCHIVE_TIMESTAMP)
			file(REMOVE_RECURSE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/${arg_DEST}")
			file(ARCHIVE_EXTRACT
				INPUT "${CMAKE_CURRENT_SOURCE_DIR}/vendor/cache/${arg_SRC}"
				DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/vendor"
			)
			if(DEFINED arg_INTERMEDIATE_DIR)
				file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/vendor/${arg_INTERMEDIATE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/vendor/${arg_DEST}")
			endif()
			file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/${arg_DEST}.extracted")
		endif()
	endfunction()

	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

	file(MAKE_DIRECTORY vendor/cache)

	# Handle dependencies that we have to build from source (e.g. because their binary distributions
	# don't include the necessary CMake configuration).
	FetchContent_Declare(
		GLEW
		URL https://github.com/nigels-com/glew/releases/download/glew-2.3.1/glew-2.3.1.tgz
		URL_HASH SHA256=b64790f94b926acd7e8f84c5d6000a86cb43967bd1e688b03089079799c9e889
		DOWNLOAD_EXTRACT_TIMESTAMP true
		SOURCE_SUBDIR build/cmake
	)
	FetchContent_Declare(
		OpenAL
		URL https://openal-soft.org/openal-releases/openal-soft-1.25.2.tar.bz2
		URL_HASH SHA256=1dbaac44e7579d5bc8847ca8db4b2e8b9fd3961041f35ee20def4958301e1089
		DOWNLOAD_EXTRACT_TIMESTAMP true
	)
	FetchContent_MakeAvailable(GLEW OpenAL)

	file(DOWNLOAD
		https://www.sfml-dev.org/files/SFML-2.6.2-windows-vc17-64-bit.zip
		${CMAKE_CURRENT_SOURCE_DIR}/vendor/cache/SFML-2.6.2-windows-vc17-64-bit.zip
		SHOW_PROGRESS
		EXPECTED_HASH SHA256=f5995724604d74d06efe89544315d11ac2d3c2d348854c6be55a7a11fb7ecad8
	)
	extract_if_needed(
		SRC SFML-2.6.2-windows-vc17-64-bit.zip
		DEST SFML
	    INTERMEDIATE_DIR SFML-2.6.2
	)

	set(SDK_DIRECTORY "C:/Program Files (x86)/Microsoft SDKs")
	set(EIGEN3_INCLUDE_DIR C:/Libraries/eigen3/)
	set(GLM_INCLUDE_DIR C:/Libraries/glm)
	set(SFML_STATIC_LIBRARIES TRUE)
	set(ANTTWEAKBAR_DIR C:/Libraries/AntTweakBar)

	set(ATB_LIB ${ANTTWEAKBAR_DIR}/lib/AntTweakBar64.lib)
	set(GLEW_LIBRARIES ${GLEW_DIR}/lib/Release/x64/glew32.lib)

	find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)

elseif(UNIX)	
	set(SFML_STATIC_LIBRARIES FALSE)
	find_package(GLEW REQUIRED)
	find_package(OpenGL REQUIRED)
	find_package(Eigen3 REQUIRED)
	find_package(glm REQUIRED)
endif()


## TARGETS
add_subdirectory(src)
target_include_directories(MarbleMarcherSources PUBLIC
  src
  ${EIGEN3_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIRS}
  ${OpenGL_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${GLM_INCLUDE_DIR})

if(WIN32)
	target_include_directories(MarbleMarcherSources PUBLIC
	  ${ANTTWEAKBAR_DIR}/include
	)
	include_directories(${ANTTWEAKBAR_DIR}/include)
	include_directories(${SDK_DIRECTORY}/Windows/v7.1/Include)
else()
	target_include_directories(MarbleMarcherSources PUBLIC
	  src
	  /usr/include
	)
endif()

target_compile_definitions(MarbleMarcherSources PRIVATE SFML_STATIC)

if(WIN32)
  add_executable(MarbleMarcher WIN32 src/Main.cpp src/Resource.rc assets/icon.ico)
  set_source_files_properties(icon.ico Resource.rc PROPERTIES LANGUAGE RC)
else()
  add_executable(MarbleMarcher src/Main.cpp)
endif()

#ADD A COMMAND TO COPY THE FILES FROM GAME_FOLDER TO COMPILATION FOLDER
add_custom_command(TARGET MarbleMarcher PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/game_folder $<TARGET_FILE_DIR:MarbleMarcher>)

if(WIN32)
	add_custom_command(TARGET MarbleMarcher PRE_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory
					${CMAKE_SOURCE_DIR}/bin64 $<TARGET_FILE_DIR:MarbleMarcher>)
endif()	

if(CMAKE_COMPILER_IS_GNUCC)
  # needed for gcc 4.6+
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()				   

target_compile_definitions(MarbleMarcher PRIVATE SFML_STATIC)
if(WIN32)
	target_link_libraries(MarbleMarcher
	  MarbleMarcherSources
	  ${ATB_LIB}
	  glew
	  sfml-system
	  sfml-window
	  sfml-graphics
	  sfml-audio
	)
elseif(UNIX)
	target_link_libraries(MarbleMarcher
		MarbleMarcherSources
		${OPENGL_LIBRARIES}
		libAntTweakBar.so	
		glew
		stdc++fs
		libsfml-system.so
		libsfml-window.so
		libsfml-graphics.so
		libsfml-audio.so
	)
	install(TARGETS MarbleMarcher
	    COMPONENT linapp
	    RUNTIME DESTINATION "/home/MMCE"
	    LIBRARY DESTINATION "/home/MMCE"
	    DESTINATION "/home/MMCE"
	)

	install(DIRECTORY "${PROJECT_SOURCE_DIR}/game_folder/" DESTINATION "/home/MMCE")

	set(CPACK_PACKAGE_NAME "MarbleMarcher")
	set(CPACK_PACKAGE_VENDOR "MarbleMarcher")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A ray marched marble game")
	set(CPACK_PACKAGE_VERSION "${MM_VERSION}")
	set(CPACK_PACKAGE_VERSION_MAJOR "1")
	set(CPACK_PACKAGE_VERSION_MINOR "4")
	set(CPACK_PACKAGE_VERSION_PATCH "6")

	SET(CPACK_GENERATOR "DEB")
	SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "KK") #required
	INCLUDE(CPack)
endif()

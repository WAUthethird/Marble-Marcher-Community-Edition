cmake_minimum_required(VERSION 3.10)
project(MarbleMarcher VERSION "1.4.6")
set(CMAKE_CXX_STANDARD 17)

configure_file(src/config.h.in config.h)

if (WIN32)
	#CHECK THE BITNESS
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	    set(BIT 64)
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	    set(BIT 32)
	endif()

	## DEPENDENCIES
	set(GLEW_DIR C:/Libraries/GLEW/)
	set(GLEW_INCLUDE_DIRS ${GLEW_DIR}/include)
	set(SDK_DIRECTORY "C:/Program Files (x86)/Microsoft SDKs")
	set(EIGEN3_INCLUDE_DIR C:/Libraries/eigen3/)
	set(GLM_INCLUDE_DIR C:/Libraries/glm)
	set(SFML_STATIC_LIBRARIES TRUE)
	set(ANTTWEAKBAR_DIR C:/Libraries/AntTweakBar)

	if(BIT EQUAL 32)
	#32 bit compilation
		set(SFML_DIR C:/Libraries/SFML-2.5.1/lib/cmake/SFML)
		set(SFML_INCLUDE_DIR C:/Libraries/SFML-2.5.1/include)
		set(ATB_LIB ${ANTTWEAKBAR_DIR}/lib/AntTweakBar.lib)
		set(GLEW_LIBRARIES ${GLEW_DIR}/lib/Release/Win32/glew32.lib)
	else()
	#64 bit compilation
		set(SFML_DIR C:/Libraries/SFML-2.5.1_64/lib/cmake/SFML)
		set(SFML_INCLUDE_DIR C:/Libraries/SFML-2.5.1_64/include)
		set(ATB_LIB ${ANTTWEAKBAR_DIR}/lib/AntTweakBar64.lib)
		set(GLEW_LIBRARIES ${GLEW_DIR}/lib/Release/x64/glew32.lib)
	endif()

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
	  ${SFML_INCLUDE_DIR}
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
	if(BIT EQUAL 32)
		add_custom_command(TARGET MarbleMarcher PRE_BUILD
		           COMMAND ${CMAKE_COMMAND} -E copy_directory
		               ${CMAKE_SOURCE_DIR}/bin32 $<TARGET_FILE_DIR:MarbleMarcher>)
	else()
		add_custom_command(TARGET MarbleMarcher PRE_BUILD
		           COMMAND ${CMAKE_COMMAND} -E copy_directory
		               ${CMAKE_SOURCE_DIR}/bin64 $<TARGET_FILE_DIR:MarbleMarcher>)
	endif()	
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
	  ${GLEW_LIBRARIES}
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
		${GLEW_LIBRARIES}
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

cmake_minimum_required(VERSION 3.0)
project(MarbleMarcher)
set(CMAKE_CXX_STANDARD 17)

#CHECK THE BITNESS
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BIT 64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(BIT 32)
endif()

## DEPENDENCIES
set(GLEW_INCLUDE_DIR C:/Libraries/GLEW)
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
	set(GLEW_LIB ${GLEW_INCLUDE_DIR}/lib/Release/Win32/glew32.lib)
else()
#64 bit compilation
	set(SFML_DIR C:/Libraries/SFML-2.5.1_64/lib/cmake/SFML)
	set(SFML_INCLUDE_DIR C:/Libraries/SFML-2.5.1_64/include)
	set(ATB_LIB ${ANTTWEAKBAR_DIR}/lib/AntTweakBar64.lib)
	set(GLEW_LIB ${GLEW_INCLUDE_DIR}/lib/Release/x64/glew32.lib)
endif()

find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)

## TARGETS

add_subdirectory(src)
target_include_directories(MarbleMarcherSources PUBLIC
  src
  ${EIGEN3_INCLUDE_DIR}
  ${SFML_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}/include
  ${ANTTWEAKBAR_DIR}/include
  ${GLM_INCLUDE_DIR}
)

include_directories(${ANTTWEAKBAR_DIR}/include)
include_directories(${SDK_DIRECTORY}/Windows/v7.1/Include)

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
if(BIT EQUAL 32)
	add_custom_command(TARGET MarbleMarcher PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/bin32 $<TARGET_FILE_DIR:MarbleMarcher>)
else()
	add_custom_command(TARGET MarbleMarcher PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/bin64 $<TARGET_FILE_DIR:MarbleMarcher>)
endif()					   

target_compile_definitions(MarbleMarcher PRIVATE SFML_STATIC)
target_link_libraries(MarbleMarcher
  MarbleMarcherSources
  ${ATB_LIB}
  ${GLEW_LIB}
  sfml-system
  sfml-window
  sfml-graphics
  sfml-audio
)

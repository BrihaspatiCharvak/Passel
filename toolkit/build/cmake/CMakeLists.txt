# CMakeLists.txt ____________________________________________________________________________________________________________

cmake_minimum_required(VERSION 3.9 FATAL_ERROR) 
set( CMAKE_VERBOSE_MAKEFILE ON)

project(passel)

set( PS_VERSIONID "1.0") 

#____________________________________________________________________________________________________________________________

OPTION( PS_DBG "Build Library with  Debug option" OFF) 
OPTION( PS_TRACEDBG "enable tracing" OFF)  
OPTION( PS_SANITY "enable SANITY" OFF)  

#____________________________________________________________________________________________________________________________

if( PS_DBG) 
	add_definitions(-DPS_DEBUG) 
endif()


if( PS_SANITY) 
	add_definitions(-DPS_SANITY) 
endif()


if( PS_TRACEDBG) 
   add_definitions(-DPS_TRACEDBG)
endif()
  
#____________________________________________________________________________________________________________________________

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)  

#____________________________________________________________________________________________________________________________

set(ROOT_PATH "${CMAKE_SOURCE_DIR}/../../../")  
set(SOURCE_PATH "${ROOT_PATH}src/")
 
set(BUILD_PATH "${CMAKE_BINARY_DIR}/") 

enable_language( CXX) 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

#------------------------------------------------------------------------------------------------------------------------------
# Set default locations

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_PATH}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_PATH}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_PATH}/lib)
#____________________________________________________________________________________________________________________________

set ( THRDPTY_INCLUDES "${SOURCE_PATH}3rdparty/vectorclass2/" )
set ( THRDPTY_LIBS) 

#____________________________________________________________________________________________________________________________

# Add a compiler flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")

#____________________________________________________________________________________________________________________________
 
if(WIN32)
	#set(CMAKE_WIN32_EXECUTABLE ON)
	add_definitions( /std:c++17)
endif() 

#______________________________________________________________________________________________________________________________

execute_process(COMMAND
	"git" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
	WORKING_DIRECTORY "${ROOT_PATH}"
	OUTPUT_VARIABLE   GIT_SHA1
	ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE) 

set( PS_VERSION ${PS_VERSIONID}-${GIT_SHA1} )

add_definitions(-DPS_VERSION="${PS_VERSION}")

#____________________________________________________________________________________________________________________________ 


include_directories( ${SOURCE_PATH} ${THRDPTY_INCLUDES}  )

#____________________________________________________________________________________________________________________________

set( PS_HIERON_SOURCES 
	${SOURCE_PATH}socle/tenor/sc_include.cpp  
)

add_library( hieron STATIC ${PS_HIERON_SOURCES} )

#____________________________________________________________________________________________________________________________

set( PS_SOCLE_SOURCES 
	${SOURCE_PATH}socle/tenor/sc_include.cpp  
)

add_library( socle STATIC ${PS_SOCLE_SOURCES} )

#____________________________________________________________________________________________________________________________
 
# Create the sisyphus executable
add_executable( sisyphus ${SOURCE_PATH}sisyphus/sc_console.cpp)
target_link_libraries( sisyphus    socle hieron ${THRDPTY_LIBS}   ${EIGENLIB} )
target_compile_features( sisyphus PRIVATE cxx_std_17)

#____________________________________________________________________________________________________________________________

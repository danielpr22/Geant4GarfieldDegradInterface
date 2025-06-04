#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10)
project(DMPX)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT QUIET REQUIRED COMPONENTS RIO Net)
if(NOT ROOT_FOUND)
  message(STATUS "DMPX: ROOT package not found. --> DMPX disabled")  
  return()
endif()
include(${ROOT_USE_FILE})

# Add additional libraries needed for GARFIELD++
link_directories($ENV{GARFIELD_LIB})


IF(APPLE)
    # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    LINK_DIRECTORIES(/usr/local/lib)
ENDIF()

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB project_headers ${PROJECT_SOURCE_DIR}/include/*.hh)
file(GLOB garfield_headers $ENV{GARFIELD_HOME}/include/*.hh)
file(GLOB heed_headers $ENV{GARFIELD_HOME}/heed/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 and GARFIELD++ libraries
find_package(Garfield REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O0")
add_executable(DMPX DMPX.cc ${sources} ${project_headers} ${garfield_headers} ${heed_headers})
target_link_libraries(DMPX -lGarfield -lgfortran ${ROOT_LIBRARIES} ${Geant4_LIBRARIES} ${Garfield_LIBRARIES})

# Setup GARFIELD++ include directories in the private scope to avoid confilcts with other targets
target_include_directories(DMPX PRIVATE
  ${Garfield_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/include
  $ENV{GARFIELD_DIR}
  $ENV{GEANT4_DIR}
  $ENV{GARFIELD_HOME}/include
  $ENV{GARFIELD_HOME}/share/Heed
  ${Geant4_INCLUDE_DIRS}
)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory. This is so that we can run the executable 
# directly because it relies on these scripts being in the current working directory.

set(DMPX_SCRIPTS
  vis.mac
  run1.mac
)

foreach(_script ${DMPX_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS DMPX DESTINATION bin)

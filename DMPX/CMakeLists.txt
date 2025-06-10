# Setting up the project and the CMake and C++ versions
cmake_minimum_required(VERSION 3.10)
project(DMPX_G_factor)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED True)

# Ensure required environment variables are set
if(NOT DEFINED ENV{GARFIELD_HOME})
  message(FATAL_ERROR "Environment variable GARFIELD_HOME is not set.")
endif()

if(NOT DEFINED ENV{GEANT4_DIR})
  message(FATAL_ERROR "Environment variable GEANT4_DIR is not set.")
endif()

if(NOT DEFINED ENV{DEGRAD_HOME})
  message(FATAL_ERROR "Environment variable DEGRAD_HOME is not set.")
endif()

# Find Geant4 package, activating all available UI and Vis drivers
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()
include(${Geant4_USE_FILE})

# Find ROOT (required package)
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT QUIET REQUIRED COMPONENTS RIO Net)
if(NOT ROOT_FOUND)
  message(STATUS "DMPX_G_factor: ROOT package not found. --> DMPX_G_factor disabled")  
  return()
endif()
include(${ROOT_USE_FILE})

# Setup Geant4 include directories and compile definitions
include_directories(${PROJECT_SOURCE_DIR}/include)

# Setup GARFIELD++ include directories
include_directories($ENV{GARFIELD_DIR})
include_directories($ENV{GEANT4_DIR}/)
include_directories($ENV{GARFIELD_HOME}/include)
include_directories($ENV{GARFIELD_HOME}/share/Heed)

# Add additional libraries needed for GARFIELD++
link_directories($ENV{GARFIELD_LIB})

# For Apple users
IF(APPLE)
    # Fix linking on 10.14+. See https://stackoverflow.com/questions/54068035
    LINK_DIRECTORIES(/usr/local/lib)
ENDIF()

# Explicitly list source and header files from the current folder
set(sources
	${PROJECT_SOURCE_DIR}/src/DegradModel.cc
	${PROJECT_SOURCE_DIR}/src/DetectorConstruction.cc
	${PROJECT_SOURCE_DIR}/src/DetectorMessenger.cc
	${PROJECT_SOURCE_DIR}/src/AnodesSD.cc
	${PROJECT_SOURCE_DIR}/src/DriftLineTrajectory.cc
	${PROJECT_SOURCE_DIR}/src/DriftLineTrajectoryPoint.cc
	${PROJECT_SOURCE_DIR}/src/EventAction.cc
	${PROJECT_SOURCE_DIR}/src/GasBoxHit.cc
	${PROJECT_SOURCE_DIR}/src/GasBoxSD.cc
	${PROJECT_SOURCE_DIR}/src/GasModelParameters.cc
	${PROJECT_SOURCE_DIR}/src/GasModelParametersMessenger.cc
	${PROJECT_SOURCE_DIR}/src/HeedDeltaElectronModel.cc
	${PROJECT_SOURCE_DIR}/src/HeedModel.cc
	${PROJECT_SOURCE_DIR}/src/MyUserActionInitialization.cc
	${PROJECT_SOURCE_DIR}/src/PhysicsList.cc
	${PROJECT_SOURCE_DIR}/src/PhysicsListMessenger.cc
	${PROJECT_SOURCE_DIR}/src/PhysListEmStandard.cc
	${PROJECT_SOURCE_DIR}/src/PrimaryGeneratorAction.cc
	${PROJECT_SOURCE_DIR}/src/RunAction.cc
)

set(headers
	${PROJECT_SOURCE_DIR}/include/DegradModel.hh
	${PROJECT_SOURCE_DIR}/include/DetectorConstruction.hh
	${PROJECT_SOURCE_DIR}/include/DetectorMessenger.hh
	${PROJECT_SOURCE_DIR}/include/AnodesSD.hh
	${PROJECT_SOURCE_DIR}/include/DriftLineTrajectory.hh
	${PROJECT_SOURCE_DIR}/include/DriftLineTrajectoryPoint.hh
	${PROJECT_SOURCE_DIR}/include/EventAction.hh
	${PROJECT_SOURCE_DIR}/include/GasBoxHit.hh
	${PROJECT_SOURCE_DIR}/include/GasBoxSD.hh
	${PROJECT_SOURCE_DIR}/include/GasModelParameters.hh
	${PROJECT_SOURCE_DIR}/include/GasModelParametersMessenger.hh
	${PROJECT_SOURCE_DIR}/include/HeedDeltaElectronModel.hh
	${PROJECT_SOURCE_DIR}/include/HeedModel.hh
	${PROJECT_SOURCE_DIR}/include/MyUserActionInitialization.hh
	${PROJECT_SOURCE_DIR}/include/PhysicsList.hh
	${PROJECT_SOURCE_DIR}/include/PhysicsListMessenger.hh
	${PROJECT_SOURCE_DIR}/include/PhysListEmStandard.hh
	${PROJECT_SOURCE_DIR}/include/PrimaryGeneratorAction.hh
	${PROJECT_SOURCE_DIR}/include/RunAction.hh
)


# Add the executable and link it to the required libraries
#find_package(Garfield REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
add_executable(DMPX_G_factor main_files/DMPX_G_factor.cc ${sources} ${headers})
target_link_libraries(DMPX_G_factor -lGarfield -lgfortran ${ROOT_LIBRARIES} ${Geant4_LIBRARIES})

# Restrict include directories to the current folder
target_include_directories(DMPX_G_factor PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${Garfield_INCLUDE_DIRS}
  $ENV{GARFIELD_DIR}
  $ENV{GEANT4_DIR}
  $ENV{GARFIELD_HOME}/include
  $ENV{GARFIELD_HOME}/share/Heed
  ${Geant4_INCLUDE_DIRS}
  $ENV{DEGRAD_HOME}
)

# Copy all scripts to the build directory
set(DMPX_G_factor_SCRIPTS
  run_files/G_factor.mac
  kr_90_ch4_10.gas
  convertDegradFile.py
)

foreach(_script ${DMPX_G_factor_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
install(TARGETS DMPX_G_factor DESTINATION bin)



#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10)
project(Interface)

add_subdirectory(DMPX_v2)
# add_subdirectory(ALICE)
# add_subdirectory(Xenon)

# add_custom_target(Interface DEPENDS ALICE DMPX)
add_custom_target(Interface DEPENDS DMPX_v2)

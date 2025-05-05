#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10)
project(Interface)

add_subdirectory(Xenon_backup)
# add_subdirectory(ALICE)
# add_subdirectory(Xenon)

# add_custom_target(Interface DEPENDS ALICE DMPX)
add_custom_target(Interface DEPENDS Xenon_backup)

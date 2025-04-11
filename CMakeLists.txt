#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10)

add_subdirectory(Xenon)
add_subdirectory(ALICE)

add_custom_target(Interface DEPENDS Xenon ALICE)

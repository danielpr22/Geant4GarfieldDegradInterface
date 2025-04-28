#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.10)
project(Interface)

# add_subdirectory(Xenon)
# add_subdirectory(ALICE)
add_subdirectory(DMPX)

# add_custom_target(Interface DEPENDS ALICE DMPX-with_comments)
add_custom_target(Interface DEPENDS DMPX)

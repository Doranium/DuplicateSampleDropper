cmake_minimum_required (VERSION 3.10)

project (duplicate_sample_dropper)

find_package(ADTF COMPONENTS filtersdk)

# Adds the duplicate_sample_dropper project to the Visual Studio solution, which when build
# creates a shared object called duplicate_sample_dropper.adtfplugin
adtf_add_filter(${CMAKE_PROJECT_NAME} 
                duplicate_sample_dropper.h 
                duplicate_sample_dropper.cpp)

# Adds the INSTALL project to the Visual Studio solution, which when build
# copies our filter to the subdirectory given as the second argument into ${CMAKE_INSTALL_PREFIX}
adtf_install_filter(${CMAKE_PROJECT_NAME} bin)

# Generate a plugindescription for our filter
adtf_create_plugindescription(TARGET ${CMAKE_PROJECT_NAME} PLUGIN_SUBDIR "bin")

# ignore linker warning 4099 for missing PDBs
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")


cmake_minimum_required (VERSION 2.6)

project (lvtwohost)

# Find depend libraries
find_package(PkgConfig)

pkg_check_modules(LILV lilv-0 REQUIRED)
include_directories( ${LILV_INCLUDE_DIRS}  )
link_directories   ( ${LILV_LIBRARY_DIRS}  )

pkg_check_modules(SNDFILE sndfile REQUIRED)
include_directories( ${SNDFILE_INCLUDE_DIRS}  )
link_directories   ( ${SNDFILE_LIBRARY_DIRS}  )

# Add all files
FILE(GLOB sources *.cxx )

# Compile binary
add_executable (lvtwohost ${sources} )

target_link_libraries( lvtwohost ${LILV_LIBRARIES}    )
target_link_libraries( lvtwohost ${SNDFILE_LIBRARIES} )


# add the install targets
install (TARGETS lvtwohost DESTINATION bin)

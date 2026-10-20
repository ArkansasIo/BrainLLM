# Copy a vendor/source directory only when it exists.
# Usage: cmake -DSOURCE_DIR=<path> -DDEST_DIR=<path> -P CopyOptionalDirectory.cmake
if(NOT DEFINED SOURCE_DIR OR NOT DEFINED DEST_DIR)
    message(FATAL_ERROR "SOURCE_DIR and DEST_DIR must be provided")
endif()

if(IS_DIRECTORY "${SOURCE_DIR}")
    get_filename_component(DEST_PARENT "${DEST_DIR}" DIRECTORY)
    file(MAKE_DIRECTORY "${DEST_PARENT}")
    file(COPY "${SOURCE_DIR}" DESTINATION "${DEST_PARENT}")
else()
    message(STATUS "Skipping absent optional directory: ${SOURCE_DIR}")
endif()

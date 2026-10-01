add_executable(sightline_client
    main.cpp
)

target_link_libraries(sightline_client PRIVATE raylib sightline_shared)

# The client runs from the build directory, so give it an absolute path to the maps.
target_compile_definitions(sightline_client PRIVATE SIGHTLINE_MAPS_DIR="${CMAKE_SOURCE_DIR}/maps")

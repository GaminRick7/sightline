add_executable(sightline_client
    main.cpp
)

target_link_libraries(sightline_client PRIVATE raylib sightline_shared)

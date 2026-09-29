add_executable(sightline_server
    main.cpp
)

target_link_libraries(sightline_server PRIVATE sightline_shared)

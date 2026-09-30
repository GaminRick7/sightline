add_library(sightline_shared STATIC
    src/config.cpp
    src/map.cpp
)

target_include_directories(sightline_shared PUBLIC include)

project(octopus)

# add_library(octopus src/pool.cpp)

include_directories(src)
add_executable(main main.cpp)
target_link_libraries(main  pthread)
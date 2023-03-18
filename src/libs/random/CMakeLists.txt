set(SOURCES
        src/RandomGenerator.cpp
        )

add_library(random ${SOURCES})

target_include_directories(random
        PUBLIC
        "include"
        PRIVATE
        "include/random"
        )

add_subdirectory(libs)
add_subdirectory(core)

FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

add_executable(${PROJECT_NAME} Main.cpp CommandLineArgumentParser.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
        core
        ${Boost_INCLUDE_DIR}
        ${BENCODE_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC core fmt::fmt ${Boost_LIBRARIES} collection fileSystem httpClient)

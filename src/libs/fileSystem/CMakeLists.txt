set(SOURCES
        src/FileSystemServiceImpl.cpp
        src/GetProjectPath.cpp
        src/GetExecutablePath.cpp
        src/FileSystemServiceFactory.cpp
        )

set(UT_SOURCES
        src/FileSystemServiceImplTest.cpp
        src/GetExecutablePathTest.cpp
        src/GetProjectPathTest.cpp
        )


add_library(fileSystem ${SOURCES})
add_executable(fileSystemUT ${SOURCES} ${UT_SOURCES})

target_include_directories(fileSystem
        PUBLIC "include"
        PRIVATE "include/fileSystem"
        ${Boost_INCLUDE_DIR}
        )
target_include_directories(fileSystemUT
        PRIVATE
        "include/fileSystem"
        ${Boost_INCLUDE_DIR}
        ${GTEST_INCLUDE_DIR}
        )

target_link_libraries(fileSystem PUBLIC collection fmt::fmt)

target_link_libraries(fileSystemUT PRIVATE gtest_main collection fmt::fmt)

add_test(fileSystemUT fileSystemUT --gtest_color=yes)

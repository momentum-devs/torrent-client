set(SOURCES
        FileSystemService.cpp
        UdpClientImpl.cpp
        )

set(UT_SOURCES
        UdpClientImplTest.cpp
        )

add_executable(${PROJECT_NAME} Main.cpp ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC fmt)

target_include_directories(${PROJECT_NAME} PUBLIC ${GMOCK_INCLUDE_DIR})

add_executable(${PROJECT_NAME}UT ${UT_SOURCES} ${SOURCES})

target_link_libraries(${PROJECT_NAME}UT PUBLIC fmt gtest gmock_main)

target_include_directories(${PROJECT_NAME}UT PUBLIC ${GMOCK_INCLUDE_DIR})

add_test(NAME ${PROJECT_NAME}UT COMMAND ${PROJECT_NAME}UT WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

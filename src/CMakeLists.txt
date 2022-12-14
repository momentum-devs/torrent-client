set(SOURCES
        FileSystemService.cpp
        TorrentFileParserImpl.cpp
        PeerRetrieverImpl.cpp
        )

set(UT_SOURCES
        PeerRetrieverImplTest.cpp
        )

add_executable(${PROJECT_NAME} Main.cpp ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC fmt cpr::cpr)

target_include_directories(${PROJECT_NAME} PUBLIC ${BENCODE_INCLUDE_DIR} ${CPR_INCLUDE_DIR})

add_executable(${PROJECT_NAME}UT ${UT_SOURCES} ${SOURCES})

target_link_libraries(${PROJECT_NAME}UT PUBLIC fmt gtest gmock_main cpr::cpr)

target_include_directories(${PROJECT_NAME}UT PUBLIC ${GMOCK_INCLUDE_DIR} ${CPR_INCLUDE_DIR})

add_test(NAME ${PROJECT_NAME}UT COMMAND ${PROJECT_NAME}UT WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

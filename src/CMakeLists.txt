add_subdirectory(common)

set(SOURCES
        TorrentFileDeserializerImpl.cpp
        PeerRetrieverImpl.cpp
        HexEncoder.cpp
        PeerIdGenerator.cpp
        AnnounceResponseDeserializerImpl.cpp
        HandshakeMessageSerializer.cpp
        PeerConnector.cpp
        MessageSerializer.cpp
        Bitfield.cpp
        )

set(UT_SOURCES
        PeerRetrieverImplTest.cpp
        TorrentFileDeserializerImplTest.cpp
        AnnounceResponseDeserializerImplTest.cpp
        HandshakeMessageSerializerTest.cpp
        MessageSerializerTest.cpp
        BitfieldTest.cpp
        )

find_package(Boost 1.30 COMPONENTS program_options REQUIRED)

add_executable(${PROJECT_NAME} Main.cpp ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC collection fileSystem httpClient ${Boost_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC ${BENCODE_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

add_executable(${PROJECT_NAME}UT ${UT_SOURCES} ${SOURCES})

target_link_libraries(${PROJECT_NAME}UT PUBLIC collection fileSystem httpClient fmt gtest gmock_main)

target_include_directories(${PROJECT_NAME}UT PUBLIC ${GMOCK_INCLUDE_DIR} ${BENCODE_INCLUDE_DIR})

add_test(NAME ${PROJECT_NAME}UT COMMAND ${PROJECT_NAME}UT WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

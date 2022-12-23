set(SOURCES
        src/torrentFile/TorrentFileDeserializerImpl.cpp
        src/tracker/PeersRetrieverImpl.cpp
        src/HexEncoder.cpp
        src/client/PeerIdGenerator.cpp
        src/tracker/AnnounceResponseDeserializerImpl.cpp
        src/session/HandshakeMessageSerializer.cpp
        src/session/PeerToPeerSessionImpl.cpp
        src/session/MessageSerializer.cpp
        src/client/TorrentClient.cpp
        src/session/Bitfield.cpp
        )

set(UT_SOURCES
        src/tracker/PeersRetrieverImplTest.cpp
        src/torrentFile/TorrentFileDeserializerImplTest.cpp
        src/tracker/AnnounceResponseDeserializerImplTest.cpp
        src/session/HandshakeMessageSerializerTest.cpp
        src/session/MessageSerializerTest.cpp
        src/session/BitfieldTest.cpp
        )

add_library(core ${SOURCES})
add_executable(coreUT ${SOURCES} ${UT_SOURCES})

target_include_directories(core
        PUBLIC
        "../common/collection/include"
        "../common/fileSystem/include"
        "../common/httpClient/include"
        "include"
        PRIVATE
        ${Boost_INCLUDE_DIR}
        ${BENCODE_INCLUDE_DIR}
        )

target_include_directories(coreUT PRIVATE
        "include"
        "../common/collection/include"
        "../common/fileSystem/include"
        "../common/httpClient/include"
        ${Boost_INCLUDE_DIR}
        ${BENCODE_INCLUDE_DIR}
        ${GTEST_INCLUDE_DIR}
        )

target_link_libraries(core PRIVATE collection fileSystem httpClient fmt gtest_main gmock_main)
target_link_libraries(coreUT PRIVATE collection fileSystem httpClient fmt gtest_main gmock_main)

target_compile_options(core PUBLIC ${FLAGS})
target_compile_options(coreUT PUBLIC ${FLAGS})

add_test(coreUT --gtest_color=yes)

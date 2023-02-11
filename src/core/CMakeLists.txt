set(SOURCES
        src/torrentFile/TorrentFileDeserializerImpl.cpp
        src/tracker/PeersRetrieverImpl.cpp
        src/client/PeerIdGenerator.cpp
        src/tracker/AnnounceResponseDeserializerImpl.cpp
        src/session/HandshakeMessageSerializer.cpp
        src/session/PeerToPeerSessionImpl.cpp
        src/session/MessageSerializer.cpp
        src/session/PieceRepositoryImpl.cpp
        src/client/TorrentClient.cpp
        )

set(UT_SOURCES
        src/tracker/PeersRetrieverImplTest.cpp
        src/torrentFile/TorrentFileDeserializerImplTest.cpp
        src/tracker/AnnounceResponseDeserializerImplTest.cpp
        src/session/HandshakeMessageSerializerTest.cpp
        src/session/MessageSerializerTest.cpp
        src/session/PieceRepositoryImplTest.cpp
        )

add_library(core ${SOURCES})
add_executable(coreUT ${SOURCES} ${UT_SOURCES})

target_include_directories(core
        PUBLIC
        "include"
        PRIVATE
        ${Boost_INCLUDE_DIR}
        ${BENCODE_INCLUDE_DIR}
        )

target_include_directories(coreUT PRIVATE
        "include"
        ${Boost_INCLUDE_DIR}
        ${BENCODE_INCLUDE_DIR}
        ${GTEST_INCLUDE_DIR}
        )

target_link_libraries(core PRIVATE collection fileSystem httpClient encoder bytes fmt)
target_link_libraries(coreUT PRIVATE collection fileSystem httpClient encoder bytes fmt gtest_main gmock_main)

add_test(coreUT coreUT --gtest_color=yes)

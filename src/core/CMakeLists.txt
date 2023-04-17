set(SOURCES
        src/torrentFile/TorrentFileDeserializerImpl.cpp
        src/tracker/PeersRetrieverImpl.cpp
        src/client/PeerIdGenerator.cpp
        src/tracker/AnnounceResponseDeserializerImpl.cpp
        src/session/HandshakeMessageSerializer.cpp
        src/session/PeerToPeerSessionImpl.cpp
        src/session/MessageSerializer.cpp
        src/session/PieceRepositoryImpl.cpp
        src/client/TorrentClientImpl.cpp
        src/session/PiecesSerializerImpl.cpp
        src/session/HashValidator.cpp
        src/client/TorrentClientFactory.cpp
        src/session/PeerToPeerSessionManager.cpp)

set(UT_SOURCES
        src/torrentFile/TorrentFileDeserializerImplTest.cpp
        src/tracker/AnnounceResponseDeserializerImplTest.cpp
        src/session/HandshakeMessageSerializerTest.cpp
        src/session/MessageSerializerTest.cpp
        src/session/HashValidatorTest.cpp
        src/session/PieceRepositoryImplSingleFileTest.cpp
        src/session/PieceRepositoryImplMultiFileTest.cpp
        src/session/PiecesSerializerImplTest.cpp
        )

add_library(core ${SOURCES})
add_executable(coreUT ${SOURCES} ${UT_SOURCES})

target_include_directories(core
        PUBLIC
        "include"
        PRIVATE
        ${Boost_INCLUDE_DIR}
        ${BENCODE_INCLUDE_DIR}
        ${JSON_INCLUDE_DIR}
        )

target_include_directories(coreUT PRIVATE
        "include"
        ${Boost_INCLUDE_DIR}
        ${BENCODE_INCLUDE_DIR}
        ${GTEST_INCLUDE_DIR}
        ${JSON_INCLUDE_DIR}
        )

target_link_libraries(core PRIVATE collection fileSystem httpClient encoder bytes random fmt loguru::loguru)
target_link_libraries(coreUT PRIVATE collection fileSystem httpClient encoder bytes fmt random gtest_main gmock_main loguru::loguru)

add_test(coreUT coreUT --gtest_color=yes)

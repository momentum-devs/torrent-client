set(SOURCES
        src/HexEncoder.cpp
        )

set(UT_SOURCES
        src/HexEncoderTest.cpp
        )

add_library(encoder ${SOURCES})
add_executable(encoderUT ${SOURCES} ${UT_SOURCES})

target_include_directories(encoder
        PUBLIC
        "include"
        PRIVATE
        "include/encoder"
        )
target_include_directories(encoderUT PRIVATE
        "include/encoder"
        ${GTEST_INCLUDE_DIR}
        )

target_link_libraries(encoderUT PRIVATE gtest_main)

add_test(encoderUT encoderUT --gtest_color=yes)

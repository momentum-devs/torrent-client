set(SOURCES
        src/BytesConverter.cpp
        src/Bitfield.cpp
        )

set(UT_SOURCES
        src/BytesConverterTest.cpp
        src/BitfieldTest.cpp
        )

add_library(bytes ${SOURCES})
add_executable(bytesUT ${SOURCES} ${UT_SOURCES})

target_include_directories(bytes
        PUBLIC
        "include"
        PRIVATE
        "include/bytes"
        )
target_include_directories(bytesUT PRIVATE
        "include/bytes"
        ${GTEST_INCLUDE_DIR}
        )

target_link_libraries(bytes PRIVATE fmt)

target_link_libraries(bytesUT PRIVATE gtest_main fmt)

add_test(bytesUT bytesUT --gtest_color=yes)

#pragma once

#include <string>

struct FilePieceInfo
{
    unsigned int firstPieceId;
    unsigned int firstPieceOffset;
    unsigned int lastPieceId;
    unsigned int lastPieceOffset;
    std::string filePath;
};

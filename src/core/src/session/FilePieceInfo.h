#pragma once

#include <string>

struct FilePieceInfo
{
    unsigned int firstPiece;
    unsigned int firstPieceOffset;
    unsigned int lastPiece;
    unsigned int lastPieceOffset;
    std::string filePath;
};

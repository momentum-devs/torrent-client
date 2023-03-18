#pragma once

#include <string>

struct FilePieceInfo
{
    unsigned int firstPiece;
    unsigned int lastPiece;
    std::string filePath;
};
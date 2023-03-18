#pragma once

#include <string>
#include <vector>

namespace core
{
class PieceRepository
{
public:
    virtual ~PieceRepository() = default;

    virtual void save(unsigned int pieceId, const std::basic_string<unsigned char>& data) = 0;
    virtual const std::vector<unsigned int>& getDownloadedPieces() const = 0;
    virtual bool contains(unsigned int pieceId) const = 0;
};
}

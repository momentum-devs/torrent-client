#include "PieceRepositoryImpl.h"

#include <utility>

namespace core
{
PieceRepositoryImpl::PieceRepositoryImpl(std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemServiceInit,
                                         unsigned int pieceSizeInit, const std::string& absoluteFilePathInit)
    : fileSystemService{std::move(fileSystemServiceInit)},
      pieceSize{pieceSizeInit},
      absoluteFilePath{absoluteFilePathInit}
{
}

void PieceRepositoryImpl::save(unsigned int pieceId, const std::basic_string<unsigned char>& data)
{
    const auto position = pieceId * pieceSize;

    fileSystemService->writeAtPosition(absoluteFilePath, data, position);
}

std::vector<unsigned int> PieceRepositoryImpl::findAllPiecesIds() const
{
    return {};
}

bool PieceRepositoryImpl::contains(unsigned int pieceId) const
{
    return false;
}
}

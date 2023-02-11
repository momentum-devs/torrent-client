#include "PieceRepositoryImpl.h"

#include <utility>

namespace core
{
PieceRepositoryImpl::PieceRepositoryImpl(std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemServiceInit,
                                         std::shared_ptr<PiecesSerializer> piecesSerializerInit,
                                         unsigned int pieceSizeInit, const std::string& absoluteDataFilePathInit,
                                         const std::string& absoluteMetadataFilePathInit)
    : fileSystemService{std::move(fileSystemServiceInit)},
      piecesSerializer{std::move(piecesSerializerInit)},
      pieceSize{pieceSizeInit},
      absoluteDataFilePath{absoluteDataFilePathInit},
      absoluteMetadataFilePath{absoluteMetadataFilePathInit}
{
    if (not fileSystemService->exists(absoluteDataFilePath))
    {
        fileSystemService->write(absoluteDataFilePath, "");
    }

    if (not fileSystemService->exists(absoluteMetadataFilePath))
    {
        fileSystemService->write(absoluteMetadataFilePath, "[]");
    }
}

void PieceRepositoryImpl::save(unsigned int pieceId, const std::basic_string<unsigned char>& data)
{
    std::lock_guard<std::mutex> guard(lock);

    const auto metadataFileContent = fileSystemService->read(absoluteMetadataFilePath);

    auto currentPiecesIds = piecesSerializer->deserialize(metadataFileContent);

    const auto pieceAlreadyExists =
        std::any_of(currentPiecesIds.cbegin(), currentPiecesIds.cend(), [&](unsigned int id) { return id == pieceId; });

    if (pieceAlreadyExists)
    {
        return;
    }

    const auto position = pieceId * pieceSize;

    fileSystemService->writeAtPosition(absoluteDataFilePath, data, position);

    currentPiecesIds.push_back(pieceId);

    const auto serializedPiecesIds = piecesSerializer->serialize(currentPiecesIds);

    fileSystemService->write(absoluteMetadataFilePath, serializedPiecesIds);
}

std::vector<unsigned int> PieceRepositoryImpl::findAllPiecesIds() const
{
    std::lock_guard<std::mutex> guard(lock);

    const auto metadataFileContent = fileSystemService->read(absoluteMetadataFilePath);

    auto piecesIds = piecesSerializer->deserialize(metadataFileContent);

    return piecesIds;
}

bool PieceRepositoryImpl::contains(unsigned int pieceId) const
{
    std::lock_guard<std::mutex> guard(lock);

    const auto metadataFileContent = fileSystemService->read(absoluteMetadataFilePath);

    const auto piecesIds = piecesSerializer->deserialize(metadataFileContent);

    return std::any_of(piecesIds.cbegin(), piecesIds.cend(), [&](unsigned int id) { return id == pieceId; });
}
}

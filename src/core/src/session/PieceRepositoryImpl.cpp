#include "PieceRepositoryImpl.h"

#include <utility>

#include "fmt/core.h"

namespace core
{
PieceRepositoryImpl::PieceRepositoryImpl(std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemServiceInit,
                                         std::shared_ptr<PiecesSerializer> piecesSerializerInit,
                                         unsigned int pieceSizeInit,
                                         std::shared_ptr<TorrentFileInfo> torrentFileInfoInit,
                                         const std::string& destinationDirectory)
    : fileSystemService{std::move(fileSystemServiceInit)},
      piecesSerializer{std::move(piecesSerializerInit)},
      pieceSize{pieceSizeInit},
      torrentFileInfo{std::move(torrentFileInfoInit)},
      absoluteMetadataFilePath{fmt::format("{}/{}.metadata", destinationDirectory, torrentFileInfo->name)}
{
    if (torrentFileInfo->nestedFilesInfo)
    {
        long long lastFilePiece = 0;

        for (auto nestedFileInfo : *torrentFileInfo->nestedFilesInfo)
        {
            auto absoluteDataFilePath =
                fmt::format("{}/{}/{}", destinationDirectory, torrentFileInfo->name, nestedFileInfo.name);

            std::cout << absoluteDataFilePath << std::endl;

            if (not fileSystemService->exists(absoluteDataFilePath))
            {
                fileSystemService->write(absoluteDataFilePath, "");
            }

            const unsigned int firstPiece = lastFilePiece;

            const unsigned int lastPiece = firstPiece + nestedFileInfo.length / torrentFileInfo->pieceLength;

            const FilePieceInfo filePieceInfo{firstPiece, lastPiece, absoluteDataFilePath};

            filesInfo.push_back(filePieceInfo);

            lastFilePiece = lastPiece;
        }
    }
    else
    {
        auto absoluteDataFilePath = fmt::format("{}/{}", destinationDirectory, torrentFileInfo->name);

        std::cout << absoluteDataFilePath << std::endl;

        if (not fileSystemService->exists(absoluteDataFilePath))
        {
            fileSystemService->write(absoluteDataFilePath, "");
        }

        const unsigned int firstPiece = 0;

        const unsigned int lastPiece = torrentFileInfo->length / torrentFileInfo->pieceLength;

        const FilePieceInfo filePieceInfo{firstPiece, lastPiece, absoluteDataFilePath};

        filesInfo.push_back(filePieceInfo);
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

    const auto& fileInfo = getFileInfo(pieceId);

    const auto position = (pieceId - fileInfo.firstPiece) * pieceSize;

    fileSystemService->writeAtPosition(fileInfo.filePath, data, position);

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

const FilePieceInfo& PieceRepositoryImpl::getFileInfo(unsigned int pieceId) const
{
    for (std::size_t i = 0; i < filesInfo.size(); ++i)
    {
        if (pieceId < filesInfo[i].lastPiece)
        {
            return filesInfo[i];
        }
    }

    throw std::out_of_range{fmt::format("PieceId out of range ({} out of {})", pieceId, filesInfo.end()->lastPiece)};
}
}

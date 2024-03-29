#include "PieceRepositoryImpl.h"

#include <cmath>
#include <utility>

#include "fmt/core.h"

#include "bytes/BytesConverter.h"

namespace core
{
PieceRepositoryImpl::PieceRepositoryImpl(std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemServiceInit,
                                         unsigned int pieceSizeInit,
                                         std::shared_ptr<TorrentFileInfo> torrentFileInfoInit,
                                         const std::string& destinationDirectory)
    : fileSystemService{std::move(fileSystemServiceInit)},
      pieceSize{pieceSizeInit},
      torrentFileInfo{std::move(torrentFileInfoInit)},
      absoluteMetadataFilePath{fmt::format("{}/{}.metadata", destinationDirectory, torrentFileInfo->name)}
{
    if (torrentFileInfo->nestedFilesInfo)
    {
        long long bytesLengthSum = 0;

        for (auto nestedFileInfo : *torrentFileInfo->nestedFilesInfo)
        {
            auto absoluteDataFilePath =
                fmt::format("{}/{}/{}", destinationDirectory, torrentFileInfo->name, nestedFileInfo.name);

            if (not fileSystemService->exists(absoluteDataFilePath))
            {
                fileSystemService->write(absoluteDataFilePath, "");
            }

            const unsigned int firstPiece = bytesLengthSum / torrentFileInfo->pieceLength;

            const unsigned int firstPieceOffset = bytesLengthSum % torrentFileInfo->pieceLength;

            bytesLengthSum += nestedFileInfo.length;

            const unsigned int lastPiece = bytesLengthSum / torrentFileInfo->pieceLength;

            const unsigned int lastPieceOffset = bytesLengthSum % torrentFileInfo->pieceLength;

            const FilePieceInfo filePieceInfo{firstPiece, firstPieceOffset, lastPiece, lastPieceOffset,
                                              absoluteDataFilePath};

            allFilesInfo.push_back(filePieceInfo);
        }
    }
    else
    {
        auto absoluteDataFilePath = fmt::format("{}/{}", destinationDirectory, torrentFileInfo->name);

        if (not fileSystemService->exists(absoluteDataFilePath))
        {
            fileSystemService->write(absoluteDataFilePath, "");
        }

        const unsigned int firstPiece = 0;

        const unsigned int firstPieceOffset = 0;

        const unsigned int lastPiece = torrentFileInfo->length / torrentFileInfo->pieceLength;

        const unsigned int lastPieceOffset = torrentFileInfo->length % torrentFileInfo->pieceLength;

        const FilePieceInfo filePieceInfo{firstPiece, firstPieceOffset, lastPiece, lastPieceOffset,
                                          absoluteDataFilePath};

        allFilesInfo.push_back(filePieceInfo);
    }

    if (not fileSystemService->exists(absoluteMetadataFilePath))
    {
        fileSystemService->write(absoluteMetadataFilePath, "");
    }
    else
    {
        auto data = fileSystemService->read(absoluteMetadataFilePath);
        for (size_t i = 0; i < data.size(); i += 4)
        {
            downloadedPiecesIds.push_back(libs::bytes::BytesConverter::bytesToInt(
                std::basic_string<unsigned char>{data.c_str() + i, data.c_str() + i + 4}));
        }
    }
}

void PieceRepositoryImpl::save(unsigned int pieceId, const std::basic_string<unsigned char>& data)
{
    std::lock_guard<std::mutex> guard(lock);

    const auto pieceAlreadyExists = std::any_of(downloadedPiecesIds.cbegin(), downloadedPiecesIds.cend(),
                                                [&](unsigned int id) { return id == pieceId; });

    if (pieceAlreadyExists)
    {
        return;
    }

    const auto filesInfo = getFilesInfoContainingPiece(pieceId);

    for (const auto& fileInfo : filesInfo)
    {

        unsigned int position;

        if (fileInfo.firstPieceId != pieceId)
        {
            position = (pieceId - fileInfo.firstPieceId) * pieceSize - fileInfo.firstPieceOffset;
        }
        else
        {
            position = 0;
        }

        unsigned int lowerDataBoundary;

        if (fileInfo.firstPieceId == pieceId)
        {
            lowerDataBoundary = fileInfo.firstPieceOffset;
        }
        else
        {
            lowerDataBoundary = 0;
        }

        unsigned int upperDataBoundary;

        if (fileInfo.lastPieceId == pieceId)
        {
            upperDataBoundary = fileInfo.lastPieceOffset;
        }
        else
        {
            upperDataBoundary = torrentFileInfo->pieceLength;
        }

        if (lowerDataBoundary == 0 && upperDataBoundary == torrentFileInfo->pieceLength)
        {
            fileSystemService->writeAtPosition(fileInfo.filePath, data, position);
        }
        else
        {
            std::basic_string<unsigned char> dataToWrite{data.begin() + lowerDataBoundary,
                                                         data.begin() + upperDataBoundary};

            fileSystemService->writeAtPosition(fileInfo.filePath, dataToWrite, position);
        }
    }

    const auto metadataFilePosition = downloadedPiecesIds.size() * 4;

    const auto metadataFileData = libs::bytes::BytesConverter::int32ToBytes(pieceId);

    fileSystemService->writeAtPosition(absoluteMetadataFilePath, metadataFileData, metadataFilePosition);

    downloadedPiecesIds.push_back(pieceId);
}

const std::vector<unsigned int>& PieceRepositoryImpl::getDownloadedPieces() const
{
    std::lock_guard<std::mutex> guard(lock);

    return downloadedPiecesIds;
}

bool PieceRepositoryImpl::contains(unsigned int pieceId) const
{
    std::lock_guard<std::mutex> guard(lock);

    const auto foundPieceIdIterator = std::find(downloadedPiecesIds.begin(), downloadedPiecesIds.end(), pieceId);

    return foundPieceIdIterator != downloadedPiecesIds.end();
}

std::vector<FilePieceInfo> PieceRepositoryImpl::getFilesInfoContainingPiece(unsigned int pieceId) const
{
    std::vector<FilePieceInfo> filesToSave;
    for (const auto& fileInfo : allFilesInfo)
    {
        if (pieceId >= fileInfo.firstPieceId && pieceId <= fileInfo.lastPieceId)
        {
            filesToSave.push_back(fileInfo);
        }
    }

    if (filesToSave.empty())
    {
        throw std::out_of_range{
            fmt::format("PieceId out of range ({} out of {})", pieceId, allFilesInfo.end()->lastPieceId)};
    }

    return filesToSave;
}
}

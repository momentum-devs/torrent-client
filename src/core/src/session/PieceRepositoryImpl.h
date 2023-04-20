#include <memory>
#include <mutex>

#include "../torrentFile/TorrentFileInfo.h"
#include "FilePieceInfo.h"
#include "fileSystem/FileSystemService.h"
#include "PieceRepository.h"

namespace core
{
class PieceRepositoryImpl : public PieceRepository
{
public:
    PieceRepositoryImpl(std::shared_ptr<libs::fileSystem::FileSystemService>, unsigned int pieceSize,
                        std::shared_ptr<TorrentFileInfo> torrentFileInfo, const std::string& destinationDirectory);

    void save(unsigned int pieceId, const std::basic_string<unsigned char>& data) override;
    const std::vector<unsigned int>& getDownloadedPieces() const override;
    bool contains(unsigned int pieceId) const override;

private:
    std::vector<FilePieceInfo> getFilesInfoContainingPiece(unsigned int pieceId) const;

    std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemService;
    unsigned int pieceSize;
    const std::shared_ptr<TorrentFileInfo> torrentFileInfo;
    const std::string absoluteMetadataFilePath;
    std::vector<FilePieceInfo> allFilesInfo;
    std::vector<unsigned int> downloadedPiecesIds;
    mutable std::mutex lock;
};
}

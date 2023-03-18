#include <memory>
#include <mutex>

#include "../torrentFile/TorrentFileInfo.h"
#include "FilePieceInfo.h"
#include "fileSystem/FileSystemService.h"
#include "PieceRepository.h"
#include "PiecesSerializer.h"

namespace core
{
class PieceRepositoryImpl : public PieceRepository
{
public:
    PieceRepositoryImpl(std::shared_ptr<libs::fileSystem::FileSystemService>, std::shared_ptr<PiecesSerializer>,
                        unsigned int pieceSize, std::shared_ptr<TorrentFileInfo> torrentFileInfo,
                        const std::string& destinationDirectory);

    void save(unsigned int pieceId, const std::basic_string<unsigned char>& data) override;
    const std::vector<unsigned int>& getDownloadedPieces() const override;
    bool contains(unsigned int pieceId) const override;

private:
    const FilePieceInfo& getFileInfo(unsigned int pieceId) const;

    std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemService;
    std::shared_ptr<PiecesSerializer> piecesSerializer;
    unsigned int pieceSize;
    const std::shared_ptr<TorrentFileInfo> torrentFileInfo;
    const std::string absoluteMetadataFilePath;
    std::vector<FilePieceInfo> filesInfo;
    std::vector<unsigned int> downloadedPiecesIds;
    mutable std::mutex lock;
};
}

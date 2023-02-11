#include <memory>
#include <mutex>

#include "fileSystem/FileSystemService.h"
#include "PieceRepository.h"
#include "PiecesSerializer.h"

namespace core
{
class PieceRepositoryImpl : public PieceRepository
{
public:
    PieceRepositoryImpl(std::shared_ptr<libs::fileSystem::FileSystemService>, std::shared_ptr<PiecesSerializer>,
                        unsigned int pieceSize, const std::string& absoluteDataFilePath,
                        const std::string& absoluteMetadataFilePath);

    void save(unsigned int pieceId, const std::basic_string<unsigned char>& data) override;
    std::vector<unsigned int> findAllPiecesIds() const override;
    bool contains(unsigned int pieceId) const override;

private:
    std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemService;
    std::shared_ptr<PiecesSerializer> piecesSerializer;
    unsigned int pieceSize;
    const std::string& absoluteDataFilePath;
    const std::string& absoluteMetadataFilePath;
    mutable std::mutex lock;
};
}

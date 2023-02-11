#include <memory>

#include "fileSystem/FileSystemService.h"
#include "PieceRepository.h"

namespace core
{
class PieceRepositoryImpl : public PieceRepository
{
public:
    PieceRepositoryImpl(std::shared_ptr<libs::fileSystem::FileSystemService>, unsigned int pieceSize,
                        const std::string& absoluteFilePath);

    void save(unsigned int pieceId, const std::basic_string<unsigned char>& data) override;
    std::vector<unsigned int> findAllPiecesIds() const override;
    bool contains(unsigned int pieceId) const override;

private:
    std::shared_ptr<libs::fileSystem::FileSystemService> fileSystemService;
    unsigned int pieceSize;
    const std::string& absoluteFilePath;
};
}

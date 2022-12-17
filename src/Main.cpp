#include <memory>
#include <iostream>

#include "FileSystemServiceImpl.h"
#include "TorrentFileDeserializerImpl.h"

int main()
{
    std::unique_ptr<FileSystemService> fileSystemService = std::make_unique<FileSystemServiceImpl>();

    std::unique_ptr<TorrentFileDeserializer> torrentFileParser = std::make_unique<TorrentFileDeserializerImpl>();

    auto torrentFilePath = "testFiles/example.torrent";

    auto torrentFileContent = fileSystemService->read(torrentFilePath);

    auto torrentFileInfo = torrentFileParser->deserialize(torrentFileContent);

    std::cout << torrentFileInfo << std::endl;

    return 0;
}

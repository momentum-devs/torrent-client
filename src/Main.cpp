#include <memory>
#include <iostream>

#include "FileSystemServiceImpl.h"
#include "TorrentFileParserImpl.h"

int main()
{
    std::unique_ptr<FileSystemService> fileSystemService = std::make_unique<FileSystemServiceImpl>();

    std::unique_ptr<TorrentFileParser> torrentFileParser = std::make_unique<TorrentFileParserImpl>();

    auto torrentFilePath = "sintel.torrent";

    auto torrentFileContent = fileSystemService->read(torrentFilePath);

    auto torrentFileInfo = torrentFileParser->parse(torrentFileContent);

    std::cout << torrentFileInfo << std::endl;

    return 0;
}

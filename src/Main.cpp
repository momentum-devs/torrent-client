#include "CommandLineArgumentParser.h"
#include "TorrentClientFactory.h"

int main(int argc, char* argv[])
{
    const auto arguments = CommandLineArgumentParser::parseArguments(argc, argv);

    const auto torrentClient = core::TorrentClientFactory::createTorrentClient();

    torrentClient->download(arguments.torrentFilePath, arguments.destinationDirectory);

    return 0;
}

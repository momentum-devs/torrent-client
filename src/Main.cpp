#include "CommandLineArgumentParser.h"
#include "loguru.hpp"
#include "TorrentClientFactory.h"

int main(int argc, char* argv[])
{
    loguru::g_preamble_date = false;

    loguru::init(argc, argv);
    
    const auto arguments = CommandLineArgumentParser::parseArguments(argc, argv);

    const auto torrentClient = core::TorrentClientFactory::createTorrentClient();

    torrentClient->download(arguments.torrentFilePath, arguments.destinationDirectory);

    return 0;
}

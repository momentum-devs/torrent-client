#include <boost/program_options.hpp>
#include <iostream>

#include "core/src/client/PeerIdGenerator.h"
#include "TorrentClientFactory.h"

int main(int argc, char* argv[])
{
    // TODO: add parser class
    boost::program_options::options_description description;

    description.add_options()("torrent_file, t", boost::program_options::value<std::string>(),
                              "path to torrent file to download");

    description.add_options()("destination_directory, d", boost::program_options::value<std::string>(),
                              "path to directory where file(s) will be saved");

    boost::program_options::variables_map variablesMap;

    auto parsedArguments = boost::program_options::parse_command_line(argc, argv, description);

    boost::program_options::store(parsedArguments, variablesMap);

    boost::program_options::notify(variablesMap);

    if (!variablesMap.count("torrent_file"))
    {
        std::cerr << "put argument: torrent_file, t - path to torrent file to download" << std::endl;
        return 0;
    }

    if (!variablesMap.count("destination_directory"))
    {
        std::cerr << "put argument: destination_directory, d - path to directory where file(s) will be saved"
                  << std::endl;
        return 0;
    }

    const auto torrentFilePath = variablesMap["torrent_file"].as<std::string>();

    const auto destinationDirectory = variablesMap["destination_directory"].as<std::string>();

    const auto torrentClient = core::TorrentClientFactory::createTorrentClient();

    torrentClient->download(torrentFilePath, destinationDirectory);

    return 0;
}

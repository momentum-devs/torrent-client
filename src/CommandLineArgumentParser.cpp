#include "CommandLineArgumentParser.h"

#include <boost/program_options.hpp>

#include "loguru.hpp"

CommandLineArguments CommandLineArgumentParser::parseArguments(int argc, char** argv)
{
    boost::program_options::options_description description;

    description.add_options()("torrent_file, t", boost::program_options::value<std::string>(),
                              "path to torrent file to download");

    description.add_options()("destination_directory, d", boost::program_options::value<std::string>(),
                              "path to directory where file(s) will be saved");

    boost::program_options::variables_map variablesMap;

    const auto parsedArguments = boost::program_options::parse_command_line(argc, argv, description);

    boost::program_options::store(parsedArguments, variablesMap);

    boost::program_options::notify(variablesMap);

    if (!variablesMap.count("torrent_file"))
    {
        LOG_S(FATAL) << "Missing argument: torrent_file, t - path to torrent file to download.";

        throw std::runtime_error("Torrent file argument not provided.");
    }

    if (!variablesMap.count("destination_directory"))
    {
        LOG_S(FATAL) << "Missing argument: destination_directory, d - path to directory where file(s) will be saved.";

        throw std::runtime_error("Destination directory argument not provided.");
    }

    const auto torrentFilePath = variablesMap["torrent_file"].as<std::string>();

    const auto destinationDirectory = variablesMap["destination_directory"].as<std::string>();

    return {torrentFilePath, destinationDirectory};
}

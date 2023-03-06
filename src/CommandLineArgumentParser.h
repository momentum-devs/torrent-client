#pragma once

#include "CommandLineArguments.h"

class CommandLineArgumentParser
{
public:
    static CommandLineArguments parseArguments(int argc, char* argv[]);
};

#include "GetProjectPath.h"

#include "GetExecutablePath.h"
#include "StringHelper.h"
#include "errors/FileNotFound.h"

namespace
{
constexpr auto fileNotFoundMessage{"Project directory not found in path: "};
}

std::string getProjectPath(const std::string& projectName)
{
    const std::string currentPath = getExecutablePath();

    const auto projectNamePosition = currentPath.find(projectName);

    if (projectNamePosition == std::string::npos)
    {
        throw errors::FileNotFound{fileNotFoundMessage + currentPath};
    }

    auto projectPath = substring(currentPath, 0, projectNamePosition + projectName.length() + 1);

    return projectPath;
}

#include "GetProjectPath.h"

#include "collection/StringHelper.h"
#include "errors/FileNotFound.h"
#include "GetExecutablePath.h"

namespace
{
constexpr auto fileNotFoundMessage{"Project directory not found in path: "};
}

namespace libs::fileSystem
{
std::string getProjectPath(const std::string& projectName)
{
    const std::string currentPath = getExecutablePath();

    const auto projectNamePosition = currentPath.find(projectName);

    if (projectNamePosition == std::string::npos)
    {
        throw errors::FileNotFound{fileNotFoundMessage + currentPath};
    }

    auto projectPath = libs::collection::substring(currentPath, 0, projectNamePosition + projectName.length() + 1);

    return projectPath;
}
}

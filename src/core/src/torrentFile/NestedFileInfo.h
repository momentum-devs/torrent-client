#pragma once

#include <ostream>
#include <string>
#include <tuple>

namespace core
{
struct NestedFileInfo
{
    std::string name;
    long long length;
};

inline std::ostream& operator<<(std::ostream& os, const NestedFileInfo& nestedFileInfo)
{
    return os << "name: " << nestedFileInfo.name << "\nlength: " << nestedFileInfo.length;
}

inline bool operator==(const NestedFileInfo& lhs, const NestedFileInfo& rhs)
{
    auto tieStruct = [](const NestedFileInfo& nestedFileInfo)
    { return std::tie(nestedFileInfo.name, nestedFileInfo.length); };

    return tieStruct(lhs) == tieStruct(rhs);
}
}

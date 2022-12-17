#pragma once

#include <ostream>

struct HttpResponse
{
    int statusCode;
    std::string data;
};

inline bool operator==(const HttpResponse& lhs, const HttpResponse& rhs)
{
    return (lhs.statusCode == rhs.statusCode && lhs.data == rhs.data);
}

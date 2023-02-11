#pragma once

#include "gmock/gmock.h"

#include "PiecesSerializer.h"

namespace core
{
class PiecesSerializerMock : public PiecesSerializer
{
public:
    MOCK_METHOD(std::string, serialize, (const std::vector<unsigned int>&), (const));
    MOCK_METHOD(std::vector<unsigned int>, deserialize, (const std::string&), (const));
};
}

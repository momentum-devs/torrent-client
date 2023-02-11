#include "PiecesSerializerImpl.h"

#include "errors/InvalidJsonError.h"
#include "nlohmann/json.hpp"

namespace core
{
namespace
{
nlohmann::json parseJsonText(const std::string& jsonText);
}

std::string PiecesSerializerImpl::serialize(const std::vector<unsigned int>& piecesIds) const
{
    nlohmann::json serializedPiecesIds = nlohmann::json::array();

    for (const auto& pieceId : piecesIds)
    {
        serializedPiecesIds.push_back(pieceId);
    }

    return serializedPiecesIds.empty() ? "[]" : serializedPiecesIds.dump();
}

std::vector<unsigned int> PiecesSerializerImpl::deserialize(const std::string& jsonText) const
{
    const auto json = parseJsonText(jsonText);

    if (json.empty())
    {
        return {};
    }

    std::vector<unsigned int> piecesIds;

    for (const auto& jsonPieceId : json)
    {
        piecesIds.emplace_back(jsonPieceId);
    }

    return piecesIds;
}

namespace
{
nlohmann::json parseJsonText(const std::string& jsonText)
{
    try
    {
        return nlohmann::json::parse(jsonText);
    }
    catch (const std::exception& e)
    {
        throw exceptions::InvalidJsonError{e.what()};
    }
}

}
}

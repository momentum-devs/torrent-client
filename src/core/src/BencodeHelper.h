#pragma once

#include "bencode.hpp"
#include "errors/InvalidBencodeFileFormatError.h"
#include "errors/MissingTorrentInfo.h"
#include "fmt/format.h"

inline bencode::data parseBencode(const std::string& bencodeText)
{
    try
    {
        return bencode::decode(bencodeText);
    }
    catch (const std::exception& e)
    {
        throw errors::InvalidBencodeFileFormatError(e.what());
    }
}

inline bencode::dict getDictionary(const bencode::data& data)
{
    try
    {
        return std::get<bencode::dict>(data);
    }
    catch (const std::exception& e)
    {
        throw errors::MissingTorrentInfo{"Missing dictionary."};
    }
}

template <typename T>
inline T getFieldValue(const bencode::dict& bencodeDictionary, const std::string& fieldName)
{
    try
    {
        return std::get<T>(bencodeDictionary.at(fieldName));
    }
    catch (const std::exception& e)
    {
        throw errors::MissingTorrentInfo{fmt::format("Missing {} field.", fieldName)};
    }
}

template <>
inline bencode::data getFieldValue<bencode::data>(const bencode::dict& bencodeDictionary, const std::string& fieldName)
{
    try
    {
        return bencodeDictionary.at(fieldName);
    }
    catch (const std::exception& e)
    {
        throw errors::MissingTorrentInfo{fmt::format("Missing {} field.", fieldName)};
    }
}

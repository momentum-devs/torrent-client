#pragma once

#include <gmock/gmock.h>

#include "UdpClient.h"

class UdpClientMock : public UdpClient
{
public:
    MOCK_METHOD(std::string, read, (const std::string& address, unsigned short port), ());
};

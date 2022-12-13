#pragma once

#include "UdpClient.h"

class UdpClientImpl : public UdpClient
{
public:
    std::string receiveData(const std::string& address, unsigned short port) override;
};

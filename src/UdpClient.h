#pragma once

#include <string>

class UdpClient
{
public:
    virtual ~UdpClient() = default;

    virtual std::string receiveData(const std::string& address, unsigned short port) = 0;
};

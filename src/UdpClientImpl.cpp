#include "UdpClientImpl.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>

std::string UdpClientImpl::receiveData(const std::string& address, unsigned short port)
{
    boost::asio::io_service ioService;
    boost::asio::ip::udp::endpoint updEndpoint(boost::asio::ip::address::from_string("0.0.0.0"), 10114);
    boost::asio::ip::udp::socket udpSocket(ioService, updEndpoint);

    std::vector<char> buffer(128);

    udpSocket.receive_from(boost::asio::buffer(buffer,8), updEndpoint);

    std::string data(buffer.begin(), buffer.end());

    return data;
}

#include "UdpClientImpl.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <iostream>

std::string UdpClientImpl::receiveData(const std::string& address, unsigned short port)
{
    try
    {
        boost::asio::io_context io_context;
        boost::asio::ip::udp::socket socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 6969));

        for (;;)
        {
            boost::array<char, 1> outputBuffer;
            boost::asio::ip::udp::endpoint remote_endpoint{};
            socket.receive_from(boost::asio::buffer(outputBuffer), remote_endpoint);

            boost::system::error_code ignored_error;
            socket.send_to(boost::asio::buffer("x"), remote_endpoint, 0, ignored_error);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return "x";
}

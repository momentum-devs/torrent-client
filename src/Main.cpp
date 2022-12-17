#include <boost/asio.hpp>
#include <iostream>

#include "TorrentFileDeserializerImpl.h"

int main()
{
    boost::asio::io_context context;
    boost::asio::ip::tcp::socket socket(context);
    boost::asio::ip::address address = boost::asio::ip::make_address("169.1.40.40");
    boost::asio::ip::tcp::endpoint endpoint(address, 51414);

    boost::system::error_code error;

    socket.connect(endpoint, error);

    if (!error)
    {
        std::cout << "The connection has been established!";
    }
    else
    {
        std::cerr << "Something went wrong :(";
    }

    return 0;
}

#include <thread>
#include <iostream>
#include "server.h"

const uint DEFAULT_THREAD_POOL_SIZE{ 2 };

int main()
{
    try
    {
        std::cout << "[GAME] Starting Monopoly Bank\n"
                "Version : 0.0.0\n"
                "Author  : Eduardo Cipriano - edfcsx@gmail.com\n"
                "Press Ctrl+C to stop the server\n\n";

        uint thread_pool_size = std::thread::hardware_concurrency() * 2;

        if (thread_pool_size == 0)
            thread_pool_size = DEFAULT_THREAD_POOL_SIZE;

        Server &server = Server::getInstance();
        server.Start(thread_pool_size);

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            server.process_io_messages();
        }
    }
    catch (const std::system_error & e)
    {
        std::cout << "Server start failed: " << e.what() << "\n";
    }

    return 0;
}

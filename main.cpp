#include <thread>
#include <iostream>
#include "server.h"

using namespace std;

const uint DEFAULT_THREAD_POOL_SIZE{ 2 };

int main()
{
    try
    {
        Server server;

        uint thread_pool_size = std::thread::hardware_concurrency() * 2;

        if (thread_pool_size == 0)
            thread_pool_size = DEFAULT_THREAD_POOL_SIZE;

        server.Start(3333, thread_pool_size);

        while (true)
            std::this_thread::sleep_for(std::chrono::seconds(15));
    }
    catch (const std::system_error & e)
    {
        cout << "Server start failed: " << e.what() << endl;
    }

    return 0;
}

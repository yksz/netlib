#include "net/tcp.h"
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <gtest/gtest.h>

using namespace net;

TEST(TCP, ListenAndConnect) {
    // setup:
    const unsigned int port = 8080;
    const char message[] = "message";
    std::condition_variable condition;
    std::mutex mutex;

    // when: run a TCP server
    std::thread th([&]() {
        error err;

        std::shared_ptr<TCPListener> listener;
        err = ListenTCP(port, &listener);
        EXPECT_EQ(error::nil, err);
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.notify_one();
        }

        std::shared_ptr<TCPSocket> socket;
        err = listener->Accept(&socket);
        EXPECT_EQ(error::nil, err);

        // then: receive the message
        char buf[256] = {0};
        err = socket->ReadFull(buf, sizeof(message));
        EXPECT_EQ(error::nil, err);
        EXPECT_STREQ(message, buf);
    });
    // wait until the UDP server starts to running
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock);
    }

    // when: connect to the TCP server
    error err;

    std::shared_ptr<TCPSocket> socket;
    err = ConnectTCP("localhost", port, 10000, &socket);
    EXPECT_EQ(error::nil, err);

    // when: send a message
    err = socket->WriteFull(message, sizeof(message));
    EXPECT_EQ(error::nil, err);

    // cleanup:
    th.join();
}

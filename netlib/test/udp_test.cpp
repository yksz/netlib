#include "net/udp.h"
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <gtest/gtest.h>

using namespace net;

TEST(UDP, ListenAndConnect) {
    // setup:
    const unsigned int port = 8080;
    const char message[] = "message";
    std::condition_variable condition;
    std::mutex mutex;

    // when: run a UDP server
    std::thread th([&]() {
        error err;

        std::shared_ptr<UDPSocket> socket;
        err = ListenUDP(port, &socket);
        EXPECT_EQ(error::nil, err);
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition.notify_one();
        }

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

    // when: connect to the UDP server
    error err;

    std::shared_ptr<UDPSocket> socket;
    err = ConnectUDP("localhost", port, &socket);
    EXPECT_EQ(error::nil, err);

    // when: send a message
    err = socket->WriteFull(message, sizeof(message));
    EXPECT_EQ(error::nil, err);

    // cleanup:
    th.join();
}

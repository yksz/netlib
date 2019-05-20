#include "netlib/udp.h"
#include <future>
#include <string>
#include <thread>
#include <gtest/gtest.h>

using namespace net;

TEST(UDP, ListenAndConnect) {
    // setup:
    const std::string host = "localhost";
    const unsigned int port = 8080;
    const char message[] = "message";
    std::promise<bool> promise;
    auto future = promise.get_future();

    // when: run a UDP server
    std::thread th([&]() {
        error err;

        std::shared_ptr<UDPSocket> socket;
        err = ListenUDP(port, &socket);
        EXPECT_EQ(error::nil, err);

        promise.set_value(true);

        // then: receive the message
        char buf[256] = {0};
        err = socket->ReadFull(buf, sizeof(message));
        EXPECT_EQ(error::nil, err);
        EXPECT_STREQ(message, buf);
    });
    // wait until the UDP server starts to running
    future.get();

    error err;

    // when: connect to the UDP server
    std::shared_ptr<UDPSocket> socket;
    err = ConnectUDP(host, port, &socket);
    EXPECT_EQ(error::nil, err);

    // when: send a message
    err = socket->WriteFull(message, sizeof(message));
    EXPECT_EQ(error::nil, err);

    // cleanup:
    th.join();
}

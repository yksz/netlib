#include "net/tcp.h"
#include <chrono>
#include <future>
#include <string>
#include <thread>
#include <gtest/gtest.h>

using namespace net;

TEST(TCP, ListenAndConnect) {
    // setup:
    const std::string host = "localhost";
    const unsigned int port = 8080;
    const int64_t connectionTimeout = 1000; // ms
    const char message[] = "message";
    std::promise<bool> promise;
    auto future = promise.get_future();

    // when: run a TCP server
    std::thread th([&]() {
        error err;

        std::shared_ptr<TCPListener> listener;
        err = ListenTCP(port, &listener);
        EXPECT_EQ(error::nil, err);

        promise.set_value(true);

        std::shared_ptr<TCPSocket> socket;
        err = listener->Accept(&socket);
        EXPECT_EQ(error::nil, err);

        // then: receive the message
        char buf[256] = {0};
        err = socket->ReadFull(buf, sizeof(message));
        EXPECT_EQ(error::nil, err);
        EXPECT_STREQ(message, buf);
    });
    // wait until the TCP server starts to running
    future.get();

    error err;

    // when: connect to the TCP server
    std::shared_ptr<TCPSocket> socket;
    err = ConnectTCP(host, port, connectionTimeout, &socket);
    EXPECT_EQ(error::nil, err);

    // when: send a message
    err = socket->WriteFull(message, sizeof(message));
    EXPECT_EQ(error::nil, err);

    // cleanup:
    th.join();
}

TEST(TCP, ConnectionTimeout) {
    using namespace std::chrono;

    // setup:
    const std::string host = "192.168.0.254"; // a host which cannot connect
    const unsigned int port = 8080;
    const int64_t connectionTimeout = 10; // ms

    // when: start stopwatch
    auto start = system_clock::now();

    // when: connect to a TCP server
    std::shared_ptr<TCPSocket> socket;
    error err = ConnectTCP(host, port, connectionTimeout, &socket);

    // when: stop stopwatch
    auto stop = system_clock::now();
    auto diff = duration_cast<milliseconds>(stop - start);

    // then: err is timeout
    EXPECT_EQ(error::timedout, err);

    // then: time of timeout is accurate
    EXPECT_NEAR(connectionTimeout, diff.count(), connectionTimeout * 0.5);
}

TEST(TCP, SocketTimeout) {
    using namespace std::chrono;

    // setup:
    const std::string host = "localhost";
    const unsigned int port = 8080;
    const int64_t connectionTimeout = 1000; // ms
    const int64_t socketTimeout = 10; // ms
    std::promise<bool> promise;
    auto future = promise.get_future();

    // when: run a TCP server
    std::thread th([&]() {
        error err;

        std::shared_ptr<TCPListener> listener;
        err = ListenTCP(port, &listener);
        EXPECT_EQ(error::nil, err);

        promise.set_value(true);

        std::shared_ptr<TCPSocket> socket;
        err = listener->Accept(&socket);
        EXPECT_EQ(error::nil, err);

        std::this_thread::sleep_for(milliseconds(socketTimeout * 2));
    });
    // wait until the TCP server starts to running
    future.get();

    error err;

    // when: connect to the TCP server
    std::shared_ptr<TCPSocket> socket;
    err = ConnectTCP(host, port, connectionTimeout, &socket);
    EXPECT_EQ(error::nil, err);

    // when: start stopwatch
    auto start = system_clock::now();

    // when: set timeout and receive a message
    socket->SetTimeout(socketTimeout);
    char buf[256] = {0};
    err = socket->ReadFull(buf, sizeof(buf));

    // when: stop stopwatch
    auto stop = system_clock::now();
    auto diff = duration_cast<milliseconds>(stop - start);

    // then: err is EAGAIN or EWOULDBLOCK
    EXPECT_TRUE(err == error::again || err == error::wouldblock);

    // then: time of timeout is accurate
    EXPECT_NEAR(socketTimeout, diff.count(), socketTimeout * 0.5);

    // cleanup:
    th.join();
}

TEST(TCP, AcceptTimeout) {
    using namespace std::chrono;

    // setup:
    const unsigned int port = 8080;
    const int64_t acceptTimeout = 10; // ms

    error err;

    // when: run a TCP server
    std::shared_ptr<TCPListener> listener;
    err = ListenTCP(port, &listener);
    EXPECT_EQ(error::nil, err);

    // when: set timeout
    listener->SetTimeout(acceptTimeout);

    // when: start stopwatch
    auto start = system_clock::now();

    // when: accept a client
    std::shared_ptr<TCPSocket> socket;
    err = listener->Accept(&socket);

    // when: stop stopwatch
    auto stop = system_clock::now();
    auto diff = duration_cast<milliseconds>(stop - start);

    // then: err is timeout
    EXPECT_EQ(error::timedout, err);

    // then: time of timeout is accurate
    EXPECT_NEAR(acceptTimeout, diff.count(), acceptTimeout * 0.5);
}
